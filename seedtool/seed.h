// Copyright © 2020 Blockchain Commons, LLC

#ifndef SEED_H
#define SEED_H

#include <stdint.h>

#include <bc-bip39.h>

#define BIP39_SEED_LEN_512 64

enum format {
  qr_ur,
  ur,
  text
};

class Seed {
public:
    static size_t const SIZE = 16;

    enum format display_format = qr_ur;

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

    uint8_t mnemonic_seed[BIP39_SEED_LEN_512];

    BIP39Seq();

    BIP39Seq(Seed const * seed);

    ~BIP39Seq();

    void set_word(size_t ndx, uint16_t word);

    // Return the char string for a dictionary index.
    static String get_dict_string(size_t ndx);

    // Return the word for a word list index.
    uint16_t get_word(size_t ndx) const;

    // Return the char string for a list index.
    String get_string(size_t ndx);

    // Returns NULL if restore fails (bad BIP39 checksum).
    Seed * restore_seed() const;

private:
    void* ctx;
    // we need to have full menmonic words saved as string
    // which is needed to calculate mnemonic seed
    String get_mnemonic_as_string();
    // menmonic seed is needed for bip32 root key
    // len of the returned bytes is BIP39_SEED_LEN_512
    bool calc_mnemonic_seed();
};

class SLIP39ShareSeq {
public:
    static size_t const MAX_SHARES = 16;
    static size_t const WORDS_PER_SHARE = 20;

    enum format display_format = text;

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

/**
 *  This function is taken from libwally. We cannot import libwally_bip39 because it is clashing with
 *  bc-bip39.h. bc-bip39.h should be deprecated and replaced with libwally_bip39. To recompile libwally
 *  with libwally_bip39 uncomment the lines in blacklist array in scripts/libwally_build.py
 *  See https://github.com/BlockchainCommons/bc-bip39/pull/19
 */
int bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase,
                            unsigned char *bytes_out, size_t len,
                            size_t *written);

#endif // SEED_H
