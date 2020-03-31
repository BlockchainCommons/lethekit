// Copyright © 2020 Blockchain Commons, LLC

#ifndef SEED_H
#define SEED_H

void seed_reset_state();

void seed_from_rolls(String const &rolls);

void seed_generate_bip39_words();

int seed_restore_bip39_words(uint16_t* words, size_t nwords);

void seed_generate_slip39_shares(void(*randgen)(uint8_t *, size_t));

int seed_combine_slip39_shares(uint8_t* master_secret,
                               size_t master_secret_len);

#endif // SEED_H
