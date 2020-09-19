// Copyright © 2020 Blockchain Commons, LLC

#include <stdarg.h>
#include <bc-sskr.h>
#include <bc-crypto-base.h>

#include "util.h"
#include "seed.h"
#include "wally_crypto.h"
#include "ur.h"

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

SSKRShareSeq * SSKRShareSeq::from_seed(Seed const * seed,
                                           uint8_t thresh,
                                           uint8_t nshares,
                                           void(*randgen)(uint8_t *, size_t, void *)) {
    uint8_t group_threshold = 1;
    uint8_t group_len = 1;
    sskr_group_descriptor group = { thresh, nshares };
    sskr_group_descriptor groups[] = { group };

    size_t bytes_in_each_share = 0;
    size_t buff_size = 1024;
    uint8_t buff[buff_size];
    
    size_t share_len = Seed::SIZE + METADATA_LENGTH_BYTES;
    int share_count = sskr_count_shards(group_threshold, groups, group_len);

    buff_size = share_len * share_count;

    int gen_share_count = sskr_generate(group_threshold,
                             groups,
                             group_len,
                             seed->data,
                             Seed::SIZE,
                             &bytes_in_each_share,
                             buff,
                             buff_size,
                             NULL,
                             randgen);
    Serial.println(gen_share_count); Serial.println(share_count);
    serial_assert(gen_share_count == (int)share_count);
    Serial.println(bytes_in_each_share); Serial.println(share_len);
    serial_assert(bytes_in_each_share == share_len);


    String strings[MAX_SHARES];
    String ur_strings[MAX_SHARES];
    uint8_t v[buff_size];
    for (int i = 0; i < share_count; i++) {
        uint8_t* bytes = buff + (i * bytes_in_each_share);
        memcpy(v, bytes, bytes_in_each_share);

        CborDynamicOutput output;
        CborWriter writer(output);

        writer.writeTag(309);
        writer.writeBytes(v, bytes_in_each_share);

        // Encode cbor payload as bytewords
        char *payload_bytewords = bytewords_encode(bw_standard, output.getData(), output.getSize());
        if(payload_bytewords == NULL) {
            Serial.println("ur_encode bytewords failed");
            return NULL;
        }
        else {
            Serial.println(payload_bytewords);
            strings[i] = String(payload_bytewords);
        }

        // and encode also as UR
        String ur;
        (void)ur_encode("crypto-sskr", output.getData(), output.getSize(), ur);
        ur_strings[i] = ur;
    }

    SSKRShareSeq * sskr = new SSKRShareSeq();
    memcpy(sskr->shares_buffer, buff, buff_size);
    sskr->shares_buffer_len = buff_size;
    sskr->shares_len = share_count;
    for (int i=0; i<share_count; i++) {
        sskr->shares[i] = strings[i];
        sskr->shares_ur[i] = ur_strings[i];
    }

    // how many words per (bytewords encoded) share are there?
    // count spaces. 1 space means 2 words, therefore start with 1 word
    sskr->words_per_share = 1;
    int pos = 0;
    while (pos < strings[0].length()) {
      pos = strings[0].indexOf(" ", pos);
      if (pos == -1) {
          break;
      }
      else {
          sskr->words_per_share++;
          pos++;
      }
    }
    
    return sskr;
}

String SSKRShareSeq::get_share_word(int sharendx, int wndx) {
    return get_word_from_sentence(shares[sharendx], ' ', wndx);
}

BIP39Seq * BIP39Seq::from_words(uint16_t * words) {
    BIP39Seq * retval = new BIP39Seq();
    for (size_t ii = 0; ii < WORD_COUNT; ++ii)
        retval->set_word(ii, words[ii]);

    // this function takes a couple of seconds!
    retval->calc_mnemonic_seed();

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

    // this function takes a couple of seconds!
    calc_mnemonic_seed();
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

String BIP39Seq::get_mnemonic_as_string() {
    String mnemonic;
    for (size_t i=0; i< WORD_COUNT; i++) {
        mnemonic += get_string(i);
        if (i != WORD_COUNT - 1)
            mnemonic += " ";
    }
    return mnemonic;
}

bool BIP39Seq::calc_mnemonic_seed() {

    String mnemonic_str = get_mnemonic_as_string();
    size_t written;

    // TODO: password currently NULL
    int ret = bip39_mnemonic_to_seed(mnemonic_str.c_str(), NULL, mnemonic_seed, sizeof(mnemonic_seed), &written);
    if (ret != WALLY_OK)
        return false;
    return true;
}

Seed * BIP39Seq::restore_seed() const {
    return bip39_verify_checksum(ctx)
        ? new Seed(bip39_get_bytes(ctx), Seed::SIZE)
        : NULL;
}

int bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase,
                            unsigned char *bytes_out, size_t len,
                            size_t *written)
{
    const uint32_t bip9_cost = 2048u;
    const char *prefix = "mnemonic";
    const size_t prefix_len = strlen(prefix);
    const size_t passphrase_len = passphrase ? strlen(passphrase) : 0;
    const size_t salt_len = prefix_len + passphrase_len;
    unsigned char *salt;
    int ret;

    if (written)
        *written = 0;

    if (!mnemonic || !bytes_out || len != BIP39_SEED_LEN_512)
        return WALLY_EINVAL;

    salt = (unsigned char *)malloc(salt_len);
    if (!salt)
        return WALLY_ENOMEM;

    memcpy(salt, prefix, prefix_len);
    if (passphrase_len)
        memcpy(salt + prefix_len, passphrase, passphrase_len);

    ret = wally_pbkdf2_hmac_sha512((unsigned char *)mnemonic, strlen(mnemonic),
                                   salt, salt_len, 0,
                                   bip9_cost, bytes_out, len);

    if (!ret && written)
        *written = BIP39_SEED_LEN_512; /* Succeeded */

    //clear_and_free(salt, salt_len);
    memset(salt, 0, salt_len);
    free(salt);

    return ret;
}
