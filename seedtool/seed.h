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

    void log() const;
    
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

    // Return the char string for a dictionary index.
    char const * get_dict_string(size_t ndx) {
        return bip39.getMnemonic(ndx);
    }

    // Return the word for a word list index.
    uint16_t get_word(size_t ndx) const {
        return bip39.getWord(ndx);
    }

    // Return the char string for a list index.
    char const * get_string(size_t ndx) {
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
    
    static bool verify_share_checksum(uint16_t const * share);
    
    static SLIP39ShareSeq * from_seed(Seed const * seed,
                                      uint8_t thresh,
                                      uint8_t nshares,
                                      void(*randgen)(uint8_t *, size_t));

    //  Read-only, don't free returned value.
    static char const * error_msg(int errval);

    SLIP39ShareSeq();

    ~SLIP39ShareSeq();

    size_t numshares() const { return nshares; }

    // Adds a copy of the argument, returns the share index.
    size_t add_share(uint16_t const * share);

    // Replace a share's value with a copy of the argument.
    void set_share(size_t ndx, uint16_t const * share);

    // Delete the specified share, compact gaps.
    void del_share(size_t ndx);

    // Read-only, don't free returned value.
    uint16_t const * get_share(size_t ndx) const;

    // Free the returned value!
    char * get_share_strings(size_t ndx) const;
    
    // Read only, don't free returned value.
    char const * get_share_word(size_t sndx, size_t wndx) const;
    
    // Returns NULL if restore fails, use last_error for diagnostic.
    Seed * restore_seed() const;

    int last_restore_error() { return last_rv; }

private:
    size_t nshares;
    
    uint16_t * shares[MAX_SHARES];

    mutable int last_rv;
};
    
#endif // SEED_H
