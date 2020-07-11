#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"

class Keystore
{
  public:
    ext_key root;
    bool update(uint8_t *seed, size_t len);
    bool get_xpub(const char *path, ext_key *root, ext_key *key_out);

  private:
    int res;
};


#endif
