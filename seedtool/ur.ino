// Copyright © 2020 Blockchain Commons, LLC

#include "ur.h"
#include "util.h"
#include "bc-crypto-base.h"
#include "bc-bytewords.h"
#include "CborEncoder.h"
#include "wally_core.h"
#include "wally_bip32.h"
#include "wally_address.h"
#include "keystore.h"
#include "network.h"

// source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md
//         f68d54efd0cf6f9943801412e273167c558c8189 (Single part UR only)


bool crypto_coin_info(class CborWriter &writer, NetwtorkType network) {
    writer.writeInt(5);
    writer.writeTag(305);
    writer.writeMap(1);
    writer.writeInt(2);
    writer.writeInt(network == MAINNET ? 0 : 1);
}

bool crypto_keypath(class CborWriter &writer, uint32_t *derivation, uint32_t derivation_len, uint32_t fingerprint) {
    // crypto-keypath:
    writer.writeTag(304);
    writer.writeMap(3);
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
      writer.writeInt(fingerprint);

      writer.writeInt(3);
      writer.writeInt(derivation_len);
}

size_t cbor_encode_output_descriptor(struct ext_key *key, uint8_t **buff_out, uint32_t parent_fingerprint, uint32_t *derivation, uint32_t derivation_len) {

    // ATM lethekit only supports single native segwit output descriptor (wpkh)
    stdDerivation path;
    if (keystore.is_standard_derivation_path(&path) != true && path != SINGLE_NATIVE_SEGWIT) {
        Serial.println("error: only single native segwit supported atm!");
        return 0;
    }

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeTag(404); // witness-public-key-hash
    writer.writeTag(303); // crypto-hdkey

    writer.writeMap(5);

    writer.writeInt(3);
    writer.writeBytes(key->pub_key, sizeof(key->pub_key));
    writer.writeInt(4);
    writer.writeBytes(key->chain_code, sizeof(key->chain_code));

    // origin: crypto-keypath
    writer.writeInt(6);
    writer.writeTag(304);
    writer.writeMap(2);
      writer.writeInt(1);
      writer.writeArray(derivation_len*2);
      uint32_t indx;
      for (size_t i=0; i< derivation_len; i++) {
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
      writer.writeInt(keystore.fingerprint);

    // children: crypto-keypath
    writer.writeInt(7);
    writer.writeTag(304);
    writer.writeMap(1);
      writer.writeInt(1);
      writer.writeArray(2*2);


      uint32_t cbor_false = 20;
      uint32_t cbor_true = 21;
      writer.writeInt(0); // for now show only "receive" descriptor supported, without "change" descriptor
      writer.writeSpecial(cbor_false);

      writer.writeArray(0);  // denotes a star /*  (for all children)
      writer.writeSpecial(cbor_false);


    writer.writeInt(8);
    writer.writeInt(parent_fingerprint);

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();
}

size_t cbor_encode_hdkey_xpub(struct ext_key *key, uint8_t **buff_out, uint32_t parent_fingerprint, uint32_t *derivation, uint32_t derivation_len) {

    CborDynamicOutput output;
    CborWriter writer(output);

    if (network.get_network() == MAINNET)
        writer.writeMap(4);
    else
        writer.writeMap(5);
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
    crypto_keypath(writer, derivation, derivation_len, keystore.fingerprint);

    writer.writeInt(8);
    writer.writeInt(parent_fingerprint);

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();
}

size_t cbor_encode_hdkey_xpriv(struct ext_key *key, uint8_t **buff_out, uint32_t parent_fingerprint, uint32_t *derivation, uint32_t derivation_len) {

    CborDynamicOutput output;
    CborWriter writer(output);

    if (network.get_network() == MAINNET)
        writer.writeMap(5);
    else
        writer.writeMap(6);

    // is_private
    writer.writeInt(2);
    uint32_t cbor_true = 21;
    writer.writeSpecial(cbor_true);

    writer.writeInt(3);
    writer.writeBytes(key->priv_key, sizeof(key->priv_key));
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
    crypto_keypath(writer, derivation, derivation_len, keystore.fingerprint);

    writer.writeInt(8);
    writer.writeInt(parent_fingerprint);

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

/*
size_t cbor_encode_sskr_share(SSKRShareSeq *sskr_generate, size_t share_wndx, uint8_t **buff_out) {

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeMap(1);
    writer.writeInt(1);
    writer.writeArray(1);
    writer.writeArray(SSKRShareSeq::WORDS_PER_SHARE);

    for (size_t i=0; i < SSKRShareSeq::WORDS_PER_SHARE; i++) {
        const char *wrd = sskr_generate->get_share_word(share_wndx, i);
        writer.writeString(wrd, strlen(wrd));
    }

    *buff_out = (uint8_t *)malloc(output.getSize());
    memcpy(*buff_out, output.getData(), output.getSize());

    return output.getSize();
}
*/

size_t cbor_encode_address(uint8_t *address, size_t len, uint8_t **buff_out, NetwtorkType network) {

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeMap(2);
    // coin info is not mandatory for bech32 addresses
    crypto_coin_info(writer, network);
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

bool ur_encode_hd_pubkey_xpriv(String &xpriv_bytewords, uint32_t *derivation, uint32_t derivation_len) {
    bool retval;
    ext_key xpriv;
    uint8_t *cbor_xpriv = NULL;

    (void)bip32_key_from_parent_path(&keystore.root, derivation, derivation_len, BIP32_FLAG_KEY_PRIVATE, &xpriv);

    uint32_t parent_fingerprint;
    ((uint8_t *)&parent_fingerprint)[0] = xpriv.parent160[3];
    ((uint8_t *)&parent_fingerprint)[1] = xpriv.parent160[2];
    ((uint8_t *)&parent_fingerprint)[2] = xpriv.parent160[1];
    ((uint8_t *)&parent_fingerprint)[3] = xpriv.parent160[0];

    size_t cbor_xpriv_size = cbor_encode_hdkey_xpriv(&xpriv, &cbor_xpriv, parent_fingerprint, derivation, derivation_len);
    if (cbor_xpriv_size == 0) {
        return false;
    }

    //Serial.println("cbor xpriv:");
    //print_hex(cbor_xpriv, cbor_xpriv_size);

    retval = ur_encode("crypto-hdkey", cbor_xpriv, cbor_xpriv_size, xpriv_bytewords);
    if (retval == false) {
        return false;
    }

    // @FIXME: free also on premature exit
    free(cbor_xpriv);

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

    size_t cbor_size = cbor_encode_address(address, address_len, &cbor, network.get_network());
    if (cbor_size == 0) {
        return false;
    }

    retval = ur_encode("crypto-address", cbor, cbor_size, address_ur);
    if (retval == false) {
        return false;
    }

    print_hex(cbor, cbor_size);

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
    cbor_size = cbor_encode_output_descriptor(&child_key, &buff_out, parent_fingerprint, derivation, derivation_len);

    Serial.println("cbor output descriptor:");
    print_hex(buff_out, cbor_size);

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

bool test_ur(void) {

    int ret;
    {   // scope for every test case
        // source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-012-bytewords.md#exampletest-vector
        uint8_t seeds[] = {0xd9, 0x01, 0x2c, 0xa2, 0x01, 0x50, 0xc7, 0x09, 0x85, 0x80, 0x12, 0x5e, 0x2a, 0xb0, 0x98, 0x12,
                           0x53, 0x46, 0x8b, 0x2d, 0xbc, 0x52, 0x02, 0xd8, 0x64, 0x19, 0x47, 0xda};
        String seed_bytewords_expected = F("taaddwoeadgdstaslplabghydrpfmkbggufgludprfgmaotpiecffltntddwgmrp");

        char *seed_bytewords = bytewords_encode(bw_minimal, seeds, sizeof(seeds));

        if (strcmp(seed_bytewords_expected.c_str(), seed_bytewords) != 0) {
            Serial.println("bytewords failed.");
            return false;
        }

        free(seed_bytewords);
    }
    {
        // https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-006-urtypes.md#exampletest-vector-1
        uint8_t payload[] = {0xC7, 0x09, 0x85, 0x80, 0x12, 0x5E, 0x2A, 0xB0, 0x98, 0x12, 0x53, 0x46, 0x8B, 0x2D, 0xBC, 0x52};
        uint32_t birthday = 18394;
        String ur_expected = F("ur:crypto-seed/oeadgdstaslplabghydrpfmkbggufgludprfgmaotpiecffltnlpqdenos");
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
    return true;
}
