// Copyright © 2020 Blockchain Commons, LLC

#ifndef _UTIL_H
#define _UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void serial_printf(char *format, ...);

#ifdef __cplusplus
}
#endif

#define serial_assert(_exp) \
    do {                                                                \
        if (!(_exp)) {                                                  \
            serial_printf("ASSERTION FAILED: %s:%d\n",                  \
                          __FILE__, __LINE__);                          \
            abort();                                                    \
        }                                                               \
    } while (false)

#endif // _UTIL_H
