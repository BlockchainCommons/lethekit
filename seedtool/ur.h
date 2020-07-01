// Copyright © 2020 Blockchain Commons, LLC

#ifndef _UR_H
#define _UR_H

/**
 * @brief       minimal CBOR encode implementation
 * @param[in]   byte_in: binary string to be encoded
 * @param[in]   size_in: size of byte_in
 * @param[out]  byte_out: cbor encoded word
 * @return      size of cbor encoded string
 * @pre         allocate at least 5 bytes more for byte_out than byte_in
 */
size_t cbor_encode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out);

/**
 * @brief       minimal CBOR decode implementation
 * @param[in]   byte_in: cbor string to be decoded
 * @param[in]   size_in: size of byte_in
 * @param[out]  byte_out: cbor decoded string
 * @return      size of cbor decoded string
 * @pre         allocate at least 5 bytes more for byte_out than byte_in
 */
size_t cbor_decode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out);

/**
 * @brief       UR encode
 * @pre         max_fragment_len * max_fragments >= 4*cbor_size + 8
 * @param[in]   type: UR type
 * @param[in]   cbor: cbor encoded string
 * @param[in]   cbor_size: size of cbor
 * @param[out]  ur_fragments: ur encoded fragments
 * @param[in]   max_fragments: max number of fragments
 * @param[in]   max_fragment_len: max bytes in a fragment
 * @return      number of UR fragments produced
 */
size_t ur_encode(String type, uint8_t *cbor, uint32_t cbor_size, String *ur_fragments, size_t max_fragments, size_t max_fragment_len);


bool test_ur();


#endif
