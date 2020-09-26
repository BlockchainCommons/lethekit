// Copyright © 2020 Blockchain Commons, LLC

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "util.h"
#include "keystore.h" // TODO remove eventually after refactored

enum UIState {
    INVALID_STATE = -1,
    SELF_TEST,
    INTRO_SCREEN,
    SEEDLESS_MENU,
    GENERATE_SEED,
    RESTORE_BIP39,
    RESTORE_SSKR,
    ENTER_SHARE,
    SEEDY_MENU,
    DISPLAY_BIP39,
    CONFIG_SSKR,
    DISPLAY_SSKR,
    DISPLAY_XPUBS,
    DERIVATION_PATH,
    SET_NETWORK,
    SET_XPUB_FORMAT,
    SET_XPUB_OPTIONS,
    DISPLAY_SEED,
    SET_SEED_FORMAT,
    CUSTOM_DERIVATION_PATH,
    ERROR_SCREEN,
    SET_SSKR_FORMAT,
    OPEN_WALLET,
    SHOW_ADDRESS,
    SET_ADDRESS_FORMAT,
    EXPORT_WALLET,
    SET_EXPORT_WALLET_FORMAT,
    UR_DEMO
};

extern void ui_reset_into_state(UIState state);

extern void ui_dispatch();

struct pg_show_address_t {
    uint32_t addr_indx;
    format addr_format;
};

struct pg_export_wallet_t {
    format wallet_format;
};

struct pg_derivation_path_t {
    bool is_standard_derivation;
    enum stdDerivation std_derivation;
};

struct pg_set_xpub_format_t {
    format xpub_format;

    String get_xpub_format_as_string(void) {
      switch(xpub_format) {
          case text:
              return "base58";
          case qr_text:
              return "qr-base58";
          case ur:
              return "ur";
          case qr_ur:
              return "qr_ur";
          default:
              return "qr_ur";
      }
    }
};

struct pg_xpub_menu_t {
    bool slip132;
};

struct pg_set_xpub_options_t {
    bool slip132;
    bool show_derivation_path;
};

struct pg_set_seed_format_t {
    format seed_format;
};

struct pg_set_sskr_format_t {
    format sskr_format;
};

#endif // USERINTERFACE_H
