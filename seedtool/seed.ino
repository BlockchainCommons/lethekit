// Copyright © 2020 Blockchain Commons, LLC

#include <stdarg.h>

#include <bc-slip39.h>
#include <bc-crypto-base.h>

#include "util.h"
#include "seed.h"

namespace seed_internal {

} // namespace seed_internal

Seed * Seed::from_rolls(String const & rolls) {
    using namespace seed_internal;

    uint8_t digest[SHA256_DIGEST_LENGTH];
    sha256_Raw((const uint8_t*)rolls.c_str(), rolls.length(), digest);
    return new Seed(digest, SIZE);

//    // Convert supplied entropy into master secret.
//    Sha256Class sha256;
//    sha256.init();
//    for(uint8_t ii=0; ii < rolls.length(); ii++) {
//        sha256.write(rolls[ii]);
//    }
//    return new Seed(sha256.result(), SIZE);
}

Seed::Seed(uint8_t const * i_data, size_t len) {
    using namespace seed_internal;

    serial_assert(len == sizeof(data));
    memcpy(data, i_data, len);
}

void Seed::log() const {
    serial_printf("seed: ");
    for (size_t ii = 0; ii < sizeof(data); ++ii)
        serial_printf("%02x", data[ii]);
    serial_printf("\n");
}

BIP39Seq * BIP39Seq::from_words(uint16_t * words) {
    BIP39Seq * retval = new BIP39Seq();
    for (size_t ii = 0; ii < WORD_COUNT; ++ii)
        retval->set_word(ii, words[ii]);
    return retval;
}

BIP39Seq::BIP39Seq() {
    ctx = bip39_new_context();
    bip39_set_byte_count(ctx, Seed::SIZE);
}

BIP39Seq::BIP39Seq(Seed const * seed) {
    using namespace seed_internal;

    ctx = bip39_new_context();
    bip39_set_byte_count(ctx, Seed::SIZE);
    bip39_set_payload(ctx, Seed::SIZE, seed->data);
}

BIP39Seq::~BIP39Seq() {
    bip39_dispose_context(ctx);
}

void BIP39Seq::set_word(size_t ndx, uint16_t word) {
    bip39_set_word(ctx, ndx, word);
}

String BIP39Seq::get_dict_string(size_t ndx) {
    char mnemonic[20];
    bip39_mnemonic_from_word(ndx, mnemonic);
    return String(mnemonic);
}

uint16_t BIP39Seq::get_word(size_t ndx) const {
    return bip39_get_word(ctx, ndx);
}

String BIP39Seq::get_string(size_t ndx) {
    uint16_t word = bip39_get_word(ctx, ndx);
    char mnemonic[20];
    bip39_mnemonic_from_word(word, mnemonic);
    return String(mnemonic);
}

Seed * BIP39Seq::restore_seed() const {
    return bip39_verify_checksum(ctx)
        ? new Seed(bip39_get_bytes(ctx), Seed::SIZE)
        : NULL;
}

bool SLIP39ShareSeq::verify_share_checksum(uint16_t const * share) {
    return rs1024_verify_checksum(share, WORDS_PER_SHARE);
}

SLIP39ShareSeq * SLIP39ShareSeq::from_seed(Seed const * seed,
                                           uint8_t thresh,
                                           uint8_t nshares,
                                           void(*randgen)(uint8_t *, size_t)) {
    const char * password = "";
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
    serial_assert(rv == (int)nshares);
    serial_assert(words_in_each_share == WORDS_PER_SHARE);

    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    for (size_t ii = 0; ii < nshares; ++ii)
        slip39->add_share(shares_buffer + (ii * WORDS_PER_SHARE));
    return slip39;
}

char const * SLIP39ShareSeq::error_msg(int errval) {
    static char buffer[1024];
    switch (errval) {
        // max message size 18 chars                |----------------|
    case ERROR_INVALID_SHARD_SET:			return "Invalid shard set";
    case ERROR_DUPLICATE_MEMBER_INDEX:		return "Duplicate shard";
    case ERROR_NOT_ENOUGH_MEMBER_SHARDS:	return "Not enough shards";
    default:
        snprintf(buffer, sizeof(buffer),           "unknown err %d", errval);
        return buffer;
        // max message size 18 chars                |----------------|
    }
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
    serial_assert(nshares < MAX_SHARES);
    size_t sharesz = sizeof(uint16_t) * WORDS_PER_SHARE;
    shares[nshares] = (uint16_t *) malloc(sharesz);
    memcpy(shares[nshares], share, sharesz);
    return nshares++;
}

uint16_t const * SLIP39ShareSeq::get_share(size_t ndx) const {
    serial_assert(ndx < nshares);
    return shares[ndx];
}

void SLIP39ShareSeq::set_share(size_t ndx, uint16_t const * share) {
    serial_assert(ndx < nshares);
    if (shares[ndx])
        free(shares[ndx]);
    size_t sharesz = sizeof(uint16_t) * WORDS_PER_SHARE;
    shares[ndx] = (uint16_t *) malloc(sharesz);
    memcpy(shares[ndx], share, sharesz);
}

void SLIP39ShareSeq::del_share(size_t ndx) {
    serial_assert(ndx < nshares);
    if (shares[ndx])
        free(shares[ndx]);
    // Compact any created gap.
    for (size_t ii = ndx; ii < nshares-1; ++ii)
        shares[ii] = shares[ii+1];
    nshares -= 1;
}

char * SLIP39ShareSeq::get_share_strings(size_t ndx) const {
    serial_assert(ndx < nshares);
    return slip39_strings_for_words(shares[ndx], WORDS_PER_SHARE);
}

char const * SLIP39ShareSeq::get_share_word(size_t sndx, size_t wndx) const {
    serial_assert(sndx < nshares);
    serial_assert(wndx < WORDS_PER_SHARE);
    return slip39_string_for_word(shares[sndx][wndx]);
}

Seed * SLIP39ShareSeq::restore_seed() const {
    uint8_t seed_data[Seed::SIZE];
    const char * password = "";
    last_rv = slip39_combine(const_cast<const uint16_t **>(shares),
                             WORDS_PER_SHARE,
                             nshares,
                             password,
                             NULL,
                             seed_data,
                             sizeof(seed_data));
    return last_rv < 0 ? NULL : new Seed(seed_data, sizeof(seed_data));
}
