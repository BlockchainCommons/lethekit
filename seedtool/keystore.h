#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"
#include "network.h"

#define HARDENED_INDEX 0x80000000
#define MAX_DERIVATION_PATH_LEN 10*sizeof(uint32_t)

enum stdDerivation {
  SINGLE_NATIVE_SEGWIT,
  SINGLE_NESTED_SEGWIT,
  MULTISIG_NATIVE_SEGWIT
};


/**
 * @brief  * This class initializes/updates bip32 root key
 *         * It is also a database that stores xpub configuration set by user.
 *           @FIXME: DB part should be moved into UI code
*/
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
     * @brief  get xpriv key based on the last derivation_path set by user
     *         or default one if none set
     * @pre    update_root_key()
     */
    bool get_xpriv(ext_key *key_out);

    /**
     * @brief  convert hdkey to base58
     */
    bool xpriv_to_base58(ext_key *key, char **output, bool slip132);

    /**
     * @brief  convert hdkey to base58
     */
    bool xpub_to_base58(ext_key *key, char **output, bool slip132);

    /**
     *  @brief calculate derivation path from string
     */
    bool calc_derivation_path(const char *path, uint32_t *derivation, uint32_t &derivation_len);

    /**
     *  @brief checks derivation path and optionally saves it
     */
    bool check_derivation_path(const char *path, bool save);

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
     * @brief check if bip32 index is hardened
     */
    bool is_bip32_indx_hardened(uint32_t indx)  {
        return (indx & BIP32_INITIAL_HARDENED_CHILD) >> 31;
    };

    size_t derivationLen;
    uint32_t derivation[MAX_DERIVATION_PATH_LEN];
    const char* default_derivation ="m/84h/1h/0h";
    String derivation_path;
    uint32_t fingerprint;
    ext_key root;

  private:
    int res;
    bool standard_derivation_path;
    stdDerivation std_derivation_path;
};

extern Keystore keystore;

#endif
