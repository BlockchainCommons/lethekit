#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"
#include "network.h"

#define MAX_DERIVATION_PATH_LEN 10*sizeof(uint32_t)

//# of elements in xpubEnc
#define XPUB_ENCODINGS 4

enum xpubEnc {
  BASE58 = 0,
  QR_BASE58,
  UR,
  QR_UR
};

enum stdDerivation {
  SINGLE_NATIVE_SEGWIT,
  SINGLE_NESTED_SEGWIT
};


class Keystore
{
  public:
    Keystore(void);

    /**
     * @brief  Whenever seed changes, call this to update bip32 root key
     */
    bool update_root_key(uint8_t *seed, size_t len, NetwtorkType network=network.get_network());

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
     * @brief  convert hdkey to base58
     */
    bool xpub_to_base58(ext_key *key, char **output);

    /**
     *  @brief checks if derivation path is valid, and makes it available as integers
     *         in derivationLen and derivation for further processing
     */
    bool check_derivation_path(const char *path);

    /**
     *  @brief checks derivation path and saves it
     */
    bool save_derivation_path(const char *path);

    /**
     *  @brief saves standard derivation path. Path can be NULL if only network changed
     *         which means we are only updating our stored derivation path
     *  @pre   when path==NULL, is_standard_derivation_path() must be true prior the call
     */
    bool save_standard_derivation_path(stdDerivation *path, NetwtorkType network);

    /**
     *  @brief is the last saved derivation path a standard one
     */
    bool is_standard_derivation_path(void);

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

    /**
     * @brief check if bip32 index is hardened
     */
    bool is_bip32_indx_hardened(uint32_t indx)  {
        return (indx & BIP32_INITIAL_HARDENED_CHILD) >> 31;
    };

    size_t derivationLen;
    uint32_t derivation[MAX_DERIVATION_PATH_LEN];
    const char* default_derivation ="m/84h/1h/0h";
    /* show xpub with derivation path (only for base58 format) */
    bool show_derivation_path;
    bool slip132;

  private:
    ext_key root;
    int res;
    String derivation_path;
    size_t derivationLen_tmp;
    uint32_t derivation_tmp[MAX_DERIVATION_PATH_LEN];
    xpubEnc format;
    bool standard_derivation_path;
};

extern Keystore keystore;

#endif
