// Copyright © 2020 Blockchain Commons, LLC

#include <stdint.h>

#include "seed.h"
#include "util.h"

#include "selftest.h"
#include <bc-crypto-base.h>
#include <bc-bip39.h>
#include "prefix1.h"

#include "secp256k1.h"
#include "wally_core.h"
#include "wally_bip32.h"
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

const char* ref_sha_input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

uint8_t ref_sha256_output[] = {
  0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
  0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
  0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
  0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
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

bool test_bip32(void) {
    int res;
    ext_key root;
    // test vector from: https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vector-1
    uint8_t seed[] ={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    String expected_xprv = F("xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi");

    res = wally_init(0);
    if (res != WALLY_OK) {
        serial_printf("test_bip32 libwally init failed\n");
        return false;
    }

    res = bip32_key_from_seed(seed, sizeof(seed), BIP32_VER_MAIN_PRIVATE, 0, &root);
    if (res != WALLY_OK) {
        serial_printf("test_bip32 bip32 failed\n");
        return false;
    }

    char *xprv = NULL;
    res = bip32_key_to_base58(&root, BIP32_FLAG_KEY_PRIVATE, &xprv);
    if (res != WALLY_OK) {
        serial_printf("test_bip32 base58 failed\n");
        return false;
    }
    if (strcmp(xprv, expected_xprv.c_str()) != 0) {
        serial_printf("test_bip32 xprv derivation failed\n");
        return false;
    }
    wally_free_string(xprv);

    // The last example in #test-vector-1:
    String derivation_path = F("m/0h/1/2h/2/1000000000");
    String expected_xpub = F("xpub6H1LXWLaKsWFhvm6RVpEL9P4KfRZSW7abD2ttkWP3SSQvnyA8FSVqNTEcYFgJS2UaFcxupHiYkro49S8yGasTvXEYBVPamhGW6cFJodrTHy");
    ext_key key;
    char *xpub = NULL;
    Keystore keystore = Keystore();

    keystore.update_root_key(seed, sizeof(seed), MAINNET);

    // STUB derivation path
    bool retval = keystore.check_derivation_path(derivation_path.c_str(), true);
    if (retval == false) {
        Serial.println(F("save derivation path failed"));
        return false;
    }

    keystore.get_xpub(&key);
    bip32_key_to_base58(&key, BIP32_FLAG_KEY_PUBLIC, &xpub);

    if (strcmp(xpub, expected_xpub.c_str()) != 0) {
        serial_printf("test_bip32 xpub derivation failed\n");
        return false;
    }

    wally_free_string(xpub);
    wally_cleanup(0);
    return true;
}

bool test_sskr(void) {

    uint8_t s[16] = {0x7d, 0xaa, 0x85, 0x12, 0x51, 0x00, 0x28, 0x74, 0xe1, 0xa1, 0x99, 0x5f, 0x08, 0x97, 0xe6, 0xb1};
    Seed seed = Seed(s, 16);
    SSKRShareSeq * sskr = SSKRShareSeq::from_seed(&seed, 2, 3, random_buffer);

    for (int i=0; i < sskr->shares_len; i++) {
         Serial.println(sskr->shares[i]);
    }

/*  
    @TODO for now only visual inspection with bc-seedtool-cli 
    gorazd@gorazd-MS-7C37:~$ seedtool --in sskr
    tuna acid epic gyro love work able acid able puff news duty also visa jowl jury wand vast saga flew inky door join lion legs yell yurt gyro bald
    tuna acid epic gyro love work able acid acid zero oboe paid tuna frog exam knob claw zinc back noon wasp runs able kept swan fizz many high cats
    7daa851251002874e1a1995f0897e6b1
*/

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
 { "BIP39 restore", test_bip39_restore },
 { "BIP39 bad chksum", test_bip39_bad_checksum },
 { "BIP32", test_bip32 },
 { "UR", test_ur },
 { "SSKR", test_sskr},
 // |--------------|
};

size_t const g_numtests = sizeof(g_selftests) / sizeof(*g_selftests);

} // namespace selftest_internal

const uint16_t * selftest_dummy_bip39() {
    using namespace selftest_internal;
    return ref_bip39_words_correct;
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
