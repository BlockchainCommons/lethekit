#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"

//# of elements in xpub_encoding_e
#define XPUB_ENCODINGS 4

enum xpub_encoding_e {
  BASE58 = 0,
  QR_BASE58,
  UR,
  QR_UR
};

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
    void set_xpub_format(xpub_encoding_e format=QR_BASE58);
    xpub_encoding_e get_xpub_format(void);
    String get_xpub_format_as_string(void);

  private:
    int res;
    String derivation_path;
    size_t derivationLen;
    uint32_t * derivation;
    xpub_encoding_e format;
};

#endif
