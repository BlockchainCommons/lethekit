// Copyright © 2020 Blockchain Commons, LLC

#include "util.h"

void serial_printf(const char *format, ...) {
  char buff[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format, args);
  va_end(args);
  buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
  Serial.print(buff);
}

void print_hex(uint8_t *data, size_t len) {

  for (size_t i=0; i<len; i++) {
    if (data[i] < 16) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
  }

  Serial.println("");
}

bool compare_bytes_with_hex(uint8_t *data, size_t len, const char * hex) {
  char str[3] = {0};

  for (size_t i=0; i < len; i++) {
      sprintf(str, "%02x", data[i]);
      if (memcmp(str, hex + 2*i, 2) != 0) {
          return false;
      }
  }

  return true;
}

// This Chunk of code takes a string and separates it based on a given character and returns The item between the separating character
// source https://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
String get_word_from_sentence(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
