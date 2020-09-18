// Copyright © 2020 Blockchain Commons, LLC

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "util.h"

enum UIState {
    INVALID_STATE = -1,
    SELF_TEST,
    INTRO_SCREEN,
    SEEDLESS_MENU,
    GENERATE_SEED,
    RESTORE_BIP39,
    RESTORE_SLIP39,
    ENTER_SHARE,
    SEEDY_MENU,
    DISPLAY_BIP39,
    CONFIG_SLIP39,
    DISPLAY_SLIP39,
    DISPLAY_XPUBS,
    XPUB_MENU,
    DERIVATION_PATH,
    SET_NETWORK,
    SET_XPUB_FORMAT,
    SEED_MENU,
    DISPLAY_SEED,
    CUSTOM_DERIVATION_PATH,
    ERROR_SCREEN,
    SET_SLIP39_FORMAT,
    OPEN_WALLET,
    SHOW_ADDRESS,
    SET_ADDRESS_FORMAT,
    EXPORT_WALLET,
    SET_EXPORT_WALLET_FORMAT,
};

extern void ui_reset_into_state(UIState state);

extern void ui_dispatch();

struct UiOption {
    String _name;
    String value;
    String tip;
    bool show;
};

struct pg_show_address_t{
    uint32_t addr_indx;
    format addr_format;
};

struct pg_export_wallet_t{
    format wallet_format;
};

#endif // USERINTERFACE_H
