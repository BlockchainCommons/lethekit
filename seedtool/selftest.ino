// Copyright © 2020 Blockchain Commons, LLC

#include <stdint.h>

#include "seed.h"
#include "util.h"

#include "selftest.h"
#include <bc-crypto-base.h>
#include <bc-bip39.h>
#include "prefix1.h"
#include "ur.h"

namespace selftest_internal {

uint8_t ref_secret[16] =
{
 0x8d, 0x96, 0x9e, 0xef, 0x6e, 0xca, 0xd3, 0xc2,
 0x9a, 0x3a, 0x62, 0x92, 0x80, 0xe6, 0x86, 0xcf
};

uint16_t ref_bip39_words_correct[BIP39Seq::WORD_COUNT] =
{
 0x046c, 0x05a7, 0x05de, 0x06ec,
 0x0569, 0x070a, 0x0347, 0x0262,
 0x0494, 0x0039, 0x050d, 0x04f1
};

uint16_t ref_bip39_words_bad_checksum[BIP39Seq::WORD_COUNT] =
{
 // third word is altered
 0x046c, 0x05a7, 0x0569, 0x06ec,
 0x0569, 0x070a, 0x0347, 0x0262,
 0x0494, 0x0039, 0x050d, 0x04f1
};

const char* ref_bip39_mnemonics[BIP39Seq::WORD_COUNT] =
{
 "mirror", "reject", "rookie", "talk",
 "pudding", "throw", "happy", "era",
 "myth", "already", "payment", "owner"
};

size_t const ref_slip39_thresh = 2;
size_t const ref_slip39_nshares = 3;
const char* ref_slip39_shares[ref_slip39_nshares] =
{
 "check academic academic acid counter "
 "both course legs visitor squeeze "
 "justice sack havoc elbow crunch "
 "scroll evoke civil exact vexed",

 "check academic academic agency custody "
 "purple ceiling walnut garlic hearing "
 "daughter kind critical necklace boundary "
 "dish away obesity glen infant",

 "check academic academic always check "
 "enemy fawn glimpse bulb rebound "
 "spelling plunge cover umbrella fused "
 "ocean desktop elephant beam aluminum"
};

uint16_t ref_slip39_words[ref_slip39_nshares][SLIP39ShareSeq::WORDS_PER_SHARE] =
{
 { 0x0088, 0x0000, 0x0000, 0x0001, 0x00a2,
   0x005d, 0x00a3, 0x0207, 0x03d8, 0x035a,
   0x01ee, 0x0307, 0x01aa, 0x0108, 0x00b0,
   0x031a, 0x0131, 0x008e, 0x0132, 0x03d1 },

 { 0x0088, 0x0000, 0x0000, 0x0011, 0x00b7,
   0x02ca, 0x0082, 0x03e0, 0x0181, 0x01af,
   0x00be, 0x01f2, 0x00ad, 0x0260, 0x005e,
   0x00e3, 0x0040, 0x026a, 0x018d, 0x01d2 },

 { 0x0088, 0x0000, 0x0000, 0x0021, 0x0088,
   0x011b, 0x0151, 0x018e, 0x006a, 0x02dc,
   0x034f, 0x02ab, 0x00a4, 0x03b4, 0x017c,
   0x026e, 0x00d3, 0x010d, 0x0043, 0x0020 }
};

// These shares are *also* generated from seed="123456", but they use
// a different random seed so they are not compatible with the others.
const char* ref_slip39_shares_alt[ref_slip39_nshares] =
{
 "deny category academic acid buyer "
 "miracle game discuss hobo decision "
 "speak depend silver yield leaves "
 "welcome increase vocal therapy burden",

 "deny category academic agency club "
 "sister freshman enjoy furl ancient "
 "undergo cage unwrap enforce machine "
 "swing check arena society language",

 "deny category academic always damage "
 "daisy froth gross march engage "
 "plot slavery agree morning moisture "
 "race snake task drink firm",
};

const char* ref_sha_input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

uint8_t ref_sha256_output[] = {
  0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
  0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
  0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
  0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
};

uint16_t ref_slip39_words_alt[ref_slip39_nshares][SLIP39ShareSeq::WORDS_PER_SHARE] =
{
 { 0x00cb,  0x0080,  0x0000,  0x0001,  0x0071,
   0x0249,  0x017e,  0x00e1,  0x01b5,  0x00c4,
   0x034d,  0x00cd,  0x032d,  0x03fd,  0x0203,
   0x03e8,  0x01ce,  0x03db,  0x0390,  0x006e, },

 { 0x00cb,  0x0080,  0x0000,  0x0011,  0x0099,
   0x0331,  0x0172,  0x011f,  0x017b,  0x0029,
   0x03b6,  0x0072,  0x03bf,  0x011d,  0x0225,
   0x0374,  0x0088,  0x0033,  0x0345,  0x01fb, },

 { 0x00cb,  0x0080,  0x0000,  0x0021,  0x00ba,
   0x00b9,  0x0177,  0x019b,  0x0232,  0x011e,
   0x02aa,  0x0335,  0x0012,  0x0251,  0x024f,
   0x02d0,  0x0342,  0x037e,  0x00f4,  0x0159, },
};

uint16_t ref_slip39_share_bad_checksum[SLIP39ShareSeq::WORDS_PER_SHARE] =
{
 // ref_slip39_words[0], 11th word is altered
 0x0088, 0x0000, 0x0000, 0x0001, 0x00a2,
 0x005d, 0x00a3, 0x0207, 0x03d8, 0x035a,
 0x0132, 0x0307, 0x01aa, 0x0108, 0x00b0,
 0x031a, 0x0131, 0x008e, 0x0132, 0x03d1
};

// Clearly not random. Only use for tests.
void fake_random(uint8_t *buf, size_t count) {
    uint8_t b = 0;
    for(size_t i = 0; i < count; i++) {
        buf[i] = b;
        b = b + 17;
    }
}

bool test_failed(const char *format, ...) {
  char buff[8192];
  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format, args);
  va_end(args);
  buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
  Serial.print(buff);
  return false;
}

bool test_sha256() {
    serial_printf("test_sha256 starting\n");
    uint8_t digest[SHA256_DIGEST_LENGTH];
    sha256_Raw((const uint8_t*)ref_sha_input, strlen(ref_sha_input), digest);
    if (memcmp(digest, ref_sha256_output, SHA256_DIGEST_LENGTH) != 0) {
        return test_failed("test_sha256 failed\n");
    }
    serial_printf("test_sha256 finished\n");
    return true;
}

bool test_seed_generate() {
    serial_printf("test_seed_generate starting\n");
    Seed * seed = Seed::from_rolls("123456");
    Seed * seed0 = new Seed(ref_secret, sizeof(ref_secret));
    if (*seed != *seed0)
        return test_failed("test_seed_generate failed: seed mismatch\n");
    delete seed0;
    delete seed;
    serial_printf("test_seed_generate finished\n");
    return true;
}

bool test_bip39_mnemonics() {
    serial_printf("test_bip39_mnemonics starting\n");
    void* ctx = bip39_new_context();
    for(size_t i = 0; i < BIP39Seq::WORD_COUNT; i++) {
        uint16_t word = ref_bip39_words_correct[i];
        const char* mnemonic1 = ref_bip39_mnemonics[i];
        const char* mnemonic2 = bip39_get_mnemonic(ctx, word);
        //serial_printf("word: 0x%0hx, mnemonic1: \"%s\", mnemonic2: \"%s\"\n", word, mnemonic1, mnemonic2);
        if(strcmp(mnemonic1, mnemonic2) != 0) {
            return test_failed("test_bip39_mnemonics failed: mismatch\n");
        }
    }
    bip39_dispose_context(ctx);
    serial_printf("test_bip39_mnemonics finished\n");
    return true;
}

bool test_bip39_generate() {
    serial_printf("test_bip39_generate starting\n");
    Seed * seed = Seed::from_rolls("123456");
    BIP39Seq * bip39 = new BIP39Seq(seed);
    for (size_t ii = 0; ii < BIP39Seq::WORD_COUNT; ++ii) {
        uint16_t word = bip39->get_word(ii);
        if (word != ref_bip39_words_correct[ii])
            return test_failed("test_bip39_generate failed: word mismatch\n");
        if (strcmp(BIP39Seq::get_dict_string(word).c_str(),
                   ref_bip39_mnemonics[ii]) != 0)
            return test_failed("test_bip39_generate failed: "
                               "dict_string mismatch\n");
        if (strcmp(bip39->get_string(ii).c_str(), ref_bip39_mnemonics[ii]) != 0)
            return test_failed("test_bip39_generate failed: "
                               "mnemonic mismatch\n");
    }
    delete bip39;
    delete seed;
    serial_printf("test_bip39_generate finished\n");
    return true;
}

bool test_bip39_restore() {
    serial_printf("test_bip39_restore starting\n");
    BIP39Seq * bip39 = BIP39Seq::from_words(ref_bip39_words_correct);
    Seed * seed = bip39->restore_seed();
    if (!seed)
        return test_failed("test_bip39_restore failed: restore failed\n");
    Seed * seed0 = Seed::from_rolls("123456");
    if (*seed != *seed0)
        return test_failed("test_bip39_restore failed: seed mismatch\n");
    delete seed0;
    delete seed;
    delete bip39;
    serial_printf("test_bip39_restore finished\n");
    return true;
}

bool test_bip39_bad_checksum() {
    serial_printf("test_bip39_bad_checksum starting\n");
    BIP39Seq * bip39 = BIP39Seq::from_words(ref_bip39_words_bad_checksum);
    Seed * seed = bip39->restore_seed();
    if (seed)
        return test_failed(
            "test_bip39_bad_checksum failed: restore verify passed\n");
    delete seed;
    delete bip39;
    serial_printf("test_bip39_bad_checksum finished\n");
    return true;
}

bool test_slip39_generate() {
    serial_printf("test_slip39_generate starting\n");
    Seed * seed = Seed::from_rolls("123456");
    SLIP39ShareSeq * slip39 =
        SLIP39ShareSeq::from_seed(seed, ref_slip39_thresh,
                                  ref_slip39_nshares, fake_random);
    for (size_t ii = 0; ii < ref_slip39_nshares; ++ii) {
        uint16_t const * words = slip39->get_share(ii);
        char * strings =
            slip39_strings_for_words(words, SLIP39ShareSeq::WORDS_PER_SHARE);
        if (strcmp(strings, ref_slip39_shares[ii]) != 0)
            return test_failed("test_slip39_generate failed: share mismatch\n");
        free(strings);
    }
    delete slip39;
    delete seed;
    serial_printf("test_slip39_generate finished\n");
    return true;
}

bool test_slip39_restore() {
    serial_printf("test_slip39_restore starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words[1]);
    Seed * seed = slip39->restore_seed();
    if (!seed)
        return test_failed("test_slip39_restore failed: restore failed\n");
    Seed * seed0 = Seed::from_rolls("123456");
    if (*seed != *seed0)
        return test_failed("test_slip39_restore failed: seed mismatch\n");
    delete seed0;
    delete seed;
    delete slip39;
    serial_printf("test_slip39_restore finished\n");
    return true;
}

bool test_slip39_verify_share_valid() {
    serial_printf("test_slip39_verify_share_valid starting\n");
    bool ok = SLIP39ShareSeq::verify_share_checksum(ref_slip39_words[0]);
    if (!ok)
        return test_failed("test_slip39_verify_share_valid failed: invalid\n");
    serial_printf("test_slip39_verify_share_valid finished\n");
    return true;
}

bool test_slip39_verify_share_invalid() {
    serial_printf("test_slip39_verify_share_invalid starting\n");
    bool ok =
        SLIP39ShareSeq::verify_share_checksum(ref_slip39_share_bad_checksum);
    if (ok)
        return test_failed("test_slip39_verify_share_invalid failed: valid\n");
    serial_printf("test_slip39_verify_share_invalid finished\n");
    return true;
}

bool test_slip39_del_share() {
    serial_printf("test_slip39_del_share starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words[0]);	// this will get deleted
    slip39->add_share(ref_slip39_words[1]);

    // delete the middle share
    slip39->del_share(1);
    if (slip39->numshares() != 2)
        return test_failed("test_slip39_del_share failed: bad numshares\n");

    // add and delete the last share
    slip39->add_share(ref_slip39_words[0]);	// this will get deleted
    slip39->del_share(2);
    if (slip39->numshares() != 2)
        return test_failed("test_slip39_del_share failed: bad numshares\n");

    Seed * seed = slip39->restore_seed();
    if (!seed)
        return test_failed("test_slip39_del_share failed: restore failed\n");
    Seed * seed0 = Seed::from_rolls("123456");
    if (*seed != *seed0)
        return test_failed("test_slip39_del_share failed: seed mismatch\n");
    delete seed0;
    delete seed;
    delete slip39;
    serial_printf("test_slip39_del_share finished\n");
    return true;
}

bool test_slip39_too_few() {
    serial_printf("test_slip39_too_few starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[2]);
    Seed * seed = slip39->restore_seed();
    if (seed)
        return test_failed("test_slip39_too_few failed: restore worked\n");
    int err = slip39->last_restore_error();
    if (err != ERROR_NOT_ENOUGH_MEMBER_SHARDS)
        return test_failed("test_slip39_too_few failed: saw err %d\n", err);
    if (strcmp(SLIP39ShareSeq::error_msg(err), "Not enough shards") != 0)
        return test_failed("test_slip39_too_few failed: wrong msg: %s\n",
                           SLIP39ShareSeq::error_msg(err));
    delete slip39;
    serial_printf("test_slip39_too_few finished\n");
    return true;
}

bool test_slip39_duplicate_share() {
    serial_printf("test_slip39_duplicate_share starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words[2]);
    Seed * seed = slip39->restore_seed();
    if (seed)
        return test_failed("test_slip39_duplicate_share failed: "
                           "restore worked\n");
    int err = slip39->last_restore_error();
    if (err != ERROR_DUPLICATE_MEMBER_INDEX)
        return test_failed(
            "test_slip39_duplicate_share failed: saw err %d\n", err);
    if (strcmp(SLIP39ShareSeq::error_msg(err), "Duplicate shard") != 0)
        return test_failed("test_slip39_duplicate_share failed: wrong msg: %s\n",
                           SLIP39ShareSeq::error_msg(err));
    delete slip39;
    serial_printf("test_slip39_duplicate_share finished\n");
    return true;
}

bool test_slip39_extra_valid_share() {
    serial_printf("test_slip39_extra_valid_share starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[1]);
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words[0]);
    Seed * seed = slip39->restore_seed();
    if (!seed)
        return test_failed(
            "test_slip39_extra_valid_share failed: restore failed\n");
    Seed * seed0 = Seed::from_rolls("123456");
    if (*seed != *seed0)
        return test_failed(
            "test_slip39_extra_valid_share failed: seed mismatch\n");
    delete seed0;
    delete seed;
    delete slip39;
    serial_printf("test_slip39_extra_valid_share finished\n");
    return true;
}

bool test_slip39_extra_dup_share() {
    serial_printf("test_slip39_extra_dup_share starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[1]);
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words[2]);
    Seed * seed = slip39->restore_seed();
    if (seed)
        return test_failed("test_slip39_duplicate_share failed: "
                           "restore worked\n");
    int err = slip39->last_restore_error();
    if (err != ERROR_DUPLICATE_MEMBER_INDEX)
        return test_failed(
            "test_slip39_duplicate_share failed: saw err %d\n", err);
    if (strcmp(SLIP39ShareSeq::error_msg(err), "Duplicate shard") != 0)
        return test_failed("test_slip39_extra_dup_share failed: wrong msg: %s\n",
                           SLIP39ShareSeq::error_msg(err));
    delete slip39;
    serial_printf("test_slip39_extra_dup_share finished\n");
    return true;
}

bool test_slip39_invalid_share() {
    // NOTE - the share has a valid checksum, but comes from
    // a different recovery set (for the same seed).
    serial_printf("test_slip39_invalid_share starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words_alt[1]);
    Seed * seed = slip39->restore_seed();
    if (seed)
        return test_failed("test_slip39_duplicate_share failed: "
                           "restore worked\n");
    int err = slip39->last_restore_error();
    if (err != ERROR_INVALID_SHARD_SET)
        return test_failed(
            "test_slip39_duplicate_share failed: saw err %d\n", err);
    if (strcmp(SLIP39ShareSeq::error_msg(err), "Invalid shard set") != 0)
        return test_failed("test_slip39_invalid_share failed: wrong msg: %s\n",
                           SLIP39ShareSeq::error_msg(err));
    delete slip39;
    serial_printf("test_slip39_invalid_share finished\n");
    return true;
}

struct selftest_t {
    char const * testname;
    bool (*testfun)();
};

selftest_t g_selftests[] =
{
 // Max test name display length is ~16 chars.
 // |--------------|
 { "SHA256", test_sha256 },
 { "seed generate", test_seed_generate },
 { "BIP39 mnemonics", test_bip39_mnemonics },
 { "BIP39 generate", test_bip39_generate },
 { "BIP39 restore", test_bip39_restore },
 { "SLIP39 generate", test_slip39_generate },
 { "SLIP39 restore", test_slip39_restore },
 { "BIP39 restore", test_bip39_restore },
 { "BIP39 bad chksum", test_bip39_bad_checksum },
 { "SLIP39 share ok", test_slip39_verify_share_valid },
 { "SLIP39 share bad", test_slip39_verify_share_invalid },
 { "SLIP39 del share", test_slip39_del_share },
 { "SLIP39 too few", test_slip39_too_few },
 { "SLIP39 dup share", test_slip39_duplicate_share },
 { "SLIP39 extra val", test_slip39_extra_valid_share },
 { "SLIP39 extra dup", test_slip39_extra_dup_share },
 { "SLIP39 inv share", test_slip39_invalid_share },
 { "UR", test_ur },
 // |--------------|
};

size_t const g_numtests = sizeof(g_selftests) / sizeof(*g_selftests);

} // namespace selftest_internal

// deprecated, calling these from userinterface now
#if 0
void selftest() {
    using namespace selftest_internal;
    serial_printf("self_test starting\n");
    test_seed_generate();
    test_bip39_generate();
    test_bip39_restore();
    test_slip39_generate();
    test_slip39_restore();
    serial_printf("self_test finished\n");
}
#endif

const uint16_t * selftest_dummy_bip39() {
    using namespace selftest_internal;
    return ref_bip39_words_correct;
}

const uint16_t * selftest_dummy_slip39(size_t ndx) {
    using namespace selftest_internal;
    if (ndx > ref_slip39_nshares - 1)
        ndx = ref_slip39_nshares - 1;
    return ref_slip39_words[ndx];
}

const uint16_t * selftest_dummy_slip39_alt(size_t ndx) {
    using namespace selftest_internal;
    if (ndx > ref_slip39_nshares - 1)
        ndx = ref_slip39_nshares - 1;
    return ref_slip39_words_alt[ndx];
}

size_t selftest_numtests() {
    using namespace selftest_internal;
    return g_numtests;
}

String selftest_testname(size_t ndx) {
    using namespace selftest_internal;
    serial_assert(ndx < g_numtests);
    return g_selftests[ndx].testname;
}

bool selftest_testrun(size_t ndx) {
    using namespace selftest_internal;
    serial_assert(ndx < g_numtests);
    return g_selftests[ndx].testfun();
}
