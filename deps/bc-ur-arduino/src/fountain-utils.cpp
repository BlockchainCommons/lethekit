//
//  fountain-utils.cpp
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

//#include <ArduinoSTL.h>
//#include <Arduino.h>
#include "fountain-utils.hpp"
#include "random-sampler.hpp"
#include "utils.hpp"
#include <set>

using namespace std;

namespace ur_arduino {

size_t choose_degree(size_t seq_len, Xoshiro256& rng) {
    std::vector<double> degree_probabilities;
    for(int i = 1; i <= seq_len; i++) {
        degree_probabilities.push_back(1.0 / i);
    }
    auto degree_chooser = RandomSampler(degree_probabilities);
    //return degree_chooser.next([&]() { return rng.next_double(); }) + 1;
    return degree_chooser.next(rng.next_double(), rng.next_double()) + 1;
}

std::set<size_t> choose_fragments(uint32_t seq_num, size_t seq_len, uint32_t checksum) {
    // The first `seq_len` parts are the "pure" fragments, not mixed with any
    // others. This means that if you only generate the first `seq_len` parts,
    // then you have all the parts you need to decode the message.
    if(seq_num <= seq_len) {
        //return std::set<size_t>({seq_num - 1});
        std::set<std::size_t> retval;
        retval.insert(seq_num - 1);
        return retval;
    } else {
        //auto seed = join(vector<uint8_t>({int_to_bytes(seq_num), int_to_bytes(checksum)}));
        ByteVector s = int_to_bytes(seq_num);
        ByteVector c = int_to_bytes(checksum);
        ByteVector seed;

        for (int i=0; i < s.size(); i++)
            seed.push_back(s[i]);

        for (int i=0; i < c.size(); i++)
            seed.push_back(c[i]);

        auto rng = Xoshiro256(seed);
        auto degree = choose_degree(seq_len, rng);
        std::vector<size_t> indexes;
        indexes.reserve(seq_len);
        for(int i = 0; i < seq_len; i++) { indexes.push_back(i); }
        auto shuffled_indexes = shuffled(indexes, rng);
        return std::set<size_t>(shuffled_indexes.begin(), shuffled_indexes.begin() + degree);
    }
}

}
