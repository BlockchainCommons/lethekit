// Copyright © 2020 Blockchain Commons, LLC

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include "hardware.h"
#include "seed.h"
#include "userinterface.h"
#include "selftest.h"	// Used to fetch dummy data for UI testing.
#include "util.h"
#include "qrcode.h"
#include "ur.h"
#include "keystore.h"

namespace userinterface_internal {

UIState g_uistate;

String g_rolls;
bool g_submitted;

Seed * g_master_seed = NULL;
BIP39Seq * g_bip39 = NULL;
SLIP39ShareSeq * g_slip39_generate = NULL;
SLIP39ShareSeq * g_slip39_restore = NULL;

int g_ndx = 0;		// index of "selected" word
int g_pos = 0;		// char position of cursor
int g_scroll = 0;	// index of scrolled window

int g_restore_slip39_selected;

int const Y_MAX = 200;

// FreeSansBold9pt7b
int const H_FSB9 = 16;	// height
int const YM_FSB9 = 6;	// y-margin

// FreeSansBold12pt7b
int const H_FSB12 = 20;	// height
int const YM_FSB12 = 9;	// y-margin

// FreeMonoBold9pt7b
int const H_FMB9 = 14;	// height
int const YM_FMB9 = 2;	// y-margin

// FreeMonoBold12pt7b
int const W_FMB12 = 14;	// width
int const H_FMB12 = 21;	// height
int const YM_FMB12 = 4;	// y-margin

bool clear_full_window = true;

Point text_center(const char * txt) {
    int16_t tbx, tby; uint16_t tbw, tbh;
    Point p;
    g_display->getTextBounds(txt, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    p.x = ((g_display->width() - tbw) / 2) - tbx;
    p.y = ((g_display->height() - tbh) / 2) - tby;
    return p;
}

int text_right(const char * txt) {
    int16_t tbx, tby; uint16_t tbw, tbh;
    g_display->getTextBounds(txt, 0, 0, &tbx, &tby, &tbw, &tbh);
    return ((g_display->width() - tbw) - tbx);
}

void full_window_clear() {
    g_display->firstPage();
    do
    {
        g_display->setFullWindow();
        g_display->fillScreen(GxEPD_WHITE);
    }
    while (g_display->nextPage());
}

void display_printf(const char *format, ...) {
    char buff[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);
    buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
    g_display->print(buff);
}

void interstitial_error(String const lines[], size_t nlines) {
    serial_assert(nlines <= 7);

    int xoff = 16;
    int yoff = 6;

    g_display->firstPage();
    do
    {
        g_display->setPartialWindow(0, 0, 200, 200);
        // g_display->fillScreen(GxEPD_WHITE);
        g_display->setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff;

        for (size_t ii = 0; ii < nlines; ++ii) {
            yy += H_FSB9 + YM_FSB9;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            serial_printf("%s", lines[ii].c_str());
            display_printf("%s", lines[ii].c_str());
        }

        yy = 190; // Absolute, stuck to bottom
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        display_printf("%", GIT_DESCRIBE);
    }
    while (g_display->nextPage());

    while (true) {
        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("interstitial_error saw " + String(key));
        switch (key) {
        case '#':
            return;
        default:
            break;
        }
    }
}


/**
 *   @brief       draw a qr code
 *   @pre         g_display->firstPage();
 *   @post        while (g_display->nextPage());
 *   @param[in]   text: text to be qr encoded
 */
bool displayQR(char * text) {
    // source: https://github.com/arcbtc/koopa/blob/master/main.ino

    // auto detect best qr code size
    int qrSize = 10;
    int ec_lvl = 0;
    int const sizes[18][4] = {
                        /* https://github.com/ricmoo/QRCode */
                        /* 1 */ { 17, 14, 11, 7 },
                        /* 2 */ { 32, 26, 20, 14 },
                        /* 3 */ { 53, 42, 32, 24 },
                        /* 4 */ { 78, 62, 46, 34 },
                        /* 5 */ { 106, 84, 60, 44 },
                        /* 6 */ { 134, 106, 74, 58 },
                        /* 7 */ { 154, 122, 86, 64 },
                        /* 8 */ { 192, 152, 108, 84 },
                        /* 9 */ { 230, 180, 130, 98 },      // OPN:0 LND:0 good
                        /* 10 */ { 271, 213, 151, 119 },    // BTP:0 OPN:1 good
                        /* 11 */ { 321, 251, 177, 137 },
                        /* 12 */ { 367, 287, 203, 155 },    // BTP:1 bad
                        /* 13 */ { 425, 331, 241, 177 },    // BTP:2 meh
                        /* 14 */ { 458, 362, 258, 194 },
                        /* 15 */ { 520, 412, 292, 220 },
                        /* 16 */ { 586, 450, 322, 250 },
                        /* 17 */ { 644, 504, 364, 280 },
    };
    int len = strlen(text);
    for(int ii=0; ii<17; ii++){
        qrSize = ii+1;
        if(sizes[ii][ec_lvl] > len){
            break;
        }
    }

    // Create the QR code
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(qrSize)];
    qrcode_initText(&qrcode, qrcodeData, qrSize, ec_lvl,
                    text);

    int width = 17 + 4*qrSize;
    int scale = 130/width;
    int padding = (200 - width*scale)/2;

    // for every pixel in QR code we draw a rectangle with size `scale`
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if(qrcode_getModule(&qrcode, x, y)){
                g_display->fillRect(padding+scale*x,
                                   padding+scale*y,
                                   scale, scale, GxEPD_BLACK);
            }
        }
    }

    return true;
}

void self_test() {
    int xoff = 8;
    int yoff = 6;

    size_t const NLINES = 8;
    String lines[NLINES];

    // Turn the green LED on for the duration of the tests.
    hw_green_led(HIGH);

    bool last_test_passed = true;
    size_t numtests = selftest_numtests();
    // Loop, once for each test.  Need an extra trip at the end in
    // case we failed the last test.
    for (size_t ndx = 0; ndx < numtests+1; ++ndx) {

        // If any key is pressed, skip remaining self test.
        if (g_keypad.getKey() != NO_KEY)
            break;

        // Append each test name to the bottom of the displayed list.
        size_t row = ndx;
        if (row > NLINES - 1) {
            // slide all the lines up one
            for (size_t ii = 0; ii < NLINES - 1; ++ii)
                lines[ii] = lines[ii+1];
            row = NLINES - 1;
        }

        if (!last_test_passed) {
            lines[row] = "TEST FAILED";
        } else if (ndx < numtests) {
            lines[row] = selftest_testname(ndx).c_str();
        } else {
            // ran last test and it passed
            lines[row] = "TESTS PASSED";
        }

        // Display the current list.
        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff;

            yy += 1*(H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("Running self tests:");

            yy += 10;

            for (size_t ii = 0; ii < NLINES; ++ii) {
                yy += 1*(H_FMB9 + YM_FMB9);
                g_display->setFont(&FreeMonoBold9pt7b);
                g_display->setCursor(xx, yy);
                display_printf("%s", lines[ii].c_str());
            }

            yy = 190; // Absolute, stuck to bottom
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            display_printf("%", GIT_DESCRIBE);
        }
        while (g_display->nextPage());

        // If the test failed, abort (leaving status on screen).
        if (!last_test_passed) {
            hw_green_led(LOW);
            abort();
        }

        // If this isn't the last pass run the next test.
        if (ndx < numtests)
            last_test_passed = selftest_testrun(ndx);
    }
    delay(1000);		// short pause ..
    hw_green_led(LOW);	// Green LED back off until there is a seed.
    g_uistate = INTRO_SCREEN;
}

void intro_screen() {
    int xoff = 16;
    int yoff = 6;

    g_display->firstPage();
    do
    {
        g_display->setPartialWindow(0, 0, 200, 200);
        // g_display->fillScreen(GxEPD_WHITE);
        g_display->setTextColor(GxEPD_BLACK);

        int xx = xoff + 14;
        int yy = yoff + (H_FSB12 + YM_FSB12);
        g_display->setFont(&FreeSansBold12pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("LetheKit v0");

        yy += 6;

        xx = xoff + 24;
        yy += H_FSB12 + YM_FSB12;
        g_display->setCursor(xx, yy);
        display_printf("Seedtool");

        xx = xoff + 50;
        yy += H_FSB9;
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        display_printf("%s", GIT_LATEST_TAG);

        xx = xoff + 28;
        yy += 1*(H_FSB9 + 2*YM_FSB9);
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("Blockchain");

        xx = xoff + 30;
        yy += H_FSB9 + 4;
        g_display->setCursor(xx, yy);
        g_display->println("Commons");

        xx = xoff + 18;
        yy += H_FSB9 + YM_FSB9 + 10;
        g_display->setCursor(xx, yy);
        g_display->println("Press any key");

        xx = xoff + 24;
        yy += H_FSB9;
        g_display->setCursor(xx, yy);
        g_display->println("to continue");
    }
    while (g_display->nextPage());

    while (true) {
        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("intro_screen saw " + String(key));
        g_uistate = SEEDLESS_MENU;
        return;
    }
}

void seedless_menu() {
    int xoff = 16;
    int yoff = 10;

    g_display->firstPage();
    do
    {
        g_display->setPartialWindow(0, 0, 200, 200);
        // g_display->fillScreen(GxEPD_WHITE);
        g_display->setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff + (H_FSB12 + YM_FSB12);
        g_display->setFont(&FreeSansBold12pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("No Seed");

        yy = yoff + 3*(H_FSB9 + YM_FSB9);
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("A - Generate Seed");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display->setCursor(xx, yy);
        g_display->println("B - Restore BIP39");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display->setCursor(xx, yy);
        g_display->println("C - Restore SLIP39");

        yy = 190; // Absolute, stuck to bottom
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        display_printf("%", GIT_DESCRIBE);
    }
    while (g_display->nextPage());

    while (true) {
        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("seedless_menu saw " + String(key));
        switch (key) {
        case 'A':
            g_uistate = GENERATE_SEED;
            return;
        case 'B':
            g_bip39 = new BIP39Seq();
            g_uistate = RESTORE_BIP39;
            return;
        case 'C':
            g_slip39_restore = new SLIP39ShareSeq();
            g_uistate = RESTORE_SLIP39;
            return;
        default:
            break;
        }
    }
}

void generate_seed() {
    while (true) {
        int xoff = 14;
        int yoff = 8;

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB12 + YM_FSB12);
            g_display->setFont(&FreeSansBold12pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("Generate Seed");

            yy += 10;

            yy += H_FSB9 + YM_FSB9;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("Enter Dice Rolls");

            yy += 10;

            yy += H_FMB12 + YM_FMB12;
            g_display->setFont(&FreeMonoBold12pt7b);
            g_display->setCursor(xx, yy);
            display_printf("Rolls: %d\n", g_rolls.length());
            yy += H_FMB12 + YM_FMB12;
            g_display->setCursor(xx, yy);
            display_printf(" Bits: %0.1f\n", g_rolls.length() * 2.5850);

            // bottom-relative position
            xx = xoff + 10;
            yy = Y_MAX - 2*(H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("Press * to clear");
            yy += H_FSB9 + YM_FSB9;
            g_display->setCursor(xx, yy);
            g_display->println("Press # to submit");
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("generate_seed saw " + String(key));
        switch (key) {
        case '1': case '2': case '3':
        case '4': case '5': case '6':
            g_rolls += key;
            break;
        case '*':
            g_rolls = "";
            break;
        case '#':
            g_submitted = true;
            serial_assert(!g_master_seed);
            if (g_master_seed)
                delete g_master_seed;
            g_master_seed = Seed::from_rolls(g_rolls);
            keystore.update_root_key(g_master_seed->data, sizeof(g_master_seed->data));
            g_master_seed->log();
            serial_assert(!g_bip39);
            if (g_bip39)
                delete g_bip39;
            g_bip39 = new BIP39Seq(g_master_seed);
            digitalWrite(GREEN_LED, HIGH);		// turn on green LED
            g_uistate = DISPLAY_BIP39;
            return;
        default:
            break;
        }
    }
}

void set_network() {
    int xoff = 5, yoff = 5;
    String title = "Set network";
    g_display->firstPage();
    do
    {
        g_display->setPartialWindow(0, 0, 200, 200);
        // g_display->fillScreen(GxEPD_WHITE);
        g_display->setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff + (H_FSB9 + YM_FSB9);
        g_display->setFont(&FreeSansBold9pt7b);
        Point p = text_center(title.c_str());
        g_display->setCursor(p.x, yy);
        g_display->println(title);

        yy += H_FSB9 + 2*YM_FSB9 + 15;
        g_display->setCursor(4*xx, yy);
        g_display->println("A: Regtest");

        yy += H_FSB9 + 2*YM_FSB9 + 5;
        g_display->setCursor(4*xx, yy);
        g_display->println("B: Testnet");


        yy += H_FSB9 + 2*YM_FSB9 + 5;
        g_display->setCursor(4*xx, yy);
        g_display->println("C: Mainnet");

        // bottom-relative position
        xx = xoff + 2;
        yy = Y_MAX - (H_FSB9) + 2;
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("*-Cancel");
    }
    while (g_display->nextPage());

    char key;
    do {
        key = g_keypad.getKey();
    } while (key == NO_KEY);
    g_uistate = XPUB_MENU;
    clear_full_window = false;
    switch (key) {
    case 'A':
        network.set_network(REGTEST);
        (void)keystore.update_root_key(g_master_seed->data, sizeof(g_master_seed->data), network.get_network());
        return;
    case 'B':
        network.set_network(TESTNET);
        (void)keystore.update_root_key(g_master_seed->data, sizeof(g_master_seed->data), network.get_network());
        return;
    case 'C':
        network.set_network(MAINNET);
        (void)keystore.update_root_key(g_master_seed->data, sizeof(g_master_seed->data), network.get_network());
        return;
    case '*':
        return;
    default:
        g_uistate = SET_NETWORK;
        break;
    }
}

void seedy_menu() {
    int xoff = 16;
    int yoff = 10;

    g_display->firstPage();
    do
    {
        g_display->setPartialWindow(0, 0, 200, 200);
        // g_display->fillScreen(GxEPD_WHITE);
        g_display->setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff + (H_FSB12 + YM_FSB12);
        g_display->setFont(&FreeSansBold12pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("Seed Present");

        yy = yoff + 3*(H_FSB9 + YM_FSB9);
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("A - Display BIP39");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display->setCursor(xx, yy);
        g_display->println("B - Generate SLIP39");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display->setCursor(xx, yy);
        g_display->println("C - Display XPUB");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display->setCursor(xx, yy);
        g_display->println("D - Display seed");
        yy += H_FSB9 + 2*YM_FSB9;
        // TODO: Wipe seed will probably be in Display Seed

        yy = 190; // Absolute, stuck to bottom
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        display_printf("%", GIT_DESCRIBE);
    }
    while (g_display->nextPage());

    while (true) {
        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("seedy_menu saw " + String(key));
        switch (key) {
        case 'A':
            g_uistate = DISPLAY_BIP39;
            return;
        case 'B':
            g_uistate = CONFIG_SLIP39;
            return;
        case 'C':
            clear_full_window = false;
            g_uistate = XPUB_MENU;
            return;
        case 'D':
            clear_full_window = false;
            g_uistate = SEED_MENU;
            return;
        case '*':
           ui_reset_into_state(SEEDLESS_MENU);
           g_uistate = SEEDLESS_MENU;
           return;
        default:
            break;
        }
    }
}

void display_bip39() {
    int const nwords = BIP39Seq::WORD_COUNT;
    int scroll = 0;

    while (true) {
        int const xoff = 12;
        int const yoff = 0;
        int const nrows = 5;

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("BIP39 Mnemonic");
            yy += H_FSB9 + YM_FSB9;

            yy += 6;

            g_display->setFont(&FreeMonoBold12pt7b);
            for (int rr = 0; rr < nrows; ++rr) {
                int wndx = scroll + rr;
                g_display->setCursor(xx, yy);
                display_printf("%2d %s", wndx+1, g_bip39->get_string(wndx).c_str());
                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("1,7-Up,Down #-Done");
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("display_bip39 saw " + String(key));
        switch (key) {
        case '1':
            if (scroll > 0)
                scroll -= 1;
            break;
        case '7':
            if (scroll < (nwords - nrows))
                scroll += 1;
            break;
        case '#':
            g_uistate = SEEDY_MENU;
            return;
        default:
            break;
        }
    }
}

// Append a character to a SLIP39 config value, check range.
String config_slip39_addkey(String str0, char key) {
    String newstr;
    if (str0 == " ")
        newstr = key;
    else
        newstr = str0 + key;
    int val = newstr.toInt();
    if (val == 0)	// didn't convert to integer somehow
        return str0;
    if (val < 1)	// too small
        return str0;
    if (val > 16)	// too big
        return str0;
    return newstr;
}

void config_slip39() {
    bool thresh_done = false;
    String threshstr = "3";
    String nsharestr = "5";

    while (true) {
        int xoff = 20;
        int yoff = 8;

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("Configure SLIP39");

            yy += 10;

            yy += H_FMB12 + 2*YM_FMB12;
            g_display->setFont(&FreeMonoBold12pt7b);
            g_display->setCursor(xx, yy);
            display_printf(" Thresh: %s", threshstr.c_str());

            if (!thresh_done) {
                int xxx = xx + (9 * W_FMB12);
                int yyy = yy - H_FMB12;
                g_display->fillRect(xxx,
                                   yyy,
                                   W_FMB12 * threshstr.length(),
                                   H_FMB12 + YM_FMB12,
                                   GxEPD_BLACK);
                g_display->setTextColor(GxEPD_WHITE);
                g_display->setCursor(xxx, yy);
                display_printf("%s", threshstr.c_str());
                g_display->setTextColor(GxEPD_BLACK);
            }

            yy += H_FMB12 + 2*YM_FMB12;
            g_display->setCursor(xx, yy);
            display_printf("NShares: %s", nsharestr.c_str());

            if (thresh_done) {
                int xxx = xx + (9 * W_FMB12);
                int yyy = yy - H_FMB12;
                g_display->fillRect(xxx,
                                   yyy,
                                   W_FMB12 * nsharestr.length(),
                                   H_FMB12 + YM_FMB12,
                                   GxEPD_BLACK);
                g_display->setTextColor(GxEPD_WHITE);
                g_display->setCursor(xxx, yy);
                display_printf("%s", nsharestr.c_str());
                g_display->setTextColor(GxEPD_BLACK);
            }

            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);

            if (!thresh_done) {
                g_display->println("*-Clear    #-Next");
            } else {
                // If nsharestr field is empty its a prev
                if (nsharestr == " ")
                    g_display->println("*-Prev     #-Done");
                else
                    g_display->println("*-Clear    #-Done");
            }
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("config_slip39 saw " + String(key));
        switch (key) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (!thresh_done)
                threshstr = config_slip39_addkey(threshstr, key);
            else
                nsharestr = config_slip39_addkey(nsharestr, key);
            break;
        case '*':
            if (!thresh_done) {
                threshstr = " ";
            } else {
                // If nsharestr field is empty its a prev
                if (nsharestr == " ")
                    thresh_done = false;
                else
                    nsharestr = " ";
            }
            break;
        case '#':
            if (!thresh_done) {
                thresh_done = true;
                break;
            } else {
                if (threshstr.toInt() > nsharestr.toInt()) {
                    // Threshold is greater than nshares, put the cursor
                    // back on the threshold.
                    thresh_done = false;
                    break;
                }
                // It's ok to generate multiple slip39 shares.
                if (g_slip39_generate)
                    delete g_slip39_generate;

                // This will take a few seconds; clear the screen
                // immediately to let the user know something is
                // happening ..
                full_window_clear();

                g_slip39_generate =
                    SLIP39ShareSeq::from_seed(g_master_seed,
                                              threshstr.toInt(),
                                              nsharestr.toInt(),
                                              hw_random_buffer);
                g_uistate = DISPLAY_SLIP39;
                return;
            }
        default:
            break;
        }
    }
}

void display_slip39() {
    int const nwords = SLIP39ShareSeq::WORDS_PER_SHARE;
    int sharendx = 0;
    int scroll = 0;

    while (true) {
        int xoff = 12;
        int yoff = 0;
        int nrows = 5;

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            display_printf("SLIP39 %d/%d",
                           sharendx+1, g_slip39_generate->numshares());
            yy += H_FSB9 + YM_FSB9;

            yy += 8;

            g_display->setFont(&FreeMonoBold12pt7b);
            for (int rr = 0; rr < nrows; ++rr) {
                int wndx = scroll + rr;
                char const * word =
                    g_slip39_generate->get_share_word(sharendx, wndx);
                g_display->setCursor(xx, yy);
                display_printf("%2d %s", wndx+1, word);
                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            if (sharendx < (int)(g_slip39_generate->numshares()-1))
                g_display->println("1,7-Up,Down #-Next");
            else
                g_display->println("1,7-Up,Down #-Done");
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("display_slip39 saw " + String(key));
        switch (key) {
        case '1':
            if (scroll > 0)
                scroll -= 1;
            break;
        case '7':
            if (scroll < (nwords - nrows))
                scroll += 1;
            break;
        case '*':	// prev
            if (sharendx > 0) {
                --sharendx;
                scroll = 0;
            }
            break;
        case '#':	// next / done
            if (sharendx < (int)(g_slip39_generate->numshares()-1)) {
                ++sharendx;
                scroll = 0;
            } else {
                g_uistate = SEEDY_MENU;
                return;
            }
            break;
        default:
            break;
        }
    }
}

struct WordListState {
    int nwords;				// number of words in the mnemonic
    int nrefwords;			// number of words in the total word list

    int* wordndx;			// ref index for each word in list

    int nrows;				// number of words visible
    int selected;			// index of selected word
    int pos;				// char index of cursor
    int scroll;				// index of first visible word

    WordListState(int i_nwords, int i_nrefwords)
        : nwords(i_nwords)
        , nrefwords(i_nrefwords)
        , nrows(5)
        , selected(0)
        , pos(0)
        , scroll(0)
    {
        wordndx = (int*) malloc(i_nwords * sizeof(int));
    }

    ~WordListState() {
        free(wordndx);
    }

    virtual String refword(int ndx) = 0;

    void set_words(uint16_t const * wordlist) {
        for (int ii = 0; ii < nwords; ++ii)
            wordndx[ii] = wordlist ? wordlist[ii] : 0;
    }

    void get_words(uint16_t * o_wordlist) const {
        for (int ii = 0; ii < nwords; ++ii)
            o_wordlist[ii] = wordndx[ii];
    }

    void compute_scroll() {
        if (selected < 3)
            scroll = 0;
        else if (selected > nwords - 3)
            scroll = nwords - nrows;
        else
            scroll = selected - 2;
    }

    void select_prev() {
        if (selected == 0)
            selected = nwords - 1;
        else
            --selected;
        pos = 0;
    }

    void select_next() {
        if (selected == nwords - 1)
            selected = 0;
        else
            ++selected;
        pos = 0;
    }

    void cursor_left() {
        if (pos >= 1)
            --pos;
    }

    void cursor_right() {
        if (pos < (int)refword(wordndx[selected]).length() - 1)
            ++pos;
    }

    void word_down() {
        // Find the previous word that differs in the cursor position.
        int wordndx0 = wordndx[selected];	// remember starting wordndx
        String prefix0 = prefix(wordndx[selected]);
        char curr0 = current(wordndx[selected]);
        do {
            if (wordndx[selected] == 0)
                wordndx[selected] = nrefwords - 1;
            else
                --wordndx[selected];
            // If we've returned to the original there are no choices.
            if (wordndx[selected] == wordndx0)
                break;
        } while (prefix(wordndx[selected]) != prefix0 ||
                 current(wordndx[selected]) == curr0);
    }

    void word_up() {
        // Find the next word that differs in the cursor position.
        int wordndx0 = wordndx[selected];	// remember starting wordndx
        String prefix0 = prefix(wordndx[selected]);
        char curr0 = current(wordndx[selected]);
        do {
            if (wordndx[selected] == (nrefwords - 1))
                wordndx[selected] = 0;
            else
                ++wordndx[selected];
            // If we've returned to the original there are no choices.
            if (wordndx[selected] == wordndx0)
                break;
        } while (prefix(wordndx[selected]) != prefix0 ||
                 current(wordndx[selected]) == curr0);
    }

    String prefix(int word) {
        return refword(word).substring(0, pos);
    }

    char current(int word) {
        return refword(word)[pos];
    }

    bool unique_match() {
        // Does the previous word also match?
        if (wordndx[selected] > 0)
            if (prefix(wordndx[selected] - 1) == prefix(wordndx[selected]))
                return false;
        // Does the next word also match?
        if (wordndx[selected] < (nrefwords - 1))
            if (prefix(wordndx[selected] + 1) == prefix(wordndx[selected]))
                return false;
        return true;
    }
};

struct SLIP39WordlistState : WordListState {
    SLIP39WordlistState(int i_nwords) : WordListState(i_nwords, 1024) {}
    virtual String refword(int ndx) {
        return String(slip39_string_for_word(ndx));
    }
};

struct BIP39WordlistState : WordListState {
    BIP39Seq * bip39;
    BIP39WordlistState(BIP39Seq * i_bip39, int i_nwords)
        : WordListState(i_nwords, 2048)
        , bip39(i_bip39)
    {}
    virtual String refword(int ndx) {
        return BIP39Seq::get_dict_string(ndx);
    }
};

void restore_bip39() {
    BIP39WordlistState state(g_bip39, BIP39Seq::WORD_COUNT);
    state.set_words(NULL);

    while (true) {
        int const xoff = 12;
        int const yoff = 0;

        state.compute_scroll();

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            display_printf("BIP39 Mnemonic");
            yy += H_FSB9 + YM_FSB9;

            g_display->setFont(&FreeMonoBold12pt7b);
            yy += 2;

            for (int rr = 0; rr < state.nrows; ++rr) {
                int wndx = state.scroll + rr;
                String word = state.refword(state.wordndx[wndx]);
                Serial.println(String(wndx) + " " + word);

                if (wndx != state.selected) {
                    // Regular entry, not being edited
                    g_display->setTextColor(GxEPD_BLACK);
                    g_display->setCursor(xx, yy);
                    display_printf("%2d %s\n", wndx+1, word.c_str());
                } else {
                    // Edited entry
                    if (state.unique_match()) {
                        // Unique, highlight entire word.
                        g_display->fillRect(xx - 1,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12 * (word.length() + 3) + 3,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);

                        g_display->setTextColor(GxEPD_WHITE);
                        g_display->setCursor(xx, yy);

                        display_printf("%2d %s\n", wndx+1, word.c_str());

                    } else {
                        // Not unique, highlight cursor.
                        g_display->setTextColor(GxEPD_BLACK);
                        g_display->setCursor(xx, yy);

                        display_printf("%2d %s\n", wndx+1, word.c_str());

                        g_display->fillRect(xx + (state.pos+3)*W_FMB12,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);

                        g_display->setTextColor(GxEPD_WHITE);
                        g_display->setCursor(xx + (state.pos+3)*W_FMB12, yy);
                        display_printf("%c", word.c_str()[state.pos]);
                    }
                }

                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff;
            yy = Y_MAX - 2*(H_FSB9) + 2;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setTextColor(GxEPD_BLACK);
            g_display->setCursor(xx, yy);
            g_display->println("4,6-L,R 2,8-chr-,chr+");
            yy += H_FSB9 + 2;
            g_display->setCursor(xx, yy);
            g_display->println("1,7-Up,Down #-Done");
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("restore bip39 saw " + String(key));
        switch (key) {
        case '1':
            state.select_prev();
            break;
        case '7':
            state.select_next();
            break;
        case '4':
            state.cursor_left();
            break;
        case '6':
            state.cursor_right();
            break;
        case '2':
            state.word_down();
            break;
        case '8':
            state.word_up();
            break;
        case 'D':
            // If 'D' and then '0' are typed, fill dummy data.
            do {
                key = g_keypad.getKey();
            } while (key == NO_KEY);
            Serial.println("restore bip39_D saw " + String(key));
            switch (key) {
            case '0':
                Serial.println("Loading dummy bip39 data");
                state.set_words(selftest_dummy_bip39());
                break;
            default:
                break;
            }
            break;
        case '#':	// done
            {
                uint16_t bip39_words[BIP39Seq::WORD_COUNT];
                for (size_t ii = 0; ii < BIP39Seq::WORD_COUNT; ++ii)
                    bip39_words[ii] = state.wordndx[ii];
                BIP39Seq * bip39 = BIP39Seq::from_words(bip39_words);
                Seed * seed = bip39->restore_seed();
                if (seed) {
                    serial_assert(!g_master_seed);
                    g_master_seed = seed;
                    g_master_seed->log();
                    delete g_bip39;
                    g_bip39 = bip39;
                    digitalWrite(GREEN_LED, HIGH);		// turn on green LED
                    g_uistate = SEEDY_MENU;
                    return;
                } else {
                    delete bip39;
                    String lines[7];
                    size_t nlines = 0;
                    lines[nlines++] = "BIP39 Word List";
                    lines[nlines++] = "Checksum Error";
                    lines[nlines++] = "";
                    lines[nlines++] = "Check your word";
                    lines[nlines++] = "list carefully";
                    lines[nlines++] = "";
                    lines[nlines++] = "Press # to revisit";
                    interstitial_error(lines, nlines);
                }
                break;
            }
            break;
        default:
            break;
        }
    }
}

void restore_slip39() {
    int scroll = 0;
    int selected = g_slip39_restore->numshares();	// selects "add" initially

    while (true) {
        int const xoff = 12;
        int const yoff = 0;
        int const nrows = 4;

        // Are we showing the restore action?
        int showrestore = g_slip39_restore->numshares() > 0 ? 1 : 0;

        // How many rows displayed?
        int disprows = g_slip39_restore->numshares() + 1 + showrestore;
        if (disprows > nrows)
            disprows = nrows;

        // Adjust the scroll to center the selection.
        if (selected < 2)
            scroll = 0;
        else if (selected > (int)g_slip39_restore->numshares())
            scroll = g_slip39_restore->numshares() + 2 - disprows;
        else
            scroll = selected - 2;
        serial_printf("scroll = %d\n", scroll);

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("Enter SLIP39 Shares");
            yy += H_FSB9 + YM_FSB9;

            xx = xoff + 20;
            yy += 16;

            g_display->setFont(&FreeMonoBold12pt7b);
            for (int rr = 0; rr < disprows; ++rr) {
                int sharendx = scroll + rr;
                char buffer[32];
                if (sharendx < (int)g_slip39_restore->numshares()) {
                    sprintf(buffer, "Share %d", sharendx+1);
                } else if (sharendx == (int)g_slip39_restore->numshares()) {
                    sprintf(buffer, "Add Share");
                } else {
                    sprintf(buffer, "Restore");
                }

                g_display->setCursor(xx, yy);
                if (sharendx != selected) {
                    g_display->println(buffer);
                } else {
                    g_display->fillRect(xx,
                                       yy - H_FMB12,
                                       W_FMB12 * strlen(buffer),
                                       H_FMB12 + YM_FMB12,
                                       GxEPD_BLACK);
                    g_display->setTextColor(GxEPD_WHITE);
                    g_display->println(buffer);
                    g_display->setTextColor(GxEPD_BLACK);
                }

                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            g_display->println("1,7-Up,Down #-Do");
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("restore_slip39 saw " + String(key));
        switch (key) {
        case '1':
            if (selected > 0)
                selected -= 1;
            break;
        case '7':
            if (selected < (int)g_slip39_restore->numshares() + 1 + showrestore - 1)
                selected += 1;
            break;
        case '*':
            g_uistate = SEEDLESS_MENU;
            return;
        case '#':
            if (selected < (int)g_slip39_restore->numshares()) {
                // Edit existing share
                g_restore_slip39_selected = selected;
                g_uistate = ENTER_SHARE;
                return;
            } else if (selected == (int)g_slip39_restore->numshares()) {
                // Add new (zeroed) share
                uint16_t share[SLIP39ShareSeq::WORDS_PER_SHARE] =
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                g_restore_slip39_selected = g_slip39_restore->add_share(share);
                g_uistate = ENTER_SHARE;
                return;
            } else {
                // Attempt restoration

                // This will take a few seconds; clear the screen
                // immediately to let the user know something is
                // happening ..
                full_window_clear();

                for (size_t ii = 0; ii < g_slip39_restore->numshares(); ++ii) {
                    char * strings =
                        g_slip39_restore->get_share_strings(ii);
                    serial_printf("%d %s\n", ii+1, strings);
                    free(strings);
                }
                Seed * seed = g_slip39_restore->restore_seed();
                if (!seed) {
                    int err = g_slip39_restore->last_restore_error();
                    String lines[7];
                    size_t nlines = 0;
                    lines[nlines++] = "SLIP39 Error";
                    lines[nlines++] = "";
                    lines[nlines++] = SLIP39ShareSeq::error_msg(err);
                    lines[nlines++] = "";
                    lines[nlines++] = "";
                    lines[nlines++] = "Press # to revisit";
                    lines[nlines++] = "";
                    interstitial_error(lines, nlines);
                } else {
                    serial_assert(!g_master_seed);
                    g_master_seed = seed;
                    g_master_seed->log();
                    serial_assert(!g_bip39);
                    g_bip39 = new BIP39Seq(seed);
                    digitalWrite(GREEN_LED, HIGH);		// turn on green LED
                    g_uistate = DISPLAY_BIP39;
                    return;
                }
            }
            break;
        default:
            break;
        }
    }
}

void enter_share() {
    SLIP39WordlistState state(SLIP39ShareSeq::WORDS_PER_SHARE);
    state.set_words(g_slip39_restore->get_share(g_restore_slip39_selected));

    while (true) {
        int const xoff = 12;
        int const yoff = 0;

        state.compute_scroll();

        g_display->firstPage();
        do
        {
            g_display->setPartialWindow(0, 0, 200, 200);
            // g_display->fillScreen(GxEPD_WHITE);
            g_display->setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setCursor(xx, yy);
            display_printf("SLIP39 Share %d", g_restore_slip39_selected+1);
            yy += H_FSB9 + YM_FSB9;

            g_display->setFont(&FreeMonoBold12pt7b);
            yy += 2;

            for (int rr = 0; rr < state.nrows; ++rr) {
                int wndx = state.scroll + rr;
                String word = state.refword(state.wordndx[wndx]);
                Serial.println(String(wndx) + " " + word);

                if (wndx != state.selected) {
                    // Regular entry, not being edited
                    g_display->setTextColor(GxEPD_BLACK);
                    g_display->setCursor(xx, yy);
                    display_printf("%2d %s\n", wndx+1, word.c_str());
                } else {
                    // Edited entry
                    if (state.unique_match()) {
                        // Unique, highlight entire word.
                        g_display->fillRect(xx - 1,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12 * (word.length() + 3) + 3,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);

                        g_display->setTextColor(GxEPD_WHITE);
                        g_display->setCursor(xx, yy);

                        display_printf("%2d %s\n", wndx+1, word.c_str());

                    } else {
                        // Not unique, highlight cursor.
                        g_display->setTextColor(GxEPD_BLACK);
                        g_display->setCursor(xx, yy);

                        display_printf("%2d %s\n", wndx+1, word.c_str());

                        g_display->fillRect(xx + (state.pos+3)*W_FMB12,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);

                        g_display->setTextColor(GxEPD_WHITE);
                        g_display->setCursor(xx + (state.pos+3)*W_FMB12, yy);
                        display_printf("%c", word.c_str()[state.pos]);
                    }
                }

                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff;
            yy = Y_MAX - 2*(H_FSB9) + 2;
            g_display->setFont(&FreeSansBold9pt7b);
            g_display->setTextColor(GxEPD_BLACK);
            g_display->setCursor(xx, yy);
            g_display->println("4,6-L,R 2,8-chr-,chr+");
            yy += H_FSB9 + 2;
            g_display->setCursor(xx, yy);
            g_display->println("1,7-Up,Down #-Done");
        }
        while (g_display->nextPage());

        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("enter_share saw " + String(key));
        switch (key) {
        case '1':
            state.select_prev();
            break;
        case '7':
            state.select_next();
            break;
        case '4':
            state.cursor_left();
            break;
        case '6':
            state.cursor_right();
            break;
        case '2':
            state.word_down();
            break;
        case '8':
            state.word_up();
            break;
        case 'D':
            do {
                key = g_keypad.getKey();
            } while (key == NO_KEY);
            Serial.println("enter_share_D saw " + String(key));
            switch (key) {
            case '0':
                // If 'D' and then '0' are typed, fill with valid dummy data.
                Serial.println("Loading dummy slip39 data");
                state.set_words(selftest_dummy_slip39(
                    g_restore_slip39_selected));
                break;
            case '9':
                // If 'D' and then '9' are typed, fill with invalid
                // share (but correct checksum).
                Serial.println("Loading dummy slip39 data");
                state.set_words(selftest_dummy_slip39_alt(
                    g_restore_slip39_selected));
                break;
            default:
                break;
            }
            break;
        case '*':
            // Don't add this share, go back to enter restore_slip39 menu.
            serial_assert(g_slip39_restore);
            g_slip39_restore->del_share(g_restore_slip39_selected);
            g_uistate = RESTORE_SLIP39;
            return;
        case '#':	// done
            {
                uint16_t words[SLIP39ShareSeq::WORDS_PER_SHARE];
                state.get_words(words);
                bool ok = SLIP39ShareSeq::verify_share_checksum(words);
                if (!ok) {
                    String lines[7];
                    size_t nlines = 0;
                    lines[nlines++] = "SLIP39 Share";
                    lines[nlines++] = "Checksum Error";
                    lines[nlines++] = "";
                    lines[nlines++] = "Check your word";
                    lines[nlines++] = "list carefully";
                    lines[nlines++] = "";
                    lines[nlines++] = "Press # to revisit";
                    interstitial_error(lines, nlines);
                } else {
                    serial_assert(g_slip39_restore);
                    g_slip39_restore->set_share(
                        g_restore_slip39_selected, words);
                    g_uistate = RESTORE_SLIP39;
                    return;
                }
            }
        default:
            break;
        }
    }
}

void derivation_path(void) {

    String path_start = "m/";
    String path_entered = "";
    int x_off = 5;
    bool path_is_valid = false;
    String is_valid_tip = "Invalid";
    ext_key xpubkey;

    while (true) {
      g_display->firstPage();
      do
      {
          g_display->setPartialWindow(0, 0, 200, 200);
          g_display->fillScreen(GxEPD_WHITE);
          g_display->setTextColor(GxEPD_BLACK);

          const char * title = "Enter derivation path";
          int yy = 20;

          g_display->setFont(&FreeSansBold9pt7b);
          Point p = text_center(title);
          g_display->setCursor(p.x, yy);
          g_display->println(title);

          g_display->setFont(&FreeMonoBold12pt7b);
          g_display->setCursor(x_off, yy + 50);
          g_display->println(path_start + path_entered);

          if (path_is_valid)
              is_valid_tip = "Valid";
          else
              is_valid_tip = "Invalid";
          p = text_center(is_valid_tip.c_str());
          g_display->setCursor(p.x, p.y + 20);
          g_display->println(is_valid_tip);

          yy = 195; // Absolute, stuck to bottom
          g_display->setFont(&FreeMono9pt7b);
          String right_option = "Ok #";
          int x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          g_display->println(right_option);

          String left_option = "Cancel *";
          g_display->setCursor(0, yy);
          g_display->println(left_option);

          yy -= 25;
          String tip = "A=h  B=/";
          g_display->setCursor(0, yy);
          g_display->println(tip);

          right_option = "Del D";
          x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          g_display->println(right_option);
      }
      while (g_display->nextPage());

      char key;
      do {
          key = g_keypad.getKey();
      } while (key == NO_KEY);

      switch (key) {
        case '#':
            if (path_is_valid) {
                keystore.save_derivation_path((path_start + path_entered).c_str());
                g_uistate = XPUB_MENU;
                return;
            }
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            path_entered += key;
            break;
        case 'A':
            path_entered += "h";
            break;
        case 'B':
            path_entered += "/";
            break;
        case 'D':
            if (path_entered.length() > 0)
                path_entered.remove(path_entered.length()-1);
            break;
        case '*':
            g_uistate = XPUB_MENU;
            return;
        default:
            break;
      }

      if (keystore.check_derivation_path((path_start + path_entered).c_str())) {
          path_is_valid = true;
      }
      else {
          path_is_valid = false;
      }
    }
}

void set_xpub_format() {
    int xoff = 5, yoff = 5;
    String title = "Set xpub format";
    g_display->firstPage();
    do
    {
        g_display->setPartialWindow(0, 0, 200, 200);
        // g_display->fillScreen(GxEPD_WHITE);
        g_display->setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff + (H_FSB9 + YM_FSB9);
        g_display->setFont(&FreeSansBold9pt7b);
        Point p = text_center(title.c_str());
        g_display->setCursor(p.x, yy);
        g_display->println(title);

        yy += H_FSB9 + 2*YM_FSB9 + 12;
        g_display->setCursor(4*xx, yy);
        g_display->println("A: Qr-Base58");

        yy += H_FSB9 + 2*YM_FSB9 + 1;
        g_display->setCursor(4*xx, yy);
        g_display->println("B: Base58");

        yy += H_FSB9 + 2*YM_FSB9 + 1;
        g_display->setCursor(4*xx, yy);
        g_display->println("C: Qr-UR");

        yy += H_FSB9 + 2*YM_FSB9 + 1;
        g_display->setCursor(4*xx, yy);
        g_display->println("D: UR");

        // bottom-relative position
        xx = xoff + 2;
        yy = Y_MAX - (H_FSB9) + 2;
        g_display->setFont(&FreeSansBold9pt7b);
        g_display->setCursor(xx, yy);
        g_display->println("*-Cancel");
    }
    while (g_display->nextPage());

    char key;
    do {
        key = g_keypad.getKey();
    } while (key == NO_KEY);
    g_uistate = XPUB_MENU;
    clear_full_window = false;
    switch (key) {
    case 'A':
        keystore.set_xpub_format(QR_BASE58);
        return;
    case 'B':
        keystore.set_xpub_format(BASE58);
        return;
    case 'C':
        keystore.set_xpub_format(QR_UR);
        return;
    case 'D':
        keystore.set_xpub_format(UR);
        return;
    case '*':
        return;
    default:
        g_uistate = SET_XPUB_FORMAT;
        break;
    }
}

void xpub_menu(void) {
    int x_off = 5;
    bool option_slip132 = false;
    Point p;
    // @FIXME option should not be retained when entering from "Seed Present" menu
    static unsigned int option_atm = 0;

    int xx = 0;
    String line = "___________";

    UiOption options[] = {{"derivation", keystore.get_derivation_path(), "Change with A"},
                          {"slip132",  "Off", "Todo"},
                          {"show derivation", keystore.show_derivation_path ? "On" : "Off", "Change with A"},
                          {"network", network.as_string(), "Change with A",},
                          {"format", keystore.get_xpub_format_as_string(), "Change with A"}};

    while (true) {

      g_display->firstPage();
      do
      {
          g_display->setPartialWindow(0, 0, 200, 200);
          g_display->fillScreen(GxEPD_WHITE);
          g_display->setTextColor(GxEPD_BLACK);

          const char * title = "Xpub Menu";
          int yy = 25;
          g_display->setFont(&FreeSansBold12pt7b);
          p = text_center(title);
          g_display->setCursor(p.x, yy);
          g_display->println(title);

          yy = yy + 34;

          p = text_center(line.c_str());
          g_display->setCursor(p.x, yy);
          display_printf("%s", line.c_str());

          g_display->setFont(&FreeMonoBold9pt7b);
          p = text_center(options[option_atm]._name.c_str());
          xx = p.x;
          g_display->setCursor(xx, yy);
          display_printf("%s", options[option_atm]._name.c_str());

          g_display->setFont(&FreeMonoBold12pt7b);
          p = text_center(options[option_atm].value.c_str());
          xx = p.x;
          yy += 35;
          g_display->setCursor(xx, yy);
          display_printf("%s", options[option_atm].value.c_str());

          g_display->setFont(&FreeMono9pt7b);
          p = text_center(options[option_atm].tip.c_str());
          xx = p.x;
          yy += 27;
          g_display->setCursor(xx, yy);
          display_printf("%s", options[option_atm].tip.c_str());

          yy += (H_FMB12 + YM_FMB12) -5;
          g_display->setFont(&FreeMonoBold12pt7b);
          p = text_center(line.c_str());
          g_display->setCursor(p.x, yy);
          xx = p.x;
          display_printf("%s", line.c_str());

          yy = 195; // Absolute, stuck to bottom
          g_display->setFont(&FreeMono9pt7b);
          String right_option = "# Ok";
          int x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          g_display->println(right_option);

          String left_option = "Cancel *";
          g_display->setCursor(0, yy);
          g_display->println(left_option);

          g_display->setFont(&FreeMonoBold9pt7b);
          yy -= 32;
          right_option = "6 next ";
          x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          if (option_atm < (ARRAY_SIZE(options) - 1))
              g_display->println(right_option);

          left_option = " prev 4";
          g_display->setCursor(0, yy);
          if (option_atm > 0)
              g_display->println(left_option);

      }
      while (g_display->nextPage());

      char key;
      do {
          key = g_keypad.getKey();
      } while (key == NO_KEY);

      clear_full_window = false;
      switch (key) {
        case '#':
            g_uistate = DISPLAY_XPUBS;
            return;
        case '*':
            g_uistate = SEEDY_MENU;
            return;
        case 'A':
              if (options[option_atm]._name == "derivation") { g_uistate = DERIVATION_PATH; return; }
              if (options[option_atm]._name == "slip132") { option_slip132 = !option_slip132; options[1].value = option_slip132 ? "On" : "Off"; break;}
              if (options[option_atm]._name == "show derivation") {
                  keystore.show_derivation_path = !keystore.show_derivation_path; options[2].value = keystore.show_derivation_path ? "On" : "Off"; break;}
              if (options[option_atm]._name == "network") { g_uistate = SET_NETWORK; return; }
              if (options[option_atm]._name == "format") {g_uistate = SET_XPUB_FORMAT; return;}
            break;
        case '4':
            if (option_atm > 0)
                option_atm -= 1;
            break;
        case '6':
            if (option_atm < (ARRAY_SIZE(options))-1)
                option_atm += 1;
            break;
        default:
            break;
      }
    }
}


void display_xpub(void) {
    ext_key key;
    uint8_t cbor_xpub[50];
    String ur_string, bytewords_string;
    String encoding_type;
    const int nrows = 5;
    int scroll = 0;
    String derivation_path = keystore.get_derivation_path();
    int scroll_strlen = 0;

    // @FIXME: return value checks
    (void)keystore.update_root_key(g_master_seed->data, sizeof(g_master_seed->data));
    (void)keystore.get_xpub(&key);

    char *xpub = NULL;
    bip32_key_to_base58(&key, BIP32_FLAG_KEY_PUBLIC, &xpub);

    (void)ur_encode_hd_pubkey_xpub(bytewords_string);

    while (true) {
      g_display->firstPage();
      do
      {
          g_display->setPartialWindow(0, 0, 200, 200);
          g_display->fillScreen(GxEPD_WHITE);
          g_display->setTextColor(GxEPD_BLACK);

          const char * title = "Master pubkey";
          int yy = 18;
          g_display->setFont(&FreeSansBold9pt7b);
          Point p = text_center(title);
          g_display->setCursor(p.x, yy);
          g_display->println(title);

          switch(keystore.get_xpub_format()) {
            case BASE58:
                g_display->setFont(&FreeMonoBold9pt7b);
                g_display->setCursor(0, yy + 30);
                if (keystore.show_derivation_path) {
                    char fingerprint[9] = {0};
                    sprintf(fingerprint, "%02x%02x%02x%02x", key.hash160[0], key.hash160[1], key.hash160[2], key.hash160[3]);
                    g_display->println("[" + String(fingerprint) + keystore.get_derivation_path().substring(1) + "]" + String(xpub));
                }
                else
                    g_display->println(xpub);
                break;
            case QR_BASE58:
                if (keystore.show_derivation_path) {
                    char fingerprint[9] = {0};
                    sprintf(fingerprint, "%02x%02x%02x%02x", key.hash160[0], key.hash160[1], key.hash160[2], key.hash160[3]);
                    String fing = "[" + String(fingerprint) + keystore.get_derivation_path().substring(1) + "]" + String(xpub);
                    displayQR((char *)fing.c_str());
                }
                else {
                    displayQR(xpub);
                }
                break;
            case UR: {
                int xx = 0;
                yy = 50;
                g_display->setFont(&FreeMonoBold9pt7b);
                g_display->setCursor(0, yy);
                int16_t tbx, tby; uint16_t tbw, tbh;
                int i = 0;

                if (scroll_strlen == 0) {
                    // determine the number of chars that fit into display width
                    for (; i < bytewords_string.length(); i++) {
                        g_display->getTextBounds(bytewords_string.substring(0, i), 0, 0, &tbx, &tby, &tbw, &tbh);
                        if (tbw >= g_display->width() - 10) {
                            scroll_strlen = bytewords_string.substring(0, i).length();
                            break;
                        }
                      }
                    }
                    if ((scroll)*scroll_strlen >= bytewords_string.length()) {
                        // don't scroll to infinity
                        scroll--;
                    }

                    for (int k = 0; k < nrows; ++k) {
                        int wndx = scroll + k;
                        g_display->setCursor(xx, yy);
                        display_printf("%s", bytewords_string.substring((k + scroll)*scroll_strlen, (1+k + scroll)*scroll_strlen).c_str());
                        yy += H_FMB12 + YM_FMB12;
                    }
                }
                break;
            case QR_UR:
                displayQR((char *)bytewords_string.c_str());
                break;
            default:
                break;
          }

          yy = 195; // Absolute, stuck to bottom
          g_display->setFont(&FreeMono9pt7b);
          String right_option = "# Done";
          int x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          g_display->println(right_option);

          String left_option = "Back *";
          g_display->setCursor(0, yy);
          g_display->println(left_option);

         if (scroll_strlen > 0) {
             yy = 195 - 20;
             g_display->setCursor(0, yy);
             left_option = "   Up 1";
             g_display->println(left_option);

             right_option = "7 Down   ";
             x_r = text_right(right_option.c_str());
             g_display->setCursor(x_r, yy);
             g_display->println(right_option);
         }
      }
      while (g_display->nextPage());

      char key;
      do {
          key = g_keypad.getKey();
      } while (key == NO_KEY);

      switch (key) {
        case '#':
            g_uistate = SEEDY_MENU;
            if (xpub != NULL) {
              wally_free_string(xpub);
            }
            return;
        case '*':
            g_uistate = XPUB_MENU;
            if (xpub != NULL) {
              wally_free_string(xpub);
            }
            return;
        case '1':
            if (scroll > 0)
                scroll -= 1;
            break;
        case '7':
            scroll += 1;
            break;
        default:
            break;
      }
    }
  }

void seed_menu(void) {
    int x_off = 5;
    /* only 2 formats: UR and Qr_UR */
    String seed_format[] = {"Qr-UR", "UR"};
    Point p;
    // @FIXME option should not be retained when entering from "Seed Present" menu
    static unsigned int option_atm = 0;

    int xx = 0;
    String line = "___________";

    UiOption options[] = {{"format", seed_format[(int)g_master_seed->display_format], "Change with A"},
                          {"RAM", "Seed", "Wipe seed with A"}};

    while (true) {

      g_display->firstPage();
      do
      {
          g_display->setPartialWindow(0, 0, 200, 200);
          g_display->fillScreen(GxEPD_WHITE);
          g_display->setTextColor(GxEPD_BLACK);

          const char * title = "Seed Menu";
          int yy = 25;
          g_display->setFont(&FreeSansBold12pt7b);
          p = text_center(title);
          g_display->setCursor(p.x, yy);
          g_display->println(title);

          yy = yy + 34;

          p = text_center(line.c_str());
          g_display->setCursor(p.x, yy);
          display_printf("%s", line.c_str());

          g_display->setFont(&FreeMonoBold9pt7b);
          p = text_center(options[option_atm]._name.c_str());
          xx = p.x;
          g_display->setCursor(xx, yy);
          display_printf("%s", options[option_atm]._name.c_str());

          g_display->setFont(&FreeMonoBold12pt7b);
          p = text_center(options[option_atm].value.c_str());
          xx = p.x;
          yy += 35;
          g_display->setCursor(xx, yy);
          display_printf("%s", options[option_atm].value.c_str());

          g_display->setFont(&FreeMono9pt7b);
          p = text_center(options[option_atm].tip.c_str());
          xx = p.x;
          yy += 27;
          g_display->setCursor(xx, yy);
          display_printf("%s", options[option_atm].tip.c_str());

          yy += (H_FMB12 + YM_FMB12) -5;
          g_display->setFont(&FreeMonoBold12pt7b);
          p = text_center(line.c_str());
          g_display->setCursor(p.x, yy);
          xx = p.x;
          display_printf("%s", line.c_str());

          yy = 195; // Absolute, stuck to bottom
          g_display->setFont(&FreeMono9pt7b);
          String right_option = "# Ok";
          int x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          g_display->println(right_option);

          String left_option = "Cancel *";
          g_display->setCursor(0, yy);
          g_display->println(left_option);

          g_display->setFont(&FreeMonoBold9pt7b);
          yy -= 32;
          right_option = "6 next ";
          x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          if (option_atm < (ARRAY_SIZE(options) - 1))
              g_display->println(right_option);

          left_option = " prev 4";
          g_display->setCursor(0, yy);
          if (option_atm > 0)
              g_display->println(left_option);

      }
      while (g_display->nextPage());

      char key;
      do {
          key = g_keypad.getKey();
      } while (key == NO_KEY);

      clear_full_window = false;
      switch (key) {
        case '#':
            g_uistate = DISPLAY_SEED;
            return;
        case '*':
            g_uistate = SEEDY_MENU;
            return;
        case 'A':
              if (options[option_atm]._name == "RAM") {
                  // @TODO wipe xpub
                  ui_reset_into_state(SEEDLESS_MENU);
                  g_uistate = SEEDLESS_MENU;
                  return;
              }
              else if (options[option_atm]._name == "format") {
                  if(g_master_seed->display_format == ur)
                      g_master_seed->display_format = qr_ur;
                  else
                      g_master_seed->display_format = ur;
                  options[0].value = seed_format[(int)g_master_seed->display_format];
                  break;
              }
            break;
        case '4':
            if (option_atm > 0)
                option_atm -= 1;
            break;
        case '6':
            if (option_atm < (ARRAY_SIZE(options))-1)
                option_atm += 1;
            break;
        default:
            break;
      }
    }
}

void display_seed(void) {
    uint8_t cbor_xpub[50];
    String ur_string;

    (void)ur_encode_crypto_seed(g_master_seed->data, sizeof(g_master_seed->data), ur_string);

    while (true) {
      g_display->firstPage();
      do
      {
          g_display->setPartialWindow(0, 0, 200, 200);
          g_display->fillScreen(GxEPD_WHITE);
          g_display->setTextColor(GxEPD_BLACK);

          const char * title = "Seed";
          int yy = 25;
          g_display->setFont(&FreeSansBold9pt7b);
          Point p = text_center(title);
          g_display->setCursor(p.x, yy);
          g_display->println(title);

          switch(g_master_seed->display_format) {
            case ur:
                g_display->setFont(&FreeMonoBold9pt7b);
                g_display->setCursor(0, yy + 40);
                g_display->println(ur_string);
                break;
            case qr_ur:
                displayQR((char *)ur_string.c_str());
                break;
            default:
                break;
          }

          yy = 195; // Absolute, stuck to bottom
          g_display->setFont(&FreeMono9pt7b);
          String right_option = "# Done";
          int x_r = text_right(right_option.c_str());
          g_display->setCursor(x_r, yy);
          g_display->println(right_option);

          String left_option = "Back *";
          g_display->setCursor(0, yy);
          g_display->println(left_option);
      }
      while (g_display->nextPage());

      char key;
      do {
          key = g_keypad.getKey();
      } while (key == NO_KEY);

      switch (key) {
        case '#':
            g_uistate = SEEDY_MENU;
            return;
        case '*':
            g_uistate = SEED_MENU;
            return;
        default:
            break;
      }
    }
  }

} // namespace userinterface_internal

void ui_reset_into_state(UIState state) {
    using namespace userinterface_internal;

    if (g_master_seed) {
        delete g_master_seed;
        g_master_seed = NULL;
    }
    if (g_bip39) {
        delete g_bip39;
        g_bip39 = NULL;
    }
    if (g_slip39_generate) {
        delete g_slip39_generate;
        g_slip39_generate = NULL;
    }
    if (g_slip39_restore) {
        delete g_slip39_restore;
        g_slip39_restore = NULL;
    }

    g_rolls = "";
    g_submitted = false;
    g_uistate = state;

    hw_green_led(LOW);
}

void ui_dispatch() {
    using namespace userinterface_internal;

    if (clear_full_window) {
        full_window_clear();
        clear_full_window = true;
    }

    switch (g_uistate) {
    case SELF_TEST:
        self_test();
        break;
    case INTRO_SCREEN:
        intro_screen();
        break;
    case SEEDLESS_MENU:
        seedless_menu();
        break;
    case GENERATE_SEED:
        generate_seed();
        break;
    case RESTORE_BIP39:
        restore_bip39();
        break;
    case RESTORE_SLIP39:
        restore_slip39();
        break;
    case ENTER_SHARE:
        enter_share();
        break;
    case SEEDY_MENU:
        seedy_menu();
        break;
    case DISPLAY_BIP39:
        display_bip39();
        break;
    case CONFIG_SLIP39:
        config_slip39();
        break;
    case DISPLAY_SLIP39:
        display_slip39();
        break;
    case XPUB_MENU:
        xpub_menu();
        break;
    case DISPLAY_XPUBS:
        display_xpub();
        break;
    case DERIVATION_PATH:
        derivation_path();
        break;
    case SET_NETWORK:
        set_network();
        break;
    case SET_XPUB_FORMAT:
        set_xpub_format();
        break;
    case SEED_MENU:
        seed_menu();
        break;
    case DISPLAY_SEED:
        display_seed();
        break;
    default:
        Serial.println("loop: unknown g_uistate " + String(g_uistate));
        break;
    }
}
