// Copyright © 2020 Blockchain Commons, LLC

#include "ur.h"
#include "util.h"
#include "bc-crypto-base.h"
#include "bc-bytewords.h"

// source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md
//         f68d54efd0cf6f9943801412e273167c558c8189 (Single part UR only)

size_t cbor_encode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out, uint32_t byte_out_size)
{
    size_t size_out = 0;

    if (byte_out_size - size_in < 3) {
        Serial.println("cbor_encode: allocate more space!");
        return 0;
    }

    if (size_in <= 23) {
        byte_out[0] = 0x40 + (uint8_t)size_in;
        size_out = 1;
    }
    else if(size_in >= 24 && size_in <= 255) {
        byte_out[0] = 0x58;
        byte_out[1] = (uint8_t)size_in;
        size_out = 2;
    }
    else if(size_in >= 256 && size_in <= 65535) {
        byte_out[0] = 0x59;
        byte_out[1] = ((uint8_t *)&size_in)[1];
        byte_out[2] = ((uint8_t *)&size_in)[0];
        size_out = 3;
    }
    else {
        Serial.println("cbor_encode: size too large!");
        return 0;
    }

    memcpy(byte_out + size_out, byte_in, size_in);

    return size_out + size_in;
}

size_t cbor_decode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out, uint32_t byte_out_size)
{
    size_t size_out = 0;

    if (size_in <= 23 + 1) {
        byte_out[0] = 0x40 + (uint8_t)size_in;
        size_out = 1;
    }
    else if(size_in >= 24 + 2 && size_in <= 255 + 2) {
        byte_out[0] = 0x58;
        byte_out[1] = (uint8_t)size_in;
        size_out = 2;
    }
    else if(size_in >= 256 + 3 && size_in <= 65535 + 3) {
        byte_out[0] = 0x59;
        byte_out[1] = ((uint8_t *)&size_in)[1];
        byte_out[2] = ((uint8_t *)&size_in)[0];
        size_out = 3;
    }
    else {
        Serial.println("cbor_decode: size too large!");
        return 0;
    }

    memcpy(byte_out, byte_in + size_out, size_in - size_out);

    return size_in - size_out;
}

bool ur_encode(String ur_type, uint8_t *cbor, uint32_t cbor_size, String &ur_string)
{
    // Encode cbor payload as bytewords
    char *payload_bytewords = bytewords_encode(bw_minimal, cbor, cbor_size);
    if(payload_bytewords == NULL) {
      Serial.println("ur_encode bytewords failed.\n");
      return false;
    }

    ur_string = "ur:" + ur_type + "/" + String(payload_bytewords);

    free(payload_bytewords);

    return true;
}
