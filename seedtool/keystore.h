#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"

class Keystore
{
  public:
    ext_key root;
    Keystore(void);
    bool update(uint8_t *seed, size_t len);
    bool get_xpub(const char *path, ext_key *root, ext_key *key_out);
    void set_derivation_path(String path = "m/84h/1h/0h");
    String get_derivation_path(void);
    bool derivation_path_from_str(const char *path);

  private:
    int res;
    String derivation_path;
    size_t derivationLen;
    uint32_t * derivation;
};


#endif
