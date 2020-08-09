#include "keystore.h"

Keystore keystore = Keystore();

#define HARDENED_INDEX 0x80000000

Keystore::Keystore(void) {
    // set default path for single native segwit key
    stdDerivation stdDer = SINGLE_NATIVE_SEGWIT;
    save_standard_derivation_path(&stdDer, network.get_network());
    // set default format as QR_BASE58
    set_xpub_format();
    show_derivation_path = false;
    slip132 = false;
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
    return true;
}

bool Keystore::check_derivation_path(const char *path) {

    // source: https://github.com/micro-bitcoin/uBitcoin/blob/master/src/HDWallet.cpp
    // char ' excluded as keypad will support only 'h'
    static const char VALID_CHARS[] = "0123456789/h";
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

    derivationLen_tmp = 1;
    // checking if all chars are valid and counting derivation length
    for(size_t i=0; i<len; i++){
        const char * pch = strchr(VALID_CHARS, cur[i]);
        if(pch == NULL){ // wrong character
            return false;
        }
        if(cur[i] == '/'){
            derivationLen_tmp++;
        }
    }

    if (derivationLen_tmp > MAX_DERIVATION_PATH_LEN)
        return false;

    memset(derivation_tmp, 0, MAX_DERIVATION_PATH_LEN);

    size_t current = 0;
    for(size_t i=0; i<len; i++){
        if(cur[i] == '/'){ // next
            current++;
            continue;
        }
        const char * pch = strchr(VALID_CHARS, cur[i]);
        uint32_t val = pch-VALID_CHARS;
        if(derivation_tmp[current] >= HARDENED_INDEX){ // can't have anything after hardened
            return false;
        }
        if(val < 10){
            derivation_tmp[current] = derivation_tmp[current]*10 + val;
        }else{ // h or ' -> hardened
            derivation_tmp[current] += HARDENED_INDEX;
        }
    }
    return true;
}

bool Keystore::save_derivation_path(const char *path) {
      if (check_derivation_path(path) == false)
        return false;
      derivation_path = path;
      derivationLen = derivationLen_tmp;

      if (derivationLen > MAX_DERIVATION_PATH_LEN)
          return false;

      memset(derivation, 0, MAX_DERIVATION_PATH_LEN);
      memcpy(derivation, derivation_tmp, derivationLen*sizeof(uint32_t));

      standard_derivation_path = false;

      return true;
}

bool Keystore::save_standard_derivation_path(stdDerivation *path, NetwtorkType network) {

      String p;

      if (path) {
        if (*path == SINGLE_NATIVE_SEGWIT) {
            p = F("m/84h/1h/0h");
        }
        else if (*path == SINGLE_NESTED_SEGWIT)
            p = F("m/49h/1h/0h");
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

      bool ret = save_derivation_path(p.c_str());

      standard_derivation_path = true;

      return ret;
}

bool Keystore::is_standard_derivation_path(void) {
    return standard_derivation_path;
}

bool Keystore::get_xpub(ext_key *key_out) {

    res = bip32_key_from_parent_path(&root, derivation, derivationLen, BIP32_FLAG_KEY_PRIVATE, key_out);
    if (res != WALLY_OK) {
        return false;
    }

    return true;
}

bool Keystore::xpub_to_base58(ext_key *key, char **output) {

    int ret;
    unsigned char bytes[BIP32_SERIALIZED_LEN];

    if ((ret = bip32_key_serialize(key, BIP32_FLAG_KEY_PUBLIC, bytes, sizeof(bytes))))
        return false;

    if (slip132) {
      switch(network.get_network()) {
        case MAINNET:
            if (standard_derivation_path == SINGLE_NATIVE_SEGWIT) {
                ((uint32_t *)bytes)[0] = __builtin_bswap32(0x04b24746);
            }
            else if (standard_derivation_path == SINGLE_NESTED_SEGWIT) {
                ((uint32_t *)bytes)[0] = __builtin_bswap32(0x049d7cb2);
            }
            break;
        case TESTNET:
        case REGTEST:
            if (standard_derivation_path == SINGLE_NATIVE_SEGWIT) {
                *((uint32_t *)bytes) = __builtin_bswap32(0x045f1cf6);
            }
            else if (standard_derivation_path == SINGLE_NESTED_SEGWIT) {
                *((uint32_t *)bytes) = __builtin_bswap32(0x044a5262);
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

void Keystore::set_xpub_format(xpubEnc _format) {
    format = _format;
}

xpubEnc Keystore::get_xpub_format(void) {
    return format;
}

String Keystore::get_xpub_format_as_string(void) {
    switch(format) {
        case BASE58:
            return "base58";
        case QR_BASE58:
            return "qr-base58";
        case UR:
            return "ur";
        case QR_UR:
            return "qr_ur";
    }
}
