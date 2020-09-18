// Copyright © 2020 Blockchain Commons, LLC

#include "ur.h"
#include "util.h"
#include "bc-crypto-base.h"
#include "bc-bytewords.h"
#include "CborEncoder.h"
#include "wally_core.h"
#include "crc32.h"
#include "wally_bip32.h"
#include "wally_address.h"
#include "keystore.h"
#include "network.h"

// source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md
//         f68d54efd0cf6f9943801412e273167c558c8189 (Single part UR only)


bool crypto_keypath(class CborWriter &writer, uint32_t *derivation, uint32_t derivation_len, uint32_t parent_fingerprint) {
    // crypto-keypath:
    writer.writeTag(304);
    writer.writeMap(2);
      writer.writeInt(1);
      writer.writeArray(derivation_len*2);
      uint32_t indx;
      for (size_t i=0; i<derivation_len; i++) {
        indx = derivation[i] & ~BIP32_INITIAL_HARDENED_CHILD;
        writer.writeInt(indx);
        if (keystore.is_bip32_indx_hardened(derivation[i])) {
          uint32_t cbor_true = 21;
          writer.writeSpecial(cbor_true);
        }
        else {
          uint32_t cbor_false = 20;
          writer.writeSpecial(cbor_false);
        }
      }
      writer.writeInt(2);
      writer.writeInt(parent_fingerprint);
}

size_t cbor_encode_hdkey_xpub(struct ext_key *key, uint8_t **buff_out, uint32_t parent_fingerprint, uint32_t *derivation, uint32_t derivation_len) {

    CborDynamicOutput output;
    CborWriter writer(output);

    if (network.get_network() == MAINNET)
        writer.writeMap(3);
    else
        writer.writeMap(4);
    writer.writeInt(3);
    writer.writeBytes(key->pub_key, sizeof(key->pub_key));
    writer.writeInt(4);
    writer.writeBytes(key->chain_code, sizeof(key->chain_code));
    // crypto-coininfo
    if (network.get_network() != MAINNET) {
        writer.writeInt(5);
        writer.writeTag(305);
          writer.writeMap(1);
          writer.writeInt(2);
          writer.writeInt(1);
    }
    writer.writeInt(6);
    crypto_keypath(writer, derivation, derivation_len, parent_fingerprint);

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();

}

size_t cbor_encode_crypto_seed(uint8_t *seed, size_t len, uint8_t **buff_out, uint32_t *unix_timestamp=NULL) {

    CborDynamicOutput output;
    CborWriter writer(output);

    if (unix_timestamp != NULL)
        writer.writeMap(2);
    else
        writer.writeMap(1);
    writer.writeInt(1);
    writer.writeBytes(seed, len);

    if (unix_timestamp != NULL) {
        writer.writeInt(2);
        writer.writeTag(100);
        writer.writeInt(*unix_timestamp);
    }

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();
}

size_t cbor_encode_slip39_share(SLIP39ShareSeq *slip39_generate, size_t share_wndx, uint8_t **buff_out) {

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeMap(1);
    writer.writeInt(1);
    writer.writeArray(1);
    writer.writeArray(SLIP39ShareSeq::WORDS_PER_SHARE);

    for (size_t i=0; i < SLIP39ShareSeq::WORDS_PER_SHARE; i++) {
        const char *wrd = slip39_generate->get_share_word(share_wndx, i);
        writer.writeString(wrd, strlen(wrd));
    }

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();
}

size_t cbor_encode_address(uint8_t *address, size_t len, uint8_t **buff_out) {

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeMap(1);
    writer.writeInt(3);
    writer.writeBytes(address, len);

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();
}

bool ur_encode(String ur_type, uint8_t *cbor, uint32_t cbor_size, String &ur_string)
{
    // Encode cbor payload as bytewords
    char *payload_bytewords = bytewords_encode(bw_minimal, cbor, cbor_size);
    if(payload_bytewords == NULL) {
      Serial.println("ur_encode bytewords failed");
      return false;
    }

    ur_string = "ur:" + ur_type + "/" + String(payload_bytewords);

    free(payload_bytewords);

    return true;
}

bool ur_encode_hd_pubkey_xpub(String &xpub_bytewords, uint32_t *derivation, uint32_t derivation_len) {
    bool retval;
    ext_key xpub;
    uint8_t *cbor_xpub = NULL;

    (void)bip32_key_from_parent_path(&keystore.root, derivation, derivation_len, BIP32_FLAG_KEY_PRIVATE, &xpub);

    uint32_t parent_fingerprint;
    ((uint8_t *)&parent_fingerprint)[0] = xpub.parent160[3];
    ((uint8_t *)&parent_fingerprint)[1] = xpub.parent160[2];
    ((uint8_t *)&parent_fingerprint)[2] = xpub.parent160[1];
    ((uint8_t *)&parent_fingerprint)[3] = xpub.parent160[0];

    size_t cbor_xpub_size = cbor_encode_hdkey_xpub(&xpub, &cbor_xpub, parent_fingerprint, derivation, derivation_len);
    if (cbor_xpub_size == 0) {
        return false;
    }

    retval = ur_encode("crypto-hdkey", cbor_xpub, cbor_xpub_size, xpub_bytewords);
    if (retval == false) {
        return false;
    }

    // @FIXME: free also on premature exit
    free(cbor_xpub);

    return true;
}

bool ur_encode_crypto_seed(uint8_t *seed, size_t seed_len, String &seed_ur, uint32_t *unix_timestamp) {
    bool retval;
    uint8_t *cbor_seed = NULL;

    size_t cbor_seed_size = cbor_encode_crypto_seed(seed, seed_len, &cbor_seed, unix_timestamp);
    if (cbor_seed_size == 0) {
        return false;
    }

    //print_hex(cbor_seed, cbor_seed_size);

    retval = ur_encode("crypto-seed", cbor_seed, cbor_seed_size, seed_ur);
    if (retval == false) {
        return false;
    }

    // @FIXME: free also on premature exit
    free(cbor_seed);

    return true;
}

bool ur_encode_address(uint8_t *address, size_t address_len, String &address_ur) {
    bool retval;
    uint8_t *cbor = NULL;

    size_t cbor_size = cbor_encode_crypto_seed(address, address_len, &cbor);
    if (cbor_size == 0) {
        return false;
    }

    retval = ur_encode("crypto-address", cbor, cbor_size, address_ur);
    if (retval == false) {
        return false;
    }

    // @FIXME: free also on premature exit
    free(cbor);

    return true;
}

bool ur_encode_output_descriptor(String &ur, uint32_t *derivation, uint32_t derivation_len, uint32_t parent_fingerprint) {
    bool retval;
    uint8_t *cbor = NULL;
    size_t cbor_size;
    uint8_t *buff_out = NULL;
    struct ext_key child_key;

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeTag(404); // @FIXME currently fixed ton only wpkh (404)

    (void)bip32_key_from_parent_path(&keystore.root, derivation, derivation_len, 0, &child_key);
    cbor_size = cbor_encode_hdkey_xpub(&child_key, &buff_out, parent_fingerprint, derivation, derivation_len);

    uint8_t *cbor_all = (uint8_t *)malloc(cbor_size + output.getSize());
    memcpy(cbor_all, output.getData(), output.getSize());
    memcpy(cbor_all + output.getSize(), buff_out, cbor_size);

    print_hex(cbor_all, cbor_size + output.getSize());

    retval = ur_encode("crypto-output", cbor_all, cbor_size + output.getSize(), ur);
    if (retval == false) {
        return false;
    }

    // @FIXME: free also on premature exit
    free(cbor);
    free(cbor_all);

    return true;
}

bool ur_encode_slip39_share(SLIP39ShareSeq *slip39_generate, size_t share_wndx, String &ur) {
    bool retval;
    uint8_t *cbor = NULL;

    size_t cbor_size = cbor_encode_slip39_share(slip39_generate, share_wndx, &cbor);
    if (cbor_size == 0) {
        return false;
    }

    print_hex(cbor, cbor_size);

    retval = ur_encode("crypto-slip39", cbor, cbor_size, ur);
    if (retval == false) {
        return false;
    }

    // @FIXME: free also on premature exit
    free(cbor);

    return true;
}

bool test_ur(void) {

    int ret;
    {   // scope for every test case
        // source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-012-bytewords.md#exampletest-vector
        uint8_t seeds[] = {0xd9, 0x01, 0x2c, 0xa2, 0x01, 0x50, 0xc7, 0x09, 0x85, 0x80, 0x12, 0x5e, 0x2a, 0xb0, 0x98, 0x12,
                           0x53, 0x46, 0x8b, 0x2d, 0xbc, 0x52, 0x02, 0xd8, 0x64, 0x19, 0x47, 0xda};
        String seed_bytewords_expected = F("taaddwoeadgdstasltlabghydrpfmkbggufgludprfgmaotpiecffltntddwgmrp");

        char *seed_bytewords = bytewords_encode(bw_minimal, seeds, sizeof(seeds));

        if (strcmp(seed_bytewords_expected.c_str(), seed_bytewords) != 0) {
            Serial.println("bytewords failed");
            return false;
        }

        free(seed_bytewords);
    }
    {
        // https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-006-urtypes.md#exampletest-vector-1
        uint8_t payload[] = {0xC7, 0x09, 0x85, 0x80, 0x12, 0x5E, 0x2A, 0xB0, 0x98, 0x12, 0x53, 0x46, 0x8B, 0x2D, 0xBC, 0x52};
        uint32_t birthday = 18394;
        String ur_expected = F("ur:crypto-seed/oeadgdstasltlabghydrpfmkbggufgludprfgmaotpiecffltnltqdenos");
        String seed_ur;

        bool rval = ur_encode_crypto_seed(payload, sizeof(payload), seed_ur, &birthday);
        if (rval == false) {
          Serial.println(F("ur_encode_crypto_seed fails"));
          return false;
        }

        if (seed_ur != ur_expected) {
          Serial.println(F("ur_encode_crypto_seed wrong"));
          return false;
        }
    }
    {
        // https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-007-hdkey.md#exampletest-vector-2
        String path = F("m/44h/1h/1h/0/1");
        String derived_key = F("tpubDHW3GtnVrTatx38EcygoSf9UhUd9Dx1rht7FAL8unrMo8r2NWhJuYNqDFS7cZFVbDaxJkV94MLZAr86XFPsAPYcoHWJ7sWYsrmHDw5sKQ2K");
        String cbor_expected = F("a4035821026fe2355745bb2db3630bbc80ef5d58951c963c841f54170ba6e5c12be7fc12a6045820ced155c72456255881793514edc5"
                               "bd9447e7f74abb88c6d6b6480fd016ee8c8505d90131a1020106d90130a2018a182cf501f501f500f401f4021ae9181cf3");
        String ur_expect = F("ur:crypto-hdkey/oxaxhdclaojlvoechgferkdpqdiabdrflawshlhdmdcemtfnlrctghchbdolvwsednvdzcbgolaahdcxtot"
                             "tgostdkhfdahdlykkecbbweskrymwflvdylgerkloswtbrpfdbsticmwylkltahtaadehoyaoadamtaaddyoeadlecsdwykadyk"
                             "adykaewkadwkaocywlcscewfiavorkat");
        uint32_t parent_fingerprint = 3910671603;
        ext_key xpub;
        uint8_t *cbor_xpub;
        String xpub_ur;

        // STUB derivation path
        keystore.check_derivation_path(path.c_str(), true);

        ret = bip32_key_from_base58(derived_key.c_str(), &xpub);
        if (ret != WALLY_OK) {
            Serial.println(F("bip32_key_from_base58 failed"));
            return false;
        }

        size_t cbor_xpub_size = cbor_encode_hdkey_xpub(&xpub, &cbor_xpub, parent_fingerprint, keystore.derivation, keystore.derivationLen);
        if (cbor_xpub_size == 0) {
            Serial.println(F("cbor_encode_hdkey_xpub failed"));
            return false;
        }

        bool retval = compare_bytes_with_hex(cbor_xpub, cbor_xpub_size, cbor_expected.c_str());
        if (retval == false) {
            Serial.println(F("key in cbor format does not match expected value"));
            return false;
        }

        retval = ur_encode("crypto-hdkey", cbor_xpub, cbor_xpub_size, xpub_ur);
        if (retval == false) {
            Serial.println(F("ur encode: crypto-hdkey failed"));
            return false;
        }

        if (ur_expect != xpub_ur) {
            Serial.println(F("ur encode: crypto-hdkey wrong"));
            //Serial.println(xpub_ur);
            return false;
        }
        free(cbor_xpub);

        // CLEAN STUB
        stdDerivation stdDer = SINGLE_NATIVE_SEGWIT;
        keystore.save_standard_derivation_path(&stdDer, network.get_network());
    }
    return true;
}
