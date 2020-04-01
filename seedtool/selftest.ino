// Copyright © 2020 Blockchain Commons, LLC

#include <stdint.h>

#include "seed.h"
#include "util.h"

#include "selftest.h"

namespace selftest_internal {

uint8_t ref_secret[16] =
{ 0x8d, 0x96, 0x9e, 0xef, 0x6e, 0xca, 0xd3, 0xc2,
  0x9a, 0x3a, 0x62, 0x92, 0x80, 0xe6, 0x86, 0xcf };

uint16_t ref_bip39_words[BIP39Seq::WORD_COUNT] =
{ 0x046c, 0x05a7, 0x05de, 0x06ec,
  0x0569, 0x070a, 0x0347, 0x0262,
  0x0494, 0x0039, 0x050d, 0x04f1 };

char* ref_bip39_mnemonics[BIP39Seq::WORD_COUNT] =
{ "mirror", "reject", "rookie", "talk",
  "pudding", "throw", "happy", "era",
  "myth", "already", "payment", "owner" };

size_t const ref_slip39_thresh = 2;
size_t const ref_slip39_nshares = 3;
char* ref_slip39_shares[ref_slip39_nshares] =
{ "check academic academic acid counter "
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
  "ocean desktop elephant beam aluminum" };

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

// Clearly not random. Only use for tests.
void fake_random(uint8_t *buf, size_t count) {
    uint8_t b = 0;
    for(int i = 0; i < count; i++) {
        buf[i] = b;
        b = b + 17;
    }
}

void test_failed(char *format, ...) {
  char buff[8192];
  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format, args);
  va_end(args);
  buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
  Serial.print(buff);
  // FIXME - figure out how to display something.
  // g_display.print(buff);
  abort();
}

void test_bip39_generate() {
    serial_printf("test_bip39_generate starting\n");
    Seed * seed = Seed::from_rolls("123456");
    BIP39Seq * bip39 = new BIP39Seq(seed);
    for (size_t ii = 0; ii < BIP39Seq::WORD_COUNT; ++ii) {
        if (bip39->get_word(ii) != ref_bip39_words[ii])
            test_failed("test_bip39_generate failed: word mismatch\n");
        if (strcmp(bip39->get_string(ii), ref_bip39_mnemonics[ii]) != 0)
            test_failed("test_bip39_generate failed: mnemonic mismatch\n");
    }
    delete bip39;
    delete seed;
    serial_printf("test_bip39_generate finished\n");
}

void test_bip39_restore() {
    serial_printf("test_bip39_restore starting\n");
    BIP39Seq * bip39 = BIP39Seq::from_words(ref_bip39_words);
    Seed * seed = bip39->restore_seed();
    if (!seed)
        test_failed("test_bip39_restore failed: restore failed\n");
    Seed * seed0 = Seed::from_rolls("123456");
    if (*seed != *seed0)
        test_failed("test_bip39_restore failed: seed mismatch\n");
    delete seed0;
    delete seed;
    delete bip39;
    serial_printf("test_bip39_restore finished\n");
}

void test_slip39_generate() {
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
            test_failed("test_slip39_generate failed: share mismatch\n");
        free(strings);
    }
    delete slip39;
    delete seed;
    serial_printf("test_slip39_generate finished\n");
}

void test_slip39_restore() {
    serial_printf("test_slip39_restore starting\n");
    SLIP39ShareSeq * slip39 = new SLIP39ShareSeq();
    slip39->add_share(ref_slip39_words[2]);
    slip39->add_share(ref_slip39_words[1]);
    Seed * seed = slip39->restore_seed();
    if (!seed)
        test_failed("test_slip39_restore failed: restore failed\n");
    Seed * seed0 = Seed::from_rolls("123456");
    if (*seed != *seed0)
        test_failed("test_slip39_restore failed: seed mismatch\n");
    delete seed0;
    delete seed;
    delete slip39;
    serial_printf("test_slip39_restore finished\n");
}

} // namespace selftest_internal

void selftest() {
    using namespace selftest_internal;
    serial_printf("self_test starting\n");
    test_bip39_generate();
    test_bip39_restore();
    test_slip39_generate();
    test_slip39_restore();
    serial_printf("self_test finished\n");
}

const uint16_t * selftest_dummy_bip39() {
    using namespace selftest_internal;
    return ref_bip39_words;
}

const uint16_t * selftest_dummy_slip39(size_t ndx) {
    using namespace selftest_internal;
    if (ndx > ref_slip39_nshares - 1)
        ndx = ref_slip39_nshares - 1;
    return ref_slip39_words[ndx];
}
