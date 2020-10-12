//
//  xoshiro256.hpp
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef XOSHIRO256_HPP
#define XOSHIRO256_HPP

#include <stdint.h>

//#include <string>
#include "utils.hpp"
#include <ArduinoSTL.h>
#include <array>

typedef std::vector<uint8_t> ByteVector;
typedef std::vector<std::string> StringVector;

namespace ur_arduino {

class Xoshiro256 {
public:
    Xoshiro256(const std::array<uint64_t, 4>& a);
    Xoshiro256(const std::array<uint8_t, 32>& a);

    Xoshiro256(const ByteVector& bytes);
    Xoshiro256(const std::string& s);
    Xoshiro256(uint32_t crc32);

    uint64_t next();
    double next_double();
    uint64_t next_int(uint64_t low, uint64_t high);
    uint8_t next_byte();
    ByteVector next_data(size_t count);

    void jump();
    void long_jump();

private:
    uint64_t s[4];

    void set_s(const std::array<uint8_t, 32>& a);
    void hash_then_set_s(const ByteVector& bytes);
};

}

#endif // XOSHIRO256_HPP
