#include "common.h"

#include "ps2/libc.h"

typedef char* va_list;

typedef struct sceCdCLOCK {
    u8 stat;
    u8 second;
    u8 minute;
    u8 hour;
    u8 pad;
    u8 day;
    u8 month;
    u8 year;
} sceCdCLOCK;

#ifdef PC_PORT
#include "PC/PC.h"
#else
s32 sceCdReadClock(sceCdCLOCK* clock);
#endif

s32 inv_RUNTIME_fn_lReturnZero(s32 unused0, s32 unused1, ...)
{
    return 0;
}

s32 printf(const char* fmt, ...)
{
#ifndef PC_PORT
    va_list args;

    args = __builtin_next_arg(fmt);
#endif
    return 0;
}

s32 func_00211A88(u32 count)
{
    u32 i;

    count *= 0x3999;
    for (i = 0; i < count; i++) {
    }
    return 0;
}

char* inv_My_strchr(char* s, s32 c)
{
    while (*s != '\0' && *s != (char) c) {
        s++;
    }
    if (*s != '\0') {
        return s;
    }
    return NULL;
}

char* Strrchr(char* string, s32 ch)
{
    char* start = string;

    while (*string++ != '\0')
        ;
    do {
        string--;
    } while (string != start && *string != (char) ch);
    if (*string == (char) ch)
        return string;
    return NULL;
}

char* Strcat(char* s1, const char* s2)
{
    char* end = s1;

    while (*end != '\0') {
        end++;
    }
    strcpy(end, s2);
    return s1;
}

static inline s32 inv_CLIB_fn_lFromBcd(u8 value)
{
    return (value & 0xF) + (value >> 4) * 10;
}

#ifdef PC_PORT
s32 inv_fn_lGetCdClockSeconds(u64* out)
{
    PcDate date;
    s32 seconds;

    if (pc_clock_read(&date)) {
        seconds = date.second +
                  60 * (date.minute + 60 * (date.hour + 24 * (date.day + 31 * (date.month + 12 * date.year))));
    } else {
        seconds = 0;
    }
    if (out != NULL) {
        *out = seconds;
    }
    return seconds;
}
#else
s32 inv_fn_lGetCdClockSeconds(u64* out)
{
    sceCdCLOCK clock;
    s32 seconds;

    if (sceCdReadClock(&clock) != 0) {
        seconds = inv_CLIB_fn_lFromBcd(clock.second) +
                  60 * (inv_CLIB_fn_lFromBcd(clock.minute) +
                        60 * (inv_CLIB_fn_lFromBcd(clock.hour) +
                              24 * (inv_CLIB_fn_lFromBcd(clock.day) +
                                    31 * (inv_CLIB_fn_lFromBcd(clock.month) + 12 * inv_CLIB_fn_lFromBcd(clock.year)))));
    } else {
        seconds = 0;
    }
    if (out != NULL) {
        *out = seconds;
    }
    return seconds;
}
#endif
