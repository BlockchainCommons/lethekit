// Copyright © 2020 Blockchain Commons, LLC

#ifndef _UR_H
#define _UR_H

/**
 * @brief       minimal CBOR encode implementation
 * @param[in]   byte_in: binary string to be encoded
 * @param[in]   size_in: size of byte_in
 * @param[out]  byte_out: cbor encoded word
 * @return      size of cbor encoded string
 * @pre         allocate at least 3 bytes more for byte_out than byte_in
 */
size_t cbor_encode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out);

/**
 * @brief       minimal CBOR decode implementation
 * @param[in]   byte_in: cbor string to be decoded
 * @param[in]   size_in: size of byte_in in bytes (max 2^16)
 * @param[out]  byte_out: cbor decoded string
 * @return      size of cbor decoded string
 * @pre         allocate the same size for byte_out and byte_in
 */
size_t cbor_decode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out);

/**
 * @brief       encode as single part ur text
 * @param[in]   ur_type: UR type
 * @param[in]   cbor: cbor encoded string
 * @param[in]   cbor_size: size of cbor in bytes (max 2^16)
 * param[out]   ur_string: ur encoded string
 * @return      true on success
 */
bool ur_encode(String ur_type, uint8_t *cbor, uint32_t cbor_size, String &ur_string);


#endif
