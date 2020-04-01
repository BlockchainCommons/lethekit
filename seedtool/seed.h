// Copyright © 2020 Blockchain Commons, LLC

#ifndef SEED_H
#define SEED_H

#include <stdint.h>

#include <bip39.h>

class Seed {
public:
    static size_t const SIZE = 16;

    static Seed * from_rolls(String const & rolls);

    Seed(uint8_t const * data, size_t len);

    bool operator==(Seed const & other) const {
        return memcmp(data, other.data, SIZE) == 0;
    }

    bool operator!=(Seed const & other) const {
        return memcmp(data, other.data, SIZE) != 0;
    }

    uint8_t data[SIZE];
};

class BIP39Seq {
public:
    static size_t const WORD_COUNT = 12;

    static BIP39Seq * from_words(uint16_t * words);
    
    BIP39Seq() { bip39.setPayloadBytes(Seed::SIZE); }
    
    BIP39Seq(Seed const * seed);

    void set_word(size_t ndx, uint16_t word) {
        bip39.setWord(ndx, word);
    }

    uint16_t get_word(size_t ndx) const {
        return bip39.getWord(ndx);
    }

    char const * get_mnemonic(size_t ndx) {
        return bip39.getMnemonic(bip39.getWord(ndx));
    }

    // Returns NULL if restore fails (bad BIP39 checksum).
    Seed * restore_seed() const ;

    Bip39 bip39;
};

class SLIP39ShareSeq {
public:
    static size_t const MAX_SHARES = 16;
    static size_t const WORDS_PER_SHARE = 20;
    
    static SLIP39ShareSeq * from_seed(Seed const * seed,
                                      size_t thresh,
                                      size_t nshares,
                                      void(*randgen)(uint8_t *, size_t));

    SLIP39ShareSeq();

    ~SLIP39ShareSeq();

    // Adds a copy of the argument, returns the share index.
    size_t add_share(uint16_t const * share);

    // Replace a share's value with a copy of the argument.
    void set_share(size_t ndx, uint16_t const * share);

    // Read-only, don't free returned value.
    uint16_t const * get_share(size_t ndx) const;
    
    // Returns NULL if restore fails, use last_error for diagnostic.
    Seed * restore_seed() const;

    int last_restore_error();

    size_t nshares;
    
    uint16_t * shares[MAX_SHARES];

    mutable int last_rv;
};

    
#if 0                 
                 
void seed_reset_state();

void seed_from_rolls(String const &rolls);

void seed_generate_bip39_words();

int seed_restore_bip39_words(uint16_t* words, size_t nwords);

void seed_generate_slip39_shares(size_t thres, size_t nshares,
                                 void(*randgen)(uint8_t *, size_t));

int seed_combine_slip39_shares(char** const shares, size_t nshares);

bool seed_master_secret_equal(uint8_t const* secret, size_t len);

bool seed_bip39_strings_equal(char** const ref, size_t nstrings);

bool seed_slip39_shares_equal(char** const shares, size_t nshares);

#endif
    
#endif // SEED_H
