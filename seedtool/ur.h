// Copyright © 2020 Blockchain Commons, LLC

#ifndef _UTIL_H
#define _UTIL_H

size_t cbor_encode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out);
size_t cbor_decode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out);

bool test_ur();


#endif
