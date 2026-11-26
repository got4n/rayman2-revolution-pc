#include <time.h>

#include "PC.h"

int32_t pc_clock_read(PcDate *date)
{
    time_t now = time(NULL);
    struct tm local;

    if (now == (time_t) -1) {
        return 0;
    }
#ifdef _WIN32
    if (localtime_s(&local, &now) != 0) {
        return 0;
    }
#else
    if (localtime_r(&now, &local) == NULL) {
        return 0;
    }
#endif
    date->second = (uint8_t) (local.tm_sec > 59 ? 59 : local.tm_sec);
    date->minute = (uint8_t) local.tm_min;
    date->hour = (uint8_t) local.tm_hour;
    date->day = (uint8_t) local.tm_mday;
    date->month = (uint8_t) (local.tm_mon + 1);
    date->year = (uint8_t) (local.tm_year % 100);
    return 1;
}
