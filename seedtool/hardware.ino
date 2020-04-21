// Copyright © 2020 Blockchain Commons, LLC

// Only define one of these
#undef ESP32
#define SAMD51	1

#include "hardware.h"
#include "util.h"

#if defined(SAMD51)
extern "C" {
#include "trngFunctions.h"
}
#endif

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
GxEPD2_BW<EPD_DRIVER, EPD_DRIVER::HEIGHT>
g_display(EPD_DRIVER(/*CS=*/   21,
                     /*DC=*/   17,
                     /*RST=*/  16,
                     /*BUSY=*/ 4));
#elif defined(SAMD51)
GxEPD2_BW<EPD_DRIVER, EPD_DRIVER::HEIGHT>
g_display(EPD_DRIVER(/*CS=*/   PIN_A4,
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

void hw_setup() {
    pinMode(BLUE_LED, OUTPUT);	// Blue LED
    digitalWrite(BLUE_LED, HIGH);
    
    pinMode(GREEN_LED, OUTPUT);	// Green LED
    digitalWrite(GREEN_LED, LOW);
    
    g_display.init(115200);
    g_display.setRotation(1);

    Serial.begin(115200);
    
#if defined(SAMD51)
    trngInit();
#endif

    // Only wait on the ESP32, the SAMD51 gets hung w/o serial here
#if defined(ESP32)
    while (!Serial);	// wait for serial to come online
#endif
}

void hw_green_led(int value) {
    digitalWrite(GREEN_LED, value);  // turn off the green LED
}    

extern "C" {

void hw_random_buffer(uint8_t *buf, size_t len) {
    serial_printf("random_buffer %d\n", len);
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

} // extern "C"
