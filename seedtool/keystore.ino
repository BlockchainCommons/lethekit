#include "keystore.h"

#define HARDENED_INDEX 0x80000000

Keystore::Keystore(void) {
    // set default path for single native segwit key
    set_derivation_path();
    // set default format as QR_BASE58
    set_xpub_format();
}

void Keystore::set_derivation_path(String path) {
    derivation_path = path;
}

String Keystore::get_derivation_path(void) {
    return derivation_path;
}

bool Keystore::update(uint8_t *seed, size_t len)
{
    res = bip32_key_from_seed(seed, len, BIP32_VER_MAIN_PRIVATE, 0, &root);
    if (res != WALLY_OK) {
        return false;
    }
    return true;
}

bool Keystore::derivation_path_from_str(const char *path) {

    // source: https://github.com/micro-bitcoin/uBitcoin/blob/master/src/HDWallet.cpp
    static const char VALID_CHARS[] = "0123456789/'h";
    size_t len = strlen(path);
    const char * cur = path;
    if(path[0] == 'm'){ // remove leading "m/"
        cur+=2;
        len-=2;
    }
    if(cur[len-1] == '/'){ // remove trailing "/"
        len--;
    }
    derivationLen = 1;
    // checking if all chars are valid and counting derivation length
    for(size_t i=0; i<len; i++){
        const char * pch = strchr(VALID_CHARS, cur[i]);
        if(pch == NULL){ // wrong character
            return false;
        }
        if(cur[i] == '/'){
            derivationLen++;
        }
    }
    derivation = (uint32_t *)calloc(derivationLen, sizeof(uint32_t));
    size_t current = 0;
    for(size_t i=0; i<len; i++){
        if(cur[i] == '/'){ // next
            current++;
            continue;
        }
        const char * pch = strchr(VALID_CHARS, cur[i]);
        uint32_t val = pch-VALID_CHARS;
        if(derivation[current] >= HARDENED_INDEX){ // can't have anything after hardened
            free(derivation);
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

bool Keystore::get_xpub(const char *path, ext_key *root, ext_key *key_out)
{
    if (derivation_path_from_str(path) == false)
        return false;
    res = bip32_key_from_parent_path(root, derivation, derivationLen, BIP32_FLAG_KEY_PRIVATE, key_out);
    if (res != WALLY_OK) {
        return false;
    }

    return true;
}

void Keystore::set_xpub_format(xpub_encoding_e _format) {
    format = _format;
}

xpub_encoding_e Keystore::get_xpub_format(void) {
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
