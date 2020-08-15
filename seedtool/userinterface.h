// Copyright © 2020 Blockchain Commons, LLC

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

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
    ERROR_SCREEN
};

extern void ui_reset_into_state(UIState state);

extern void ui_dispatch();

struct UiOption {
    String _name;
    String value;
    String tip;
    bool show;
};

#endif // USERINTERFACE_H
