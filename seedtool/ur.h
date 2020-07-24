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


bool test_cbor(void);


#endif
