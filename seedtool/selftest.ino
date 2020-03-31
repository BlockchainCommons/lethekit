// Copyright © 2020 Blockchain Commons, LLC

#include "selftest.h"

namespace {

uint8_t ref_secret[16] =
{ 0x8d, 0x96, 0x9e, 0xef, 0x6e, 0xca, 0xd3, 0xc2,
  0x9a, 0x3a, 0x62, 0x92, 0x80, 0xe6, 0x86, 0xcf };

char* ref_bip39_strings[BIP39_WORD_COUNT] =
{ "mirror", "reject", "rookie", "talk",
  "pudding", "throw", "happy", "era",
  "myth", "already", "payment", "owner" };

uint16_t ref_bip39_words[BIP39_WORD_COUNT] =
{ 0x046c, 0x05a7, 0x05de, 0x06ec,
  0x0569, 0x070a, 0x0347, 0x0262,
  0x0494, 0x0039, 0x050d, 0x04f1 };

char* ref_slip39_shares[3] =
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

void test_generate_seed() {
    serial_printf("test_generate_seed starting\n");
    seed_from_rolls("123456");
    if (memcmp(g_master_secret, ref_secret, sizeof(ref_secret)) != 0)
        test_failed("test_generate_seed failed: secret mismatch\n");
    serial_printf("test_generate_seed finished\n");
}     

void test_generate_bip39() {
    serial_printf("test_generate_bip39 starting\n");
    seed_generate_bip39_words();
    for (int ii = 0; ii < BIP39_WORD_COUNT; ++ii)
        if (strcmp(g_bip39.getMnemonic(g_bip39.getWord(ii)),
                   ref_bip39_strings[ii]) != 0)
            test_failed("test_generate_bip39 failed: word list mismatch\n");
    serial_printf("test_generate_bip39 finished\n");
}     

void test_restore_bip39() {
    serial_printf("test_restore_bip39 starting\n");
    int rv = seed_restore_bip39_words(ref_bip39_words, BIP39_WORD_COUNT);
    if (rv != 0)
        test_failed("test_restore_bip39 failed: restore failed\n");
    if (memcmp(g_master_secret, ref_secret, sizeof(ref_secret)) != 0)
        test_failed("test_restore_bip39 failed: secret mismatch\n");
    serial_printf("test_restore_bip39 finished\n");
}     

void test_generate_slip39() {
    serial_printf("test_generate_slip39 starting\n");
    g_generate_slip39_thresh = 2;
    g_generate_slip39_nshares = 3;
    seed_generate_slip39_shares(fake_random);
    for (int ii = 0; ii < g_generate_slip39_nshares; ++ii)
        if (strcmp(g_generate_slip39_shares[ii], ref_slip39_shares[ii]) != 0)
            test_failed("test_generate_slip39 failed: words mismatch\n");
    serial_printf("test_generate_slip39 finished\n");
}    

void test_restore_slip39() {
    serial_printf("test_restore_slip39 starting\n");
    g_restore_slip39_nshares = 2;
    g_restore_slip39_shares[0] = strdup(ref_slip39_shares[2]);
    g_restore_slip39_shares[1] = strdup(ref_slip39_shares[1]);
    int rv =
        seed_combine_slip39_shares(g_master_secret, sizeof(g_master_secret));
    if (rv < 0)
        test_failed("test_restore_slip39 failed: combine failed\n");
    if (memcmp(g_master_secret, ref_secret, sizeof(ref_secret)) != 0)
        test_failed("test_restore_slip39 failed: secret mismatch\n");
    serial_printf("test_restore_slip39 finished\n");
}     

} // namespace

void selftest() {
    serial_printf("self_test starting\n");
    {
        // Order important here.
        seed_reset_state();
        test_generate_seed();
        test_generate_bip39();
    }
    {
        // Order important here.
        seed_reset_state();
        test_restore_bip39();
        test_generate_slip39();
    }
    {
        // Order important here.
        seed_reset_state();
        test_restore_slip39();
    }
    seed_reset_state();
    serial_printf("self_test finished\n");
}

