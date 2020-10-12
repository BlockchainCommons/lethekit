//
//  fountain-encoder.cpp
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#include "fountain-encoder.hpp"
#include <assert.h>
#include <cmath>
//#include <optional>
#include <vector>
//#include <limits>
//#include "cbor-lite.hpp"
#include <ArduinoSTL.h>
#include <Arduino.h>
#include <string>
#include "CborEncoder.h"
#include "CborDecoder.h"

//#include "util.h"


using namespace std;

namespace ur_arduino {

class CborListen_Part : public CborListener {
  public:
    void OnInteger(int32_t value){Serial.println("integer"); integers[i] = value; i++; Serial.println(value); };
    void OnBytes(unsigned char *data, unsigned int size) {Serial.println("bytes");};
    void OnString(String &str) {Serial.println("string");};
    void OnArray(unsigned int size) {Serial.println("array");};
    void OnMap(unsigned int size) {Serial.println("map");};
    void OnTag(uint32_t tag) {Serial.println("tag");};
    void OnSpecial(uint32_t code) {Serial.println("tag");};
    void OnError(const char *error) {Serial.println("error");};
    
    // we are gonna collect 4 integers: seqnum, seqlen, msglen and checksum
    size_t i = 0;
    uint32_t integers[4];
};

size_t FountainEncoder::find_nominal_fragment_length(size_t message_len, size_t min_fragment_len, size_t max_fragment_len) {
    assert(message_len > 0);
    assert(min_fragment_len > 0);
    assert(max_fragment_len >= min_fragment_len);
    auto max_fragment_count = message_len / min_fragment_len;
    size_t fragment_len = 0;
    for(size_t fragment_count = 1; fragment_count <= max_fragment_count; fragment_count++) {
        fragment_len = size_t(ceil(double(message_len) / fragment_count));
        if(fragment_len <= max_fragment_len) {
            break;
        }
    }
    assert(fragment_len > 0);
    return fragment_len;
}

vector<ByteVector> FountainEncoder::partition_message(ByteVector message, size_t fragment_len) {
    auto remaining = message;
    vector<ByteVector> fragments;
    while(!remaining.empty()) {
        auto a = split(remaining, fragment_len);
        auto fragment = a.first;
        remaining = a.second;
        auto padding = fragment_len - fragment.size();
        while(padding > 0) {
            fragment.push_back(0);
            padding--;
        }
        fragments.push_back(fragment);
    }
    return fragments;
}

FountainEncoder::Part::Part(const ByteVector& cbor) {
        Serial.println("*******   Part::Part(const ByteVector& cbor)    ********");

    uint8_t* cbor_arr = (uint8_t *)&cbor[0];
    CborInput input(cbor_arr, cbor.size());
    CborReader reader(input);
    CborListen_Part listener;
    reader.SetListener(listener);
    reader.Run();

    seq_num_ = listener.integers[0];
    seq_len_ = listener.integers[1];
    message_len_ = listener.integers[2];
    checksum_ = listener.integers[3];

    Serial.println("seq_num_:");
    Serial.println(seq_num_);

    Serial.println("seq_len_:");
    Serial.println(seq_len_);

    Serial.println("message_len_:");
    Serial.println(message_len_);

    Serial.println("checksum_:");
    Serial.println(checksum_);
}

ByteVector FountainEncoder::Part::cbor() const {
    //using namespace CborLite;  // TODO!!!!!!!!!!!!!!!!!!1

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeArray(5);
    writer.writeInt(seq_num());
    writer.writeInt((uint32_t)seq_len());
    writer.writeInt((uint32_t)message_len());
    writer.writeInt(checksum());

    ByteVector dat = data();
    uint8_t *d = &dat[0];
    writer.writeBytes(d, data().size());

    ByteVector result(output.getData(), output.getData() + output.getSize());

    return result;
}

FountainEncoder::FountainEncoder(const ByteVector& message, size_t max_fragment_len, uint32_t first_seq_num, size_t min_fragment_len) {
    assert(message.size() <= UINT32_MAX);
    message_len_ = message.size();
    checksum_ = crc32_int(message);
    fragment_len_ = find_nominal_fragment_length(message_len_, min_fragment_len, max_fragment_len);
    fragments_ = partition_message(message, fragment_len_);
    seq_num_ = first_seq_num;
}

ByteVector FountainEncoder::mix(const PartIndexes& indexes) const {
    ByteVector result(fragment_len_, 0);
    for(auto index: indexes) { xor_into(result, fragments_[index]); }
    return result;
}

FountainEncoder::Part FountainEncoder::next_part() {
    seq_num_ += 1; // wrap at period 2^32
    auto indexes = choose_fragments(seq_num_, seq_len(), checksum_);
    auto mixed = mix(indexes);
    return Part(seq_num_, seq_len(), message_len_, checksum_, mixed);
}

string FountainEncoder::Part::description() const {
    string seq_num_str = (String(seq_num_)).c_str();
    string seq_len_str = (String(seq_len_)).c_str();
    string message_len_str = (String(message_len_)).c_str();
    string checksum_str = (String(checksum_)).c_str();
 
    return "seqNum:" + seq_num_str + ", seqLen:" + seq_len_str + ", messageLen:" + message_len_str + ", checksum:" + checksum_str + ", data:" + data_to_hex(data_);
}

}
