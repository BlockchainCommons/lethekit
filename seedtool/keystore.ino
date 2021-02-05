#include "keystore.h"

Keystore keystore = Keystore();

Keystore::Keystore(void) {
    // set default path for single native segwit key
    stdDerivation stdDer = SINGLE_NATIVE_SEGWIT;
    save_standard_derivation_path(&stdDer, network.get_network());
}

String Keystore::get_derivation_path(void) {
    return derivation_path;
}

bool Keystore::update_root_key(uint8_t *seed, size_t len, NetwtorkType network)
{
    uint32_t version_code = BIP32_VER_TEST_PRIVATE;
    switch(network) {
        case REGTEST:
        case TESTNET:
            version_code = BIP32_VER_TEST_PRIVATE;
            break;
        case MAINNET:
            version_code = BIP32_VER_MAIN_PRIVATE;
            break;
        default:
            version_code = BIP32_VER_TEST_PRIVATE;
    }

    res = bip32_key_from_seed(seed, len, version_code, 0, &root);
    if (res != WALLY_OK) {
        return false;
    }

    ((uint8_t *)&fingerprint)[0] = root.hash160[3];
    ((uint8_t *)&fingerprint)[1] = root.hash160[2];
    ((uint8_t *)&fingerprint)[2] = root.hash160[1];
    ((uint8_t *)&fingerprint)[3] = root.hash160[0];

    return true;
}

bool Keystore::calc_derivation_path(const char *path, uint32_t *derivation, uint32_t &derivation_len) {

    // source: https://github.com/micro-bitcoin/uBitcoin/blob/master/src/HDWallet.cpp
    static const char VALID_CHARS[] = "0123456789/h'";
    size_t len = strlen(path);
    const char * cur = path;

    // first check for weird char sequences
    for (size_t i=0; i<len; i++) {
        if(path[i] == '/'){
            if (i+1 < len && (path[i+1] == 'h' || path[i+1] == '/')) {
                // 'h' or '/' must not follow '/'
                return false;
            }
            if (i+2 < len && path[i+1] == '0' && isDigit(path[i+2])) {
                // e.g. /01 is invalid
                return false;
            }
        }
    }

    if(path[0] == 'm'){ // remove leading "m/"
        cur+=2;
        len-=2;
    }
    if(cur[len-1] == '/'){ // remove trailing "/"
        len--;
    }

    derivation_len = 1;
    // checking if all chars are valid and counting derivation length
    for(size_t i=0; i<len; i++){
        const char * pch = strchr(VALID_CHARS, cur[i]);
        if(pch == NULL){ // wrong character
            return false;
        }
        if(cur[i] == '/'){
            derivation_len++;
        }
    }

    if (derivation_len > MAX_DERIVATION_PATH_LEN)
        return false;

    memset(derivation, 0, MAX_DERIVATION_PATH_LEN);

    size_t current = 0;
    for(size_t i=0; i<len; i++){
        if(cur[i] == '/'){ // next
            current++;
            continue;
        }
        const char * pch = strchr(VALID_CHARS, cur[i]);
        uint32_t val = pch-VALID_CHARS;
        if(derivation[current] >= HARDENED_INDEX){ // can't have anything after hardened
            return false;
        }
        if(val < 10){
            derivation[current] = derivation[current]*10 + val;
        }else{ // h or ' -> hardened
            derivation[current] += HARDENED_INDEX;
        }
    }
    return true;
}

bool Keystore::check_derivation_path(const char *path, bool save) {

      uint32_t derivationLen_tmp;
      uint32_t derivation_tmp[MAX_DERIVATION_PATH_LEN];

      if (calc_derivation_path(path, derivation_tmp, derivationLen_tmp) == false) {
        Serial.println("calc_deriv fail 1");
        return false;
      }

      if (derivationLen_tmp > MAX_DERIVATION_PATH_LEN) {
          Serial.println("calc_deriv fail 2");
          return false;
      }

      if (save == false)
          return true;

      // save
      derivation_path = path;
      derivationLen = derivationLen_tmp;

      memset(derivation, 0, MAX_DERIVATION_PATH_LEN);
      memcpy(derivation, derivation_tmp, derivationLen*sizeof(uint32_t));

      standard_derivation_path = false;

      return true;
}

bool Keystore::save_standard_derivation_path(stdDerivation *path, NetwtorkType network) {

      String p;

      if (path) {
        std_derivation_path = *path;

        if (*path == SINGLE_NATIVE_SEGWIT) {
            p = F("m/84h/1h/0h");
        }
        else if (*path == SINGLE_NESTED_SEGWIT)
            p = F("m/49h/1h/0h");

        else if (*path == MULTISIG_NATIVE_SEGWIT)
            p = F("m/48h/0h/0h/2h");
      }
      else {
        if (standard_derivation_path == false)
            return false;
        p = get_derivation_path();
      }

      if (network == MAINNET)
        p.setCharAt(6, '0');
      else
        p.setCharAt(6, '1');

      bool ret = check_derivation_path(p.c_str(), true);

      standard_derivation_path = true;

      return ret;
}

bool Keystore::is_standard_derivation_path(void) {
    return standard_derivation_path;
}

bool Keystore::get_xpriv(ext_key *key_out) {

    res = bip32_key_from_parent_path(&root, derivation, derivationLen, BIP32_FLAG_KEY_PRIVATE, key_out);
    if (res != WALLY_OK) {
        return false;
    }

    return true;
}

bool Keystore::xpub_to_base58(ext_key *key, char **output, bool slip132) {

    int ret;
    unsigned char bytes[BIP32_SERIALIZED_LEN];
    uint32_t *bytes_ptr = (uint32_t *)bytes;

    if ((ret = bip32_key_serialize(key, BIP32_FLAG_KEY_PUBLIC, bytes, sizeof(bytes))))
        return false;

    if (slip132 && standard_derivation_path) {
      switch(network.get_network()) {
        case MAINNET:
            if (std_derivation_path == SINGLE_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x04b24746);
            }
            else if (std_derivation_path == SINGLE_NESTED_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x049d7cb2);
            }
            else if (std_derivation_path == MULTISIG_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x02aa7ed3);
            }
            break;
        case TESTNET:
        case REGTEST:
            if (std_derivation_path == SINGLE_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x045f1cf6);
            }
            else if (std_derivation_path == SINGLE_NESTED_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x044a5262);
            }
            else if (std_derivation_path == MULTISIG_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x02575483);
            }
            break;
        default:
            break;
      }
    }

    ret = wally_base58_from_bytes(bytes, BIP32_SERIALIZED_LEN, BASE58_FLAG_CHECKSUM, output);

    // clear
    memset(bytes, 0, sizeof(bytes));

    if (ret == WALLY_OK)
        return true;
    else
        return false;
}

bool Keystore::xpriv_to_base58(ext_key *key, char **output /*, bool slip132 */) {

    int ret;
    unsigned char bytes[BIP32_SERIALIZED_LEN];
    uint32_t *bytes_ptr = (uint32_t *)bytes;

    if ((ret = bip32_key_serialize(key, BIP32_FLAG_KEY_PRIVATE, bytes, sizeof(bytes))))
        return false;


/*  TODO: slip132
    if (slip132 && standard_derivation_path) {
      switch(network.get_network()) {
        case MAINNET:
            if (std_derivation_path == SINGLE_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x04b24746);
            }
            else if (std_derivation_path == SINGLE_NESTED_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x049d7cb2);
            }
            else if (std_derivation_path == MULTISIG_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x02aa7ed3);
            }
            break;
        case TESTNET:
        case REGTEST:
            if (std_derivation_path == SINGLE_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x045f1cf6);
            }
            else if (std_derivation_path == SINGLE_NESTED_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x044a5262);
            }
            else if (std_derivation_path == MULTISIG_NATIVE_SEGWIT) {
                *bytes_ptr = __builtin_bswap32(0x02575483);
            }
            break;
        default:
            break;
      }
    }
    */

    ret = wally_base58_from_bytes(bytes, BIP32_SERIALIZED_LEN, BASE58_FLAG_CHECKSUM, output);

    // clear
    memset(bytes, 0, sizeof(bytes));

    if (ret == WALLY_OK)
        return true;
    else
        return false;
}
