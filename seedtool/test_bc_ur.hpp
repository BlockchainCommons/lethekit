
#ifndef TEST_BC_UR_H
#define TEST_BC_UR_H

#include <ArduinoSTL.h>
#include "util.h"
#include "bc-ur.hpp"

using namespace ur_arduino;
using namespace std;

std::vector<uint8_t> make_message(size_t len, const std::string& seed = "Wolf") {
    auto rng = Xoshiro256(seed);
    return rng.next_data(len);
}

class UR make_message_ur(size_t len, const std::string& seed = "Wolf") {
    auto message = make_message(len, seed);
    //CborLite::encodeBytes(cbor, message);

    uint8_t *cbor_arr = &message[0];
    CborDynamicOutput output;
    CborWriter writer(output);
    writer.writeBytes(cbor_arr, len);

    ByteVector cbor(output.getData(), output.getData() + output.getSize());

    return UR("bytes", cbor);
}

bool test_bc_ur(void);


#endif
