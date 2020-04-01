// Copyright © 2020 Blockchain Commons, LLC

#include <assert.h>
#include <stdarg.h>

extern "C" {
#include <bc-slip39.h>
#include <wordlist-english.h>
}

#include "util.h"
#include "seed.h"

namespace seed_internal {

} // namespace seed_internal

Seed * Seed::from_rolls(String const & rolls) {
    using namespace seed_internal;
    
    // Convert supplied entropy into master secret.
    Sha256Class sha256;
    sha256.init();
    for(uint8_t ii=0; ii < rolls.length(); ii++) {
        sha256.write(rolls[ii]);
    }
    return new Seed(sha256.result(), SIZE);
}

Seed::Seed(uint8_t const * i_data, size_t len) {
    using namespace seed_internal;
    
    assert(len == sizeof(data));
    memcpy(data, i_data, len);
}

void Seed::log() const {
    serial_printf("seed: ");
    for (int ii = 0; ii < sizeof(data); ++ii)
        serial_printf("%02x", (int) data[ii]);
    serial_printf("\n");
}

BIP39Seq * BIP39Seq::from_words(uint16_t * words) {
    BIP39Seq * retval = new BIP39Seq();
    for (size_t ii = 0; ii < WORD_COUNT; ++ii)
        retval->set_word(ii, words[ii]);
    return retval;
}

BIP39Seq::BIP39Seq(Seed const * seed) {
    using namespace seed_internal;

    bip39.setPayloadBytes(Seed::SIZE);
    bip39.setPayload(Seed::SIZE, const_cast<uint8_t *>(seed->data));
}

Seed * BIP39Seq::restore_seed() const {
    return bip39.verifyChecksum()
        ? new Seed(bip39.getPayload(), Seed::SIZE)
        : NULL;
}

SLIP39ShareSeq * SLIP39ShareSeq::from_seed(Seed const * seed,
                                           size_t thresh,
                                           size_t nshares,
                                           void(*randgen)(uint8_t *, size_t)) {
    char * password = "";
    uint8_t group_threshold = 1;
    uint8_t group_count = 1;
    group_descriptor group = { thresh, nshares, NULL };
    group_descriptor groups[] = { group };

    uint8_t iteration_exponent = 0;

    uint32_t words_in_each_share = 0;
    size_t shares_buffer_size = 1024;
    uint16_t shares_buffer[shares_buffer_size];

    int rv = slip39_generate(group_threshold,
                             groups,
                             group_count,
                             seed->data,
                             Seed::SIZE,
                             password,
                             iteration_exponent,
                             &words_in_each_share,
                             shares_buffer,
                             shares_buffer_size,
                             randgen);
    assert(rv == nshares);
    assert(words_in_each_share == WORDS_PER_SHARE);

    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    for (size_t ii = 0; ii < nshares; ++ii)
        slip39->add_share(shares_buffer + (ii * WORDS_PER_SHARE));
    return slip39;
}

SLIP39ShareSeq::SLIP39ShareSeq() {
    nshares = 0;
    memset(shares, 0x00, sizeof(shares));
}

SLIP39ShareSeq::~SLIP39ShareSeq() {
    for (size_t ii = 0; ii < nshares; ++ii)
        free(shares[ii]);
}

size_t SLIP39ShareSeq::add_share(uint16_t const * share) {
    assert(nshares < MAX_SHARES);
    size_t sharesz = sizeof(uint16_t) * WORDS_PER_SHARE;
    shares[nshares] = (uint16_t *) malloc(sharesz);
    memcpy(shares[nshares], share, sharesz);
    return nshares++;
}

uint16_t const * SLIP39ShareSeq::get_share(size_t ndx) const {
    assert(ndx < nshares);
    return shares[ndx];
}

void SLIP39ShareSeq::set_share(size_t ndx, uint16_t const * share) {
    assert(ndx < nshares);
    if (shares[ndx])
        free(shares[ndx]);
    size_t sharesz = sizeof(uint16_t) * WORDS_PER_SHARE;
    shares[ndx] = (uint16_t *) malloc(sharesz);
    memcpy(shares[nshares], share, sharesz);
}

char * SLIP39ShareSeq::get_share_strings(size_t ndx) const {
    assert(ndx < nshares);
    return slip39_strings_for_words(shares[ndx], WORDS_PER_SHARE);
}

char const * SLIP39ShareSeq::get_share_word(size_t sndx, size_t wndx) const {
    assert(sndx < nshares);
    assert(wndx < WORDS_PER_SHARE);
    return slip39_string_for_word(shares[sndx][wndx]);
}

Seed * SLIP39ShareSeq::restore_seed() const {
    uint8_t seed_data[Seed::SIZE];
    char * password = "";
    last_rv = slip39_combine(const_cast<const uint16_t **>(shares),
                             WORDS_PER_SHARE,
                             nshares,
                             password,
                             NULL,
                             seed_data,
                             sizeof(seed_data));
    return last_rv < 0 ? NULL : new Seed(seed_data, sizeof(seed_data));
}

#if 0

#define BIP39_WORD_COUNT	12

// This will be 256 when we support multiple levels.
#define MAX_SLIP39_SHARES	16

#define SLIP39_WORDS_IN_EACH_SHARE	20

// NOTE - Since the arduino build system concatenates all of the ino
// files together it is important to use named namespaces and place
// the "using namespace xxx" directives inside the functions.
namespace seed_internal {

uint8_t g_master_secret[16];

Bip39 g_bip39;

int g_generate_slip39_thresh;
int g_generate_slip39_nshares;
char** g_generate_slip39_shares = NULL;

void free_slip39_generate_shares() {
    if (g_generate_slip39_shares) {
        for (int ndx = 0; g_generate_slip39_shares[ndx]; ++ndx)
            free(g_generate_slip39_shares[ndx]);
        free(g_generate_slip39_shares);
        g_generate_slip39_shares = NULL;
    }
}


} // namespace seed_internal

void seed_reset_state() {
    using namespace seed_internal;
    
    // Clear the master secret.
    memset(g_master_secret, '\0', sizeof(g_master_secret));

    // Clear the BIP39 mnemonic (regenerate on all zero secret)
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *) g_master_secret);

    free_slip39_generate_shares();
}

void seed_from_rolls(String const &rolls) {
    using namespace seed_internal;
    
    // Convert supplied entropy into master secret.
    Sha256Class sha256;
    sha256.init();
    for(uint8_t ii=0; ii < rolls.length(); ii++) {
        sha256.write(rolls[ii]);
    }
    memcpy(g_master_secret, sha256.result(), sizeof(g_master_secret));

    seed_generate_bip39_words();
}

void seed_generate_bip39_words() {
    using namespace seed_internal;
    
    // Generate the BIP39 mnemonic for this secret.
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *)g_master_secret);
}

int seed_restore_bip39_words(uint16_t* words, size_t nwords) {
    using namespace seed_internal;
    
    for (int ii = 0; ii < nwords; ++ii)
        g_bip39.setWord(ii, words[ii]);
    if (g_bip39.verifyChecksum()) {
        memcpy(g_master_secret, g_bip39.getPayload(), 16);
        return 0;
    }
    return 1;
}

void seed_generate_slip39_shares(size_t thresh, size_t nshares,
                                 void(*randgen)(uint8_t *, size_t)) {
    using namespace seed_internal;
    
    // If there are already shares, free them.
    free_slip39_generate_shares();

    char* password = "";
    uint8_t group_threshold = 1;
    uint8_t group_count = 1;
    group_descriptor group = { thresh, nshares, NULL };
    group_descriptor groups[] = { group };

    uint8_t iteration_exponent = 0;

    uint32_t words_in_each_share = 0;
    size_t shares_buffer_size = 1024;
    uint16_t shares_buffer[shares_buffer_size];

    int rv = slip39_generate(group_threshold,
                             groups,
                             group_count,
                             g_master_secret,
                             sizeof(g_master_secret),
                             password,
                             iteration_exponent,
                             &words_in_each_share,
                             shares_buffer,
                             shares_buffer_size,
                             randgen);
    assert(rv == nshares);
    assert(words_in_each_share == SLIP39_WORDS_IN_EACH_SHARE);

    // Success, store the shares.
    g_generate_slip39_thresh = thresh;
    g_generate_slip39_nshares = nshares;
    g_generate_slip39_shares =
        (char **) malloc(sizeof(char *) * (nshares+1));
    for (int ndx = 0; ndx < nshares; ++ndx) {
        uint16_t* words = shares_buffer + (ndx * words_in_each_share);
        g_generate_slip39_shares[ndx] =
            slip39_strings_for_words(words, words_in_each_share);
    }
    g_generate_slip39_shares[nshares] = NULL; // sentinel
}

int seed_combine_slip39_shares() {
    using namespace seed_internal;
    
    uint8_t ms[16];
    char* password = "";
    uint32_t words_in_each_share = SLIP39_WORDS_IN_EACH_SHARE;
    size_t selected_shares_len = g_restore_slip39_nshares;
    uint16_t* selected_shares_words[selected_shares_len];

    for (int ii = 0; ii < selected_shares_len; ++ii) {
        uint16_t* words_buf = (uint16_t *)
            malloc(sizeof(uint16_t) * words_in_each_share);
        for (int jj = 0; jj < words_in_each_share; ++jj)
            words_buf[jj] = 0;
        selected_shares_words[ii] = words_buf;
        char* string = g_restore_slip39_shares[ii];
        slip39_words_for_strings(string, words_buf, words_in_each_share);
    }

    int rv = slip39_combine(
                            (const uint16_t **) selected_shares_words,
                            words_in_each_share,
                            selected_shares_len,
                            password,
                            NULL,
                            ms,
                            sizeof(ms)
                            );

    for(int ii = 0; ii < selected_shares_len; ii++) {
        free(selected_shares_words[ii]);
    }

    // If we were successful update the master secret.
    if (rv > 0) {
        memcpy(g_master_secret, ms, sizeof(ms));
    }

    return rv;
}

bool seed_master_secret_equal(uint8_t const* secret, size_t len) {
    using namespace seed_internal;
    assert(len == sizeof(g_master_secret));
    return memcmp(g_master_secret, secret, len) == 0;
}

bool seed_bip39_strings_equal(char** const ref, size_t nstrings) {
    using namespace seed_internal;
    assert(nstrings == BIP39_WORD_COUNT);
    for (int ii = 0; ii < BIP39_WORD_COUNT; ++ii)
        if (strcmp(g_bip39.getMnemonic(g_bip39.getWord(ii)), ref[ii]) != 0)
            return false;
    return true;
}

bool seed_slip39_shares_equal(char** const shares, size_t nshares) {
    using namespace seed_internal;
    assert(nshares == g_generate_slip39_nshares);
    for (int ii = 0; ii < nshares; ++ii)
        if (strcmp(g_generate_slip39_shares[ii], shares[ii]) != 0)
            return false;
    return true;
}

#endif
