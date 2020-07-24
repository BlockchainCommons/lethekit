// Copyright © 2020 Blockchain Commons, LLC

#include "ur.h"
#include "util.h"
#include "bc-crypto-base.h"
#include "bc-bytewords.h"
#include "CborEncoder.h"
#include "wally_core.h"

// source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md
//         f68d54efd0cf6f9943801412e273167c558c8189 (Single part UR only)


size_t ur_encode_crypto_seed(uint8_t *seed, size_t len, uint8_t **buff_out, uint32_t *unix_timestamp=NULL) {

    CborDynamicOutput output;
    CborWriter writer(output);

    if (unix_timestamp != NULL)
        writer.writeMap(2);
    else
        writer.writeMap(1);
    writer.writeInt(1);
    writer.writeBytes(seed, len);

    if (unix_timestamp != NULL) {
        writer.writeInt(2);
        writer.writeTag(100);
        writer.writeInt(*unix_timestamp);
    }

    *buff_out = output.getData();

    return output.getSize();
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

bool test_cbor(void) {
    // https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-006-urtypes.md#exampletest-vector-1
    uint8_t payload[] = {0xC7, 0x09, 0x85, 0x80, 0x12, 0x5E, 0x2A, 0xB0, 0x98, 0x12, 0x53, 0x46, 0x8B, 0x2D, 0xBC, 0x52};
    uint32_t timestamp = 18394;
    char *expected = "a20150c7098580125e2ab0981253468b2dbc5202d8641947da";
    int ret;
    uint8_t *cbor;
    char *cbor_as_hex;

    size_t len = ur_encode_crypto_seed(payload, sizeof(payload), &cbor, &timestamp);

    ret = wally_hex_from_bytes(cbor, len, &cbor_as_hex);
    if (ret != WALLY_OK) {
        Serial.println("libwally: hex_from_bytes failed");
        return false;
    }

    if (strcmp(expected, cbor_as_hex) != 0) {
        serial_printf("test_cbor encodeing failed\n");
        Serial.println(expected);
        Serial.println(cbor_as_hex);
        return false;
    }

    return true;
}
