// Copyright © 2020 Blockchain Commons, LLC

#ifndef _UR_H
#define _UR_H

/**
 * @brief       encode as single part ur text
 * @param[in]   ur_type: UR type
 * @param[in]   cbor: cbor encoded string
 * @param[in]   cbor_size: size of cbor in bytes (max 2^16)
 * param[out]   ur_string: ur encoded string
 * @return      true on success
 */
bool ur_encode(String ur_type, uint8_t *cbor, uint32_t cbor_size, String &ur_string);
bool ur_encode_hd_pubkey_xpub(String &xpub_bytewords);
bool ur_encode_crypto_seed(uint8_t *seed, size_t seed_len, String &seed_bytewords, uint32_t *unix_timestamp=NULL);
bool ur_encode_slip39_share(SLIP39ShareSeq *slip39_generate, size_t share_wndx, String &ur);

bool test_ur(void);


#endif
