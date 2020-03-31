// Copyright © 2020 Blockchain Commons, LLC

#include <bip39.h>

extern "C" {
#include <bc-slip39.h>
#include <wordlist-english.h>
}

#include "util.h"
#include "seed.h"

#define BIP39_WORD_COUNT	12

// This will be 256 when we support multiple levels.
#define MAX_SLIP39_SHARES	16

#define SLIP39_WORDS_IN_EACH_SHARE	20

namespace {

uint8_t g_master_secret[16];

Bip39 g_bip39;

int g_generate_slip39_thresh;
int g_generate_slip39_nshares;
char** g_generate_slip39_shares = NULL;

char* g_restore_slip39_shares[MAX_SLIP39_SHARES];
int g_restore_slip39_nshares = 0;
int g_restore_slip39_selected;

void free_slip39_generate_shares() {
    if (g_generate_slip39_shares) {
        for (int ndx = 0; g_generate_slip39_shares[ndx]; ++ndx)
            free(g_generate_slip39_shares[ndx]);
        free(g_generate_slip39_shares);
        g_generate_slip39_shares = NULL;
    }
}


void free_restore_shares() {
    for (int ndx = 0; ndx < g_restore_slip39_nshares; ++ndx) {
        free(g_restore_slip39_shares[ndx]);
        g_restore_slip39_shares[ndx] = NULL;
    }
    g_restore_slip39_nshares = 0;
}

void log_master_secret() {
    serial_printf("master secret: ");
    for (int ii = 0; ii < sizeof(g_master_secret); ++ii)
        serial_printf("%02x", (int) g_master_secret[ii]);
    serial_printf("\n");
}

} // end namespace

void seed_reset_state() {
    // Clear the master secret.
    memset(g_master_secret, '\0', sizeof(g_master_secret));

    // Clear the BIP39 mnemonic (regenerate on all zero secret)
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *) g_master_secret);

    free_slip39_generate_shares();
    
    // Clear the restore shares
    free_restore_shares();
}

void seed_from_rolls(String const &rolls) {
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
    // Generate the BIP39 mnemonic for this secret.
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *)g_master_secret);
}

int seed_restore_bip39_words(uint16_t* words, size_t nwords) {
    for (int ii = 0; ii < nwords; ++ii)
        g_bip39.setWord(ii, words[ii]);
    if (g_bip39.verifyChecksum()) {
        memcpy(g_master_secret, g_bip39.getPayload(), 16);
        return 0;
    }
    return 1;
}

void seed_generate_slip39_shares(void(*randgen)(uint8_t *, size_t)) {
    int nshares = g_generate_slip39_nshares;

    // If there are already shares, free them.
    free_slip39_generate_shares();

    char* password = "";

    uint8_t group_threshold = 1;
    uint8_t group_count = 1;
    group_descriptor group =
        { g_generate_slip39_thresh, g_generate_slip39_nshares, NULL };
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
    assert(rv == g_generate_slip39_nshares);
    assert(words_in_each_share == SLIP39_WORDS_IN_EACH_SHARE);

    g_generate_slip39_shares =
        (char **) malloc(sizeof(char *) * (g_generate_slip39_nshares+1));
    for (int ndx = 0; ndx < g_generate_slip39_nshares; ++ndx) {
        uint16_t* words = shares_buffer + (ndx * words_in_each_share);
        g_generate_slip39_shares[ndx] =
            slip39_strings_for_words(words, words_in_each_share);
    }
    g_generate_slip39_shares[g_generate_slip39_nshares] = NULL; // sentinel
}

int seed_combine_slip39_shares(uint8_t* master_secret,
                               size_t master_secret_len) {
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
                            master_secret,
                            master_secret_len
                            );

    for(int ii = 0; ii < selected_shares_len; ii++) {
        free(selected_shares_words[ii]);
    }

    return rv;
}
