// Copyright © 2020 Blockchain Commons, LLC

// Only define one of these
#undef ESP32
#define SAMD51	1

/*
 *  Epaper PIN MAP: [
 *            ESP32     SAMD51
 * ---------------------------
 *      VCC - 3.3V      3.3V
 *      GND - GND       GND
 *      SDI - GPIO18    23/MOSI
 *     SCLK - GPIO5     24/SCK
 *       CS - GPIO21    A4
 *      D/C - GPIO17    A3
 *    Reset - GPIO16    A2
 *     Busy - GPIO4     A1
 *  ]
 *
 *  Keypad Matrix PIN MAP: [
 *            ESP32     SAMD51
 * ---------------------------
 *     Pin8 - GPIO13    13
 *        7 - GPIO12    12
 *        6 - GPIO27    11
 *        5 - GPIO33    10
 *        4 - GPIO15    9
 *        3 - GPIO32    6
 *        2 - GPIO14    5
 *     Pin1 - GPIO22    21/SCL
 *  ]
 *
 *  BLUE LED PIN MAP: [
 *                      ESP32       SAMD51
 * ---------------------------------------
 *    POS (long leg)  - GPIO25      1
 *    NEG (short leg) - GND         GND
 *  ]
 *
 *  GREEN LED PIN MAP: [
 *                      ESP32       SAMD51
 * ---------------------------------------
 *    POS (long leg)  - GPIO26      4
 *    NEG (short leg) - GND         GND
 *  ]
 */

#include <stdarg.h>

#include <bip39.h>
#include <GxEPD2_BW.h>
#include <Keypad.h>

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

extern "C" {
#include <slip39.h>
#include <slip39_wordlist.h>
}

#if defined(SAMD51)
extern "C" {
#include "trngFunctions.h"
}
#endif

// LEDS
#if defined(ESP32)
#define BLUE_LED	25
#define GREEN_LED	26
#elif defined(SAMD51)
#define BLUE_LED	1
#define GREEN_LED	4
#endif

// Display
#if defined(ESP32)
GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT>
    g_display(GxEPD2_154(/*CS=*/   21,
                         /*DC=*/   17,
                         /*RST=*/  16,
                         /*BUSY=*/ 4));
#elif defined(SAMD51)
GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT>
    g_display(GxEPD2_154(/*CS=*/   PIN_A4,
                         /*DC=*/   PIN_A3,
                         /*RST=*/  PIN_A2,
                         /*BUSY=*/ PIN_A1));
#endif

// Keypad
const byte rows_ = 4;
const byte cols_ = 4;
char keys_[rows_][cols_] = {
                         {'1','2','3','A'},
                         {'4','5','6','B'},
                         {'7','8','9','C'},
                         {'*','0','#','D'}
};
#if defined(ESP32)
byte rowPins_[rows_] = {13, 12, 27, 33};
byte colPins_[cols_] = {15, 32, 14, 22};
#elif defined(SAMD51)
byte rowPins_[rows_] = {13, 12, 11, 10};
byte colPins_[cols_] = {9, 6, 5, 21};
#endif

Keypad g_keypad = Keypad(makeKeymap(keys_), rowPins_, colPins_, rows_, cols_);

enum UIState {
    SEEDLESS_MENU,
    GENERATE_SEED,
    RESTORE_BIP39,
    RESTORE_SLIP39,
    ENTER_SHARE,
    SEEDY_MENU,
    DISPLAY_BIP39,
    CONFIG_SLIP39,
    DISPLAY_SLIP39,
};

// This will be 256 when we support multiple levels.
#define MAX_SLIP39_SHARES	16

UIState g_uistate;
String g_rolls;
bool g_submitted;
uint8_t g_master_secret[16];
Bip39 g_bip39;
int g_slip39_thresh;
int g_slip39_nshares;
char** g_slip39_shares = NULL;
char* g_restore_shares[MAX_SLIP39_SHARES];
int g_num_restore_shares = 0;
int g_selected_share;

// FIXME - these should be removed
int g_wordndx[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

int g_ndx = 0;		// index of "selected" word
int g_pos = 0;		// char position of cursor
int g_scroll = 0;	// index of scrolled window
    
void setup() {
    pinMode(BLUE_LED, OUTPUT);	// Blue LED
    digitalWrite(BLUE_LED, HIGH);
    
    pinMode(GREEN_LED, OUTPUT);	// Green LED
    digitalWrite(GREEN_LED, LOW);
    
    g_display.init(115200);
    g_display.setRotation(1);

    Serial.begin(115200);

    // Only wait on the ESP32, the SAMD51 gets hung w/o serial here
#if defined(ESP32)
    while (!Serial);	// wait for serial to come online
#endif

    Serial.println("seedtool starting");

#if defined(SAMD51)
    trngInit();
#endif

    reset_state();
}

void loop() {
    full_window_clear();
    
    switch (g_uistate) {
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
    default:
        Serial.println("loop: unknown g_uistate " + String(g_uistate));
        break;
    }
}

void full_window_clear() {
    g_display.firstPage();
    do
    {
        g_display.setFullWindow();
        g_display.fillScreen(GxEPD_WHITE);
    }
    while (g_display.nextPage());
}

void reset_state() {
    digitalWrite(GREEN_LED, LOW);  // turn off the green LED
            
    g_uistate = SEEDLESS_MENU;
    g_rolls = "";
    g_submitted = false;

    // Clear the master secret.
    memset(g_master_secret, '\0', sizeof(g_master_secret));

    // Clear the BIP39 mnemonic (regenerate on all zero secret)
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *) g_master_secret);

    free_slip39_shares();
    
    // Clear the restore shares
    free_restore_shares();
}

void log_master_secret() {
    serial_printf("master secret: ");
    for (int ii = 0; ii < sizeof(g_master_secret); ++ii)
        serial_printf("%02x", (int) g_master_secret[ii]);
    serial_printf("\n");
}

int const Y_MAX = 200;

// FreeSansBold9pt7b
int const H_FSB9 = 16;	// height
int const YM_FSB9 = 6;	// y-margin

// FreeSansBold12pt7b
int const H_FSB12 = 20;	// height
int const YM_FSB12 = 9;	// y-margin

// FreeMonoBold12pt7b
int const W_FMB12 = 14;	// width
int const H_FMB12 = 21;	// height
int const YM_FMB12 = 4;	// y-margin

void seedless_menu() {
    int xoff = 16;
    int yoff = 10;
    
    g_display.firstPage();
    do
    {
        g_display.setPartialWindow(0, 0, 200, 200);
        // g_display.fillScreen(GxEPD_WHITE);
        g_display.setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff + (H_FSB12 + YM_FSB12);
        g_display.setFont(&FreeSansBold12pt7b);
        g_display.setCursor(xx, yy);
        g_display.println("No Seed");

        yy = yoff + 3*(H_FSB9 + YM_FSB9);
        g_display.setFont(&FreeSansBold9pt7b);
        g_display.setCursor(xx, yy);
        g_display.println("A - Generate Seed");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display.setCursor(xx, yy);
        g_display.println("B - Restore BIP39");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display.setCursor(xx, yy);
        g_display.println("C - Restore SLIP39");
    }
    while (g_display.nextPage());

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
            g_uistate = RESTORE_BIP39;
            return;
        case 'C':
            g_uistate = RESTORE_SLIP39;
            return;
        case NO_KEY:
        default:
            break;
        }
    }
}

void generate_seed() {
    while (true) {
        int xoff = 14;
        int yoff = 8;
    
        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB12 + YM_FSB12);
            g_display.setFont(&FreeSansBold12pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("Generate Seed");

            yy += 10;
        
            yy += H_FSB9 + YM_FSB9;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("Enter Dice Rolls");

            yy += 10;
        
            yy += H_FMB12 + YM_FMB12;
            g_display.setFont(&FreeMonoBold12pt7b);
            g_display.setCursor(xx, yy);
            display_printf("Rolls: %d\n", g_rolls.length());
            yy += H_FMB12 + YM_FMB12;
            g_display.setCursor(xx, yy);
            display_printf(" Bits: %0.1f\n", g_rolls.length() * 2.5850);

            // bottom-relative position
            xx = xoff + 10;
            yy = Y_MAX - 2*(H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("Press * to clear");
            yy += H_FSB9 + YM_FSB9;
            g_display.setCursor(xx, yy);
            g_display.println("Press # to submit");
        }
        while (g_display.nextPage());
    
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
            seed_from_rolls();
            log_master_secret();
            digitalWrite(GREEN_LED, HIGH);		// turn on green LED
            g_uistate = DISPLAY_BIP39;
            return;
        default:
            break;
        }
    }
}

void seed_from_rolls() {
    // Convert supplied entropy into master secret.
    Sha256Class sha256;
    sha256.init();
    for(uint8_t ii=0; ii < g_rolls.length(); ii++) {
        sha256.write(g_rolls[ii]);
    }
    memcpy(g_master_secret, sha256.result(), sizeof(g_master_secret));

    generate_bip39();
}

void generate_bip39() {
    // Generate the BIP39 mnemonic for this secret.
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *)g_master_secret);
}

void seedy_menu() {
    int xoff = 16;
    int yoff = 10;
    
    g_display.firstPage();
    do
    {
        g_display.setPartialWindow(0, 0, 200, 200);
        // g_display.fillScreen(GxEPD_WHITE);
        g_display.setTextColor(GxEPD_BLACK);

        int xx = xoff;
        int yy = yoff + (H_FSB12 + YM_FSB12);
        g_display.setFont(&FreeSansBold12pt7b);
        g_display.setCursor(xx, yy);
        g_display.println("Seed Present");

        yy = yoff + 3*(H_FSB9 + YM_FSB9);
        g_display.setFont(&FreeSansBold9pt7b);
        g_display.setCursor(xx, yy);
        g_display.println("A - Display BIP39");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display.setCursor(xx, yy);
        g_display.println("B - Generate SLIP39");
        yy += H_FSB9 + 2*YM_FSB9;
        g_display.setCursor(xx, yy);
        g_display.println("C - Wipe Seed");
    }
    while (g_display.nextPage());

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
            reset_state();
            g_uistate = SEEDLESS_MENU;
            return;
        case NO_KEY:
        default:
            break;
        }
    }
}

void display_bip39() {
    int const nwords = 12;
    int scroll = 0;
    
    while (true) {
        int const xoff = 12;
        int const yoff = 0;
        int const nrows = 5;
    
        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("BIP39 Mnemonic");
            yy += H_FSB9 + YM_FSB9;
            
            yy += 6;
        
            g_display.setFont(&FreeMonoBold12pt7b);
            for (int rr = 0; rr < nrows; ++rr) {
                int wndx = scroll + rr;
                uint16_t word = g_bip39.getWord(wndx);
                g_display.setCursor(xx, yy);
                display_printf("%2d %s", wndx+1, g_bip39.getMnemonic(word));
                yy += H_FMB12 + YM_FMB12;
            }
            
            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("1,7-Up,Down #-Done");
        }
        while (g_display.nextPage());
        
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
    
        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("Configure SLIP39");

            yy += 10;

            yy += H_FMB12 + 2*YM_FMB12;
            g_display.setFont(&FreeMonoBold12pt7b);
            g_display.setCursor(xx, yy);
            display_printf(" Thresh: %s", threshstr.c_str());

            if (!thresh_done) {
                int xxx = xx + (9 * W_FMB12);
                int yyy = yy - H_FMB12;
                g_display.fillRect(xxx,
                                   yyy,
                                   W_FMB12 * threshstr.length(),
                                   H_FMB12 + YM_FMB12,
                                   GxEPD_BLACK);
                g_display.setTextColor(GxEPD_WHITE);
                g_display.setCursor(xxx, yy);
                display_printf("%s", threshstr.c_str());
                g_display.setTextColor(GxEPD_BLACK);
            }
            
            yy += H_FMB12 + 2*YM_FMB12;
            g_display.setCursor(xx, yy);
            display_printf("NShares: %s", nsharestr.c_str());

            if (thresh_done) {
                int xxx = xx + (9 * W_FMB12);
                int yyy = yy - H_FMB12;
                g_display.fillRect(xxx,
                                   yyy,
                                   W_FMB12 * nsharestr.length(),
                                   H_FMB12 + YM_FMB12,
                                   GxEPD_BLACK);
                g_display.setTextColor(GxEPD_WHITE);
                g_display.setCursor(xxx, yy);
                display_printf("%s", nsharestr.c_str());
                g_display.setTextColor(GxEPD_BLACK);
            }

            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);

            if (!thresh_done) {
                g_display.println("*-Clear    #-Next");
            } else {
                // If nsharestr field is empty its a prev
                if (nsharestr == " ")
                    g_display.println("*-Prev     #-Done");
                else
                    g_display.println("*-Clear    #-Done");
            }
        }
        while (g_display.nextPage());
            
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
                g_slip39_thresh = threshstr.toInt();
                g_slip39_nshares = nsharestr.toInt();
                generate_slip39_shares();
                g_uistate = DISPLAY_SLIP39;
                return;
            }
        default:
            break;
        }
    }
}

void free_slip39_shares() {
    if (g_slip39_shares) {
        for (int ndx = 0; g_slip39_shares[ndx]; ++ndx)
            free(g_slip39_shares[ndx]);
        free(g_slip39_shares);
        g_slip39_shares = NULL;
    }
}

void generate_slip39_shares() {
    int nshares = g_slip39_nshares;

    // If there are already shares, free them.
    free_slip39_shares();
    
    // Allocate space for new shares
    g_slip39_shares = (char **) malloc(sizeof(char *) * (nshares+1));
    for (int ndx = 0; ndx < nshares; ++ndx)
        g_slip39_shares[ndx] = (char *)malloc(MNEMONIC_LIST_LEN);
    g_slip39_shares[nshares] = NULL; // sentinel

    generate_mnemonics(g_slip39_thresh, g_slip39_nshares,
                       g_master_secret, sizeof(g_master_secret),
                       NULL, 0, 0, g_slip39_shares);
}

// Extract the indicted word from the wordlist.
String slip39_select(char* wordlist, int wordndx) {
    // Start at the begining of the wordlist.
    char* pos = wordlist;

    // Advance to the nth word.
    for (int ii = 0; ii < wordndx; ++ii) {
        do {
            ++pos;
        } while (*pos != ' ');
        // stops on the next space
        ++pos; // advance to the next word.
    }

    char* pos0 = pos;
    do {
        ++pos;
    } while (*pos != ' ');
    char* epos = pos;

    return String(pos0).substring(0, epos-pos0);	// yuk
}

void display_slip39() {
    int const nwords = 20;
    int sharendx = 0;
    int scroll = 0;
    
    while (true) {
        int xoff = 12;
        int yoff = 0;
        int nrows = 5;
    
        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            display_printf("SLIP39 %d/%d", sharendx+1, g_slip39_nshares);
            yy += H_FSB9 + YM_FSB9;
            
            yy += 8;
        
            g_display.setFont(&FreeMonoBold12pt7b);
            for (int rr = 0; rr < nrows; ++rr) {
                int wndx = scroll + rr;
                String word = slip39_select(g_slip39_shares[sharendx], wndx);
                g_display.setCursor(xx, yy);
                display_printf("%2d %s", wndx+1, word.c_str());
                yy += H_FMB12 + YM_FMB12;
            }
            
            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            if (sharendx < (g_slip39_nshares-1))
                g_display.println("1,7-Up,Down #-Next");
            else
                g_display.println("1,7-Up,Down #-Done");
        }
        while (g_display.nextPage());
        
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
            if (sharendx < (g_slip39_nshares-1)) {
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

void free_restore_shares() {
    for (int ndx = 0; ndx < g_num_restore_shares; ++ndx) {
        free(g_restore_shares[ndx]);
        g_restore_shares[ndx] = NULL;
    }
    g_num_restore_shares = 0;
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

    virtual char const * refword(int ndx) = 0;
    
    void set_words(char* wordlist) {
        if (!wordlist) {
            for (int ii = 0; ii < nwords; ++ii)
                wordndx[ii] = 0;
        } else {
            char* tmp = strdup(wordlist);	// so we can poke NULL w/ strtok
            char* ptr = strtok(tmp, " ");
            for (int ii = 0; ii < nwords; ++ii) {
                for (int ndx = 0; ndx < nrefwords; ++ndx) {
                    if (strcmp(ptr, refword(ndx)) == 0) {
                        wordndx[ii] = ndx;
                        break;
                    }
                }
                ptr = strtok(NULL, " ");
            }
            free(tmp);
        }
    }
    
    char* word_list_string() {
        String retval;
        for (int ii = 0; ii < nwords; ++ii) {
            retval += refword(wordndx[ii]);
            if (ii != nwords-1)
                retval += ' ';
        }
        return strdup(retval.c_str());
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
        if (pos < strlen(refword(wordndx[selected])) - 1)
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
        return String(refword(word)).substring(0, pos);
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
    virtual char const * refword(int ndx) { return slip39_wordlist[ndx]; }
};

struct BIP39WordlistState : WordListState {
    Bip39 & bip39;
    BIP39WordlistState(Bip39 i_bip39, int i_nwords)
        : WordListState(i_nwords, 2048)
        , bip39(i_bip39)
    {}
    virtual char const * refword(int ndx) { return bip39.getMnemonic(ndx); }
};

void restore_bip39() {
    BIP39WordlistState state(g_bip39, 12);
    state.set_words(NULL);

    while (true) {
        int const xoff = 12;
        int const yoff = 0;

        state.compute_scroll();

        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            display_printf("BIP39 Mnemonic");
            yy += H_FSB9 + YM_FSB9;

            g_display.setFont(&FreeMonoBold12pt7b);
            yy += 2;

            for (int rr = 0; rr < state.nrows; ++rr) {
                int wndx = state.scroll + rr;
                String word = String(state.refword(state.wordndx[wndx]));
                Serial.println(String(wndx) + " " + word);

                if (wndx != state.selected) {
                    // Regular entry, not being edited
                    g_display.setTextColor(GxEPD_BLACK);
                    g_display.setCursor(xx, yy);
                    display_printf("%2d %s\n", wndx+1, word.c_str());
                } else {
                    // Edited entry
                    if (state.unique_match()) {
                        // Unique, highlight entire word.
                        g_display.fillRect(xx - 1,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12 * (word.length() + 3) + 3,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);
        
                        g_display.setTextColor(GxEPD_WHITE);
                        g_display.setCursor(xx, yy);

                        display_printf("%2d %s\n", wndx+1, word.c_str());

                    } else {
                        // Not unique, highlight cursor.
                        g_display.setTextColor(GxEPD_BLACK);
                        g_display.setCursor(xx, yy);
            
                        display_printf("%2d %s\n", wndx+1, word.c_str());

                        g_display.fillRect(xx + (state.pos+3)*W_FMB12,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);
        
                        g_display.setTextColor(GxEPD_WHITE);
                        g_display.setCursor(xx + (state.pos+3)*W_FMB12, yy);
                        display_printf("%c", word.c_str()[state.pos]);
                    }
                }

                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff;
            yy = Y_MAX - 2*(H_FSB9) + 2;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setTextColor(GxEPD_BLACK);
            g_display.setCursor(xx, yy);
            g_display.println("4,6-L,R 2,8-chr-,chr+");
            yy += H_FSB9 + 2;
            g_display.setCursor(xx, yy);
            g_display.println("1,7-Up,Down #-Done");
        }
        while (g_display.nextPage());
        
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
        case '#':	// done
            for (int ii = 0; ii < state.nwords; ++ii)
                g_bip39.setWord(ii, state.wordndx[ii]);
            if (g_bip39.verifyChecksum()) {
                memcpy(g_master_secret, g_bip39.getPayload(), 16);
                log_master_secret();
                digitalWrite(GREEN_LED, HIGH);		// turn on green LED
                g_uistate = SEEDY_MENU;
                return;
            }
            // FIXME - need diagnostic here
            break;
        default:
            break;
        }
    }

}

void restore_slip39() {
    int scroll = 0;
    int selected = g_num_restore_shares;	// selects "add" initially
    
    while (true) {
        int const xoff = 12;
        int const yoff = 0;
        int const nrows = 4;
    
        // Are we showing the restore action?
        int showrestore = g_num_restore_shares > 0 ? 1 : 0;

        // How many rows displayed?
        int disprows = g_num_restore_shares + 1 + showrestore;
        if (disprows > nrows)
            disprows = nrows;
            
        // Adjust the scroll to center the selection.
        if (selected < 2)
            scroll = 0;
        else if (selected > g_num_restore_shares)
            scroll = g_num_restore_shares + 2 - disprows;
        else
            scroll = selected - 2;
        serial_printf("scroll = %d\n", scroll);
        
        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("Enter SLIP39 Shares");
            yy += H_FSB9 + YM_FSB9;

            xx = xoff + 20;
            yy += 16;

            g_display.setFont(&FreeMonoBold12pt7b);
            for (int rr = 0; rr < disprows; ++rr) {
                int sharendx = scroll + rr;
                char buffer[32];
                if (sharendx < g_num_restore_shares) {
                    sprintf(buffer, "Share %d", sharendx+1);
                } else if (sharendx == g_num_restore_shares) {
                    sprintf(buffer, "Add Share");
                } else {
                    sprintf(buffer, "Restore");
                }
                
                g_display.setCursor(xx, yy);
                if (sharendx != selected) {
                    g_display.println(buffer);
                } else {
                    g_display.fillRect(xx,
                                       yy - H_FMB12,
                                       W_FMB12 * strlen(buffer),
                                       H_FMB12 + YM_FMB12,
                                       GxEPD_BLACK);
                    g_display.setTextColor(GxEPD_WHITE);
                    g_display.println(buffer);
                    g_display.setTextColor(GxEPD_BLACK);
                }

                yy += H_FMB12 + YM_FMB12;
            }
            
            // bottom-relative position
            xx = xoff + 2;
            yy = Y_MAX - (H_FSB9) + 2;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            g_display.println("1,7-Up,Down #-Do");
        }
        while (g_display.nextPage());
        
        char key;
        do {
            key = g_keypad.getKey();
        } while (key == NO_KEY);
        Serial.println("display_bip39 saw " + String(key));
        switch (key) {
        case '1':
            if (selected > 0)
                selected -= 1;
            break;
        case '7':
            if (selected < g_num_restore_shares + 1 + showrestore - 1)
                selected += 1;
            break;
        case '*':
            g_uistate = SEEDLESS_MENU;
            return;
        case '#':
            if (selected < g_num_restore_shares) {
                // Edit existing share
                g_selected_share = selected;
                g_uistate = ENTER_SHARE;
                return;
            } else if (selected == g_num_restore_shares) {
                // Add new share
                g_selected_share = g_num_restore_shares;
                g_restore_shares[g_selected_share] = NULL;
                g_num_restore_shares += 1;
                g_uistate = ENTER_SHARE;
                return;
            } else {
                // Attempt restoration
                uint8_t ms[16];
                int msl = sizeof(ms);
                for (int ii = 0; ii < g_num_restore_shares; ++ii)
                    serial_printf("%d %s\n", ii+1, g_restore_shares[ii]);
                int rv = combine_mnemonics(g_num_restore_shares,
                                           g_restore_shares,
                                           NULL, 0,
                                           ms, &msl);
                if (rv != 0) {
                    // Something went wrong
                    serial_printf("combine_mnemonics failed: %d\n", rv);
                    g_uistate = RESTORE_SLIP39;
                    return;
                } else {
                    memcpy(g_master_secret, ms, msl);
                    log_master_secret();
                    generate_bip39();
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
    SLIP39WordlistState state(20);
    state.set_words(g_restore_shares[g_selected_share]);

    while (true) {
        int const xoff = 12;
        int const yoff = 0;

        state.compute_scroll();

        g_display.firstPage();
        do
        {
            g_display.setPartialWindow(0, 0, 200, 200);
            // g_display.fillScreen(GxEPD_WHITE);
            g_display.setTextColor(GxEPD_BLACK);

            int xx = xoff;
            int yy = yoff + (H_FSB9 + YM_FSB9);
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setCursor(xx, yy);
            display_printf("SLIP39 Share %d", g_selected_share+1);
            yy += H_FSB9 + YM_FSB9;

            g_display.setFont(&FreeMonoBold12pt7b);
            yy += 2;

            for (int rr = 0; rr < state.nrows; ++rr) {
                int wndx = state.scroll + rr;
                String word = String(state.refword(state.wordndx[wndx]));
                Serial.println(String(wndx) + " " + word);

                if (wndx != state.selected) {
                    // Regular entry, not being edited
                    g_display.setTextColor(GxEPD_BLACK);
                    g_display.setCursor(xx, yy);
                    display_printf("%2d %s\n", wndx+1, word.c_str());
                } else {
                    // Edited entry
                    if (state.unique_match()) {
                        // Unique, highlight entire word.
                        g_display.fillRect(xx - 1,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12 * (word.length() + 3) + 3,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);
        
                        g_display.setTextColor(GxEPD_WHITE);
                        g_display.setCursor(xx, yy);

                        display_printf("%2d %s\n", wndx+1, word.c_str());

                    } else {
                        // Not unique, highlight cursor.
                        g_display.setTextColor(GxEPD_BLACK);
                        g_display.setCursor(xx, yy);
            
                        display_printf("%2d %s\n", wndx+1, word.c_str());

                        g_display.fillRect(xx + (state.pos+3)*W_FMB12,
                                           yy - H_FMB12 + YM_FMB12,
                                           W_FMB12,
                                           H_FMB12 + YM_FMB12,
                                           GxEPD_BLACK);
        
                        g_display.setTextColor(GxEPD_WHITE);
                        g_display.setCursor(xx + (state.pos+3)*W_FMB12, yy);
                        display_printf("%c", word.c_str()[state.pos]);
                    }
                }

                yy += H_FMB12 + YM_FMB12;
            }

            // bottom-relative position
            xx = xoff;
            yy = Y_MAX - 2*(H_FSB9) + 2;
            g_display.setFont(&FreeSansBold9pt7b);
            g_display.setTextColor(GxEPD_BLACK);
            g_display.setCursor(xx, yy);
            g_display.println("4,6-L,R 2,8-chr-,chr+");
            yy += H_FSB9 + 2;
            g_display.setCursor(xx, yy);
            g_display.println("1,7-Up,Down #-Done");
        }
        while (g_display.nextPage());
        
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
        case '#':	// done
            if (g_restore_shares[g_selected_share])
                free(g_restore_shares[g_selected_share]);
            g_restore_shares[g_selected_share] = state.word_list_string();
            g_uistate = RESTORE_SLIP39;
            return;
        default:
            break;
        }
    }

}

extern "C" {
void random_buffer(uint8_t *buf, size_t len) {
    printf("random_buffer %d\n", len);
    uint32_t r = 0;
    for (size_t i = 0; i < len; i++) {
        if (i % 4 == 0) {
#if defined(ESP32)
            r = esp_random();
#elif defined(SAMD51)
            r = trngGetRandomNumber();
#endif
        }
        buf[i] = (r >> ((i % 4) * 8)) & 0xFF;
    }
}

void debug_print(char const * str) {
    Serial.println(str);
}
}

void display_printf(char *format, ...) {
  char buff[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format,args);
  va_end(args);
  buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
  g_display.print(buff);
}

void serial_printf(char *format, ...) {
  char buff[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format,args);
  va_end(args);
  buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
  Serial.print(buff);
}

// ----------------------------------------------------------------

void no_input_or_display() {
    g_rolls = "123456";
    
    generate_key();
            
    for (int ndx = 0; ndx < 12; ndx += 2) {
        int col = 15;
        int row = ndx*10 + 40;
        String word0 = g_bip39.getMnemonic(g_bip39.getWord(ndx));
        String word1 = g_bip39.getMnemonic(g_bip39.getWord(ndx+1));
        Serial.println(word0 + " " + word1);
    }
    
    make_slip39_wordlist();
}

void generate_key() {
    Sha256Class sha256;
    sha256.init();
    for(uint8_t ii=0; ii < g_rolls.length(); ii++) {
        sha256.write(g_rolls[ii]);
    }
    memcpy(g_master_secret, sha256.result(), sizeof(g_master_secret));
    g_bip39.setPayloadBytes(sizeof(g_master_secret));
    g_bip39.setPayload(sizeof(g_master_secret), (uint8_t *)g_master_secret);
    for (int ndx = 0; ndx < 12; ++ndx) {
        uint16_t word = g_bip39.getWord(ndx);
        Serial.println(String(ndx) + " " + String(g_bip39.getMnemonic(word)));
    }
}

void make_slip39_wordlist() {
    int gt = 3;
    int gn = 5;
    
    char *ml[gn];
    
    for (int ii = 0; ii < gn; ++ii)
        ml[ii] = (char *)malloc(MNEMONIC_LIST_LEN);

    Serial.println("calling generate_mnemonics");
    generate_mnemonics(gt, gn,
                       g_master_secret, sizeof(g_master_secret),
                       NULL, 0, 0, ml);

    for (int ii = 0; ii < gn; ++ii)
        Serial.println(String(ii) + " " + String(ml[ii]));

    // Combine 2, 0, 4 to recover the secret.
    char *dml[5];
    dml[0] = ml[2];
    dml[1] = ml[0];
    dml[2] = ml[4];
    
    uint8_t ms[16];
    int msl;
    msl = sizeof(ms);
    combine_mnemonics(gt, dml, NULL, 0, ms, &msl);

    for (int ii = 0; ii < gn; ++ii)
        free(ml[ii]);
    
    if (msl == sizeof(g_master_secret) &&
        memcmp(g_master_secret, ms, msl) == 0) {
        Serial.println("SUCCESS");
    } else {
        Serial.println("FAIL");
    }
}

void verify_slip39_multilevel() {
    int gt = 5;
    char* ml[5] = {
                      "eraser senior decision roster beard "
                      "treat identify grumpy salt index "
                      "fake aviation theater cubic bike "
                      "cause research dragon emphasis counter",

                      "eraser senior ceramic snake clay "
                      "various huge numb argue hesitate "
                      "auction category timber browser greatest "
                      "hanger petition script leaf pickup",
                      
                      "eraser senior ceramic shaft dynamic "
                      "become junior wrist silver peasant "
                      "force math alto coal amazing "
                      "segment yelp velvet image paces",
#if 0
                      // Duplicate of the prior, should fail.
                      "eraser senior ceramic shaft dynamic "
                      "become junior wrist silver peasant "
                      "force math alto coal amazing "
                      "segment yelp velvet image paces",
#else
                      // This one works
                      "eraser senior ceramic round column "
                      "hawk trust auction smug shame "
                      "alive greatest sheriff living perfect "
                      "corner chest sled fumes adequate",
#endif
                      
                      "eraser senior decision smug corner "
                      "ruin rescue cubic angel tackle "
                      "skin skunk program roster trash "
                      "rumor slush angel flea amazing",
    };

    uint8_t ms[16];
    int msl;
    msl = sizeof(ms);
    int rc = combine_mnemonics(gt, (char**) ml, NULL, 0, ms, &msl);

    if (rc == 0) {
        Serial.println("VERIFIED " + String(rc));
    } else {
        Serial.println("FAILED " + String(rc));
    }
}
