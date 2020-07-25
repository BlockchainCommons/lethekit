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

  for (int i=0; i<len; i++) {
    if (data[i] < 16) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
  }

  Serial.println("");
}

bool compare_bytes_with_hex(uint8_t *data, size_t len, char * hex) {
  bool ret;
  char *str = (char *)malloc(4*len);

  for (int i=0; i < len; i++) {
      sprintf(str + i*2, "%02x", data[i]);
  }

  str[2*len] = 0;

  if (strcmp(str, hex) == 0)
    ret = true;
  else
    ret = false;

  free(str);

  return ret;
}
