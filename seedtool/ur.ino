// Copyright © 2020 Blockchain Commons, LLC

#include "ur.h"
#include "util.h"
#include "bc-bech32.h"
#include <bc-crypto-base.h>

// source: https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-005-ur.md

/* allocate at least 5 bytes more for byte_out than byte_in */
size_t cbor_encode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out, uint32_t byte_out_size)
{
    size_t size_out = 0;

    if (byte_out_size - size_in < 5)
        return 0;
    
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
    else if(size_in >= 65536 && size_in <= 0xffffffff-1) {
        byte_out[0] = 0x60;
        byte_out[1] = ((uint8_t *)&size_in)[3];
        byte_out[2] = ((uint8_t *)&size_in)[2];
        byte_out[3] = ((uint8_t *)&size_in)[1];
        byte_out[4] = ((uint8_t *)&size_in)[0];
        size_out = 5;
    }
    else {
        return 0;
    }
    
    memcpy(byte_out + size_out, byte_in, size_in);
    
    return size_out + size_in;
}

/* allocate at least 5 bytes more for byte_out than byte_in */
size_t cbor_decode(uint8_t *byte_in, uint32_t size_in, uint8_t *byte_out, uint32_t byte_out_size)
{
    size_t size_out = 0;

    if (byte_out_size - size_in < 5)
        return 0;
  
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
    else if(size_in >= 65536 + 5 && size_in <= 0xffffffff-1 + 5) {
        byte_out[0] = 0x60;
        byte_out[1] = ((uint8_t *)&size_in)[3];
        byte_out[2] = ((uint8_t *)&size_in)[2];
        byte_out[3] = ((uint8_t *)&size_in)[1];
        byte_out[4] = ((uint8_t *)&size_in)[0];
        size_out = 5;
    }
    else {
        return 0;
    }
    
    memcpy(byte_out, byte_in + size_out, size_in - size_out);
   
    return size_in - size_out;
}


/////////// source: https://github.com/micro-bitcoin/uBitcoin
////////// @FIXME: use libwally for these functions once integrated

uint8_t hexToVal(char c){
  if(c >= '0' && c <= '9'){
    return ((uint8_t)(c - '0')) & 0x0F;
  }
  if(c >= 'A' && c <= 'F'){
    return ((uint8_t)(c-'A'+10)) & 0x0F;
  }
  if(c >= 'a' && c <= 'f'){
    return ((uint8_t)(c-'a'+10)) & 0x0F;
  }
  return 0xFF;
}

size_t toHex(const uint8_t * array, size_t arraySize, char * output, size_t outputSize){
    // uint8_t * array = (uint8_t *) arr;
    if(outputSize < 2*arraySize){
        return 0;
    }
    memset(output, 0, outputSize);

    for(size_t i=0; i < arraySize; i++){
        output[2*i] = (array[i] >> 4) + '0';
        if(output[2*i] > '9'){
            output[2*i] += 'a'-'9'-1;
        }

        output[2*i+1] = (array[i] & 0x0F) + '0';
        if(output[2*i+1] > '9'){
            output[2*i+1] += 'a'-'9'-1;
        }
    }
    return 2*arraySize;
}

size_t fromHex(const char * hex, size_t hexLen, uint8_t * array, size_t arraySize){
    memset(array, 0, arraySize);
    // ignoring all non-hex characters in the beginning
    size_t offset = 0;
    while(offset < hexLen){
        uint8_t v = hexToVal(hex[offset]);
        if(v > 0x0F){ // if invalid char
            offset++;
        }else{
            break;
        }
    }
    hexLen -= offset;
    for(size_t i=0; i<hexLen/2; i++){
        uint8_t v1 = hexToVal(hex[offset+2*i]);
        uint8_t v2 = hexToVal(hex[offset+2*i+1]);
        if((v1 > 0x0F) || (v2 > 0x0F)){ // if invalid char stop parsing
            return i;
        }
        array[i] = (v1<<4) | v2;
    }
    return hexLen/2;
}

/////////////////


size_t ur_encode(String type, uint8_t *cbor, uint32_t cbor_size, String *ur_fragments, size_t max_fragments, size_t max_fragment_len)
{
    // Encode cbor payload as bc32
    char *payload_bc32 = bc32_encode(cbor, cbor_size);
    if(payload_bc32 == NULL) {
      serial_printf("BC32 encode fails.\n");
      return 0;
    }
    String payload_bc32_str = String(payload_bc32);
    free(payload_bc32);

    // Compute the SHA256 digest of the CBOR-encoded payload
    uint8_t digest[SHA256_DIGEST_LENGTH];
    sha256_Raw(cbor, cbor_size, digest);

    // Encode digest as BC32
    char *digest_bc32 = bc32_encode(digest, SHA256_DIGEST_LENGTH);
    if(digest_bc32 == NULL) {
      serial_printf("BC32 encode fails.\n");
      return 0;
    }
    String digest_bc32_str = String(digest_bc32);
    free(digest_bc32);

    if (payload_bc32_str.length() > max_fragments*max_fragment_len)
    {
      serial_printf("memory: string partitioning failed.\n");
      return 0;
    }

    // Partition the BC32-encoded payload into a sequence of fragments
    size_t fragments_len = 0;
    for (size_t i=0; i < max_fragments; i++) {
        ur_fragments[i] = payload_bc32_str.substring(i*max_fragment_len, i*max_fragment_len + max_fragment_len);
        if (ur_fragments[i].length() < max_fragment_len)
        {
          fragments_len = i + 1;
          break;
        }
    }

    // Prepend each fragment with a header that includes scheme, type, sequencing, and digest
    for (size_t i = 0; i < fragments_len; i++) {
      ur_fragments[i] = "ur:" + type + "/" + String(i+1) + "of" + String(fragments_len) + "/" +  \
                         digest_bc32_str + "/" + ur_fragments[i];
      ur_fragments[i].toUpperCase();
    }

    return fragments_len;
}


/* for testing purposes */
struct Data {
  uint8_t buff[200];
  size_t len;
};

typedef struct Data Data; 

bool test_ur()
{
    serial_printf("test_ur starting\n");

    // test vectors obtained from UR.nb: https://github.com/BlockchainCommons/Research/tree/master/papers/bcr-2020-005
    const char *payload_of_arbitrary_bytes =
                ("4596fd467c944dd595af9b6936592371eb5ee99175840eacb6762d6279f512079b22e9"
                 "a4621802ebef28f3d13345f504b26880d3e600ca788361ca4e9d531659c990025c1e12"
                 "4469f4f0074ac53d0d87b26c99d36e9a63c9d3d873b04b3bdfc8e60d340e");

    const char *payload_as_cbor_expected =
                ("58644596fd467c944dd595af9b6936592371eb5ee99175840eacb6762d6279f512079b"
                 "22e9a4621802ebef28f3d13345f504b26880d3e600ca788361ca4e9d531659c990025c"
                 "1e124469f4f0074ac53d0d87b26c99d36e9a63c9d3d873b04b3bdfc8e60d340e");

    Data arbitrary_bytes_raw;
    Data cbor_expected_raw;
    Data payload_cbor;

    // get payload_of_arbitrary_bytes in raw format
    arbitrary_bytes_raw.len = fromHex(payload_of_arbitrary_bytes, strlen(payload_of_arbitrary_bytes), arbitrary_bytes_raw.buff, sizeof(arbitrary_bytes_raw.buff));
    
    // cbor encode payload_of_arbitrary_bytes
    payload_cbor.len = cbor_encode(arbitrary_bytes_raw.buff, arbitrary_bytes_raw.len, payload_cbor.buff, sizeof(payload_cbor.buff));
    if (payload_cbor.len == 0) {
      serial_printf("cbor encode failed \n");
      return false;
    }
    
    // get payload_as_cbor_expected in raw format
    cbor_expected_raw.len = fromHex(payload_as_cbor_expected, strlen(payload_as_cbor_expected), cbor_expected_raw.buff, sizeof(cbor_expected_raw.buff));

    // compare payload_as_cbor_expected with calculated payload_as_cbor
    if (memcmp(cbor_expected_raw.buff, payload_cbor.buff, cbor_expected_raw.len) != 0)
    {
      serial_printf("cbor encode failed \n");
      return false;
    }

    const char *expected_result = ("UR:BYTES/1OF1/"
                                   "5Y69MT2LNMP0GWCJ65LL60TLGNZUSQ0HKC9573G22L5PT87YJNLQGL34HU/"
                                   "TPJYT9HAGE7FGNW4JKHEK6FKTY3HR667AXGHTPQW4JM8VTTZ0863YPUMYT56GCSCQT4772"
                                   "8N6YE5TAGYKF5GP5LXQR983QMPEF8F65CKT8YEQQJURCFYG6057QR543FAPKRMYMYE6DHF"
                                   "5C7F60V88VZT800U3ESDXS8QNC9R93");
    String ur_fragments[15];
    size_t size_fragments = ur_encode("bytes", payload_cbor.buff, payload_cbor.len, ur_fragments, 15, 400);

    if (size_fragments != 1) {
        serial_printf("cbor encode: size fragments \n");
        return false;
    }

    if (strcmp(expected_result, ur_fragments[0].c_str()) != 0)
    {
      Serial.println(ur_fragments[0]);
      serial_printf("ur encode failed 1\n");
      return false;
    }

    String expected_res[5] = {"UR:BYTES/1OF5/5Y69MT2LNMP0GWCJ65LL60TLGNZUSQ0HKC9573G22L5PT87YJNLQGL34HU/TPJYT9HAGE7FGNW4JKHEK6FKTY3HR667AXGHTPQW",
                              "UR:BYTES/2OF5/5Y69MT2LNMP0GWCJ65LL60TLGNZUSQ0HKC9573G22L5PT87YJNLQGL34HU/4JM8VTTZ0863YPUMYT56GCSCQT47728N6YE5TAGY",
                              "UR:BYTES/3OF5/5Y69MT2LNMP0GWCJ65LL60TLGNZUSQ0HKC9573G22L5PT87YJNLQGL34HU/KF5GP5LXQR983QMPEF8F65CKT8YEQQJURCFYG605",
                              "UR:BYTES/4OF5/5Y69MT2LNMP0GWCJ65LL60TLGNZUSQ0HKC9573G22L5PT87YJNLQGL34HU/7QR543FAPKRMYMYE6DHF5C7F60V88VZT800U3ESD",
                              "UR:BYTES/5OF5/5Y69MT2LNMP0GWCJ65LL60TLGNZUSQ0HKC9573G22L5PT87YJNLQGL34HU/XS8QNC9R93"};

    size_fragments = ur_encode("bytes", payload_cbor.buff, payload_cbor.len, ur_fragments, 15, 40);

    if (size_fragments != 5)
    {
        serial_printf("ur encode: fragment size incorrect \n");
        return false;
    }

    for (size_t i=0; i < size_fragments; i++) {
        if (expected_res[i] != ur_fragments[i]) {
            serial_printf("ur encode with partitioning failed\n");
            Serial.println(ur_fragments[i]);
            Serial.println(expected_res[i]);
            return false;
        }
    }

    serial_printf("test_ur finished\n");

    return true;
}
