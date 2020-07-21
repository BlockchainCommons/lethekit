#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"

//# of elements in xpub_encoding_e
#define XPUB_ENCODINGS 4

enum xpubEnc {
  BASE58 = 0,
  QR_BASE58,
  UR,
  QR_UR
};

class Keystore
{
  public:
    Keystore(void);

    /**
     * @brief  Whenever seed changes, call this to update bip32 root key
     */
    bool update_root_key(uint8_t *seed, size_t len);

    /**
     * @brief  use when a new derivation path is enetered by user
     */
    void set_derivation_path(String path = "m/84h/1h/0h");

    /**
     * @brief  read the last derivation path set/saved by user or default one
     *         if none entered yet
     */
    String get_derivation_path(void);

    /**
     * @brief  get xpub key based on the last derivation_path set by user
     *         or default one if none set
     * @pre    update_root_key()
     */
    bool get_xpub(ext_key *key_out);

    /**
     *  @brief checks if derivation path is valid, and makes it available as integers
     *         in derivationLen and derivation for further processing
     */
    bool check_derivation_path(const char *path);

    /**
     * @brief  save xpub format entered by user
     */
    void set_xpub_format(xpubEnc format=QR_BASE58);

    /**
     * @brief  get the last xpub format entered by user or default one if none
     *         entered yet
     */
    xpubEnc get_xpub_format(void);

    /**
     * @brief  get the last xpub format entered by user in string format
     *         or default one if none entered yet
     */
    String get_xpub_format_as_string(void);

  private:
    ext_key root;
    int res;
    String derivation_path;
    size_t derivationLen;
    uint32_t * derivation;
    xpubEnc format;
};

#endif