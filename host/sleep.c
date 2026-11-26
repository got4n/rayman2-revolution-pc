#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

void revo_sleep_ns(uint64_t ns);
void revo_sleep_ns(uint64_t ns) {
#ifdef _WIN32
    static __thread HANDLE timer;
    static __thread int tried;
    if (!tried) {
        tried = 1;
        timer = CreateWaitableTimerExW(NULL, NULL, 0x00000002 ,
                                       TIMER_ALL_ACCESS);
    }
    if (timer) {
        LARGE_INTEGER due = {.QuadPart = -(LONGLONG)(ns / 100)};
        if (SetWaitableTimer(timer, &due, 0, NULL, NULL, FALSE)) {
            WaitForSingleObject(timer, INFINITE);
            return;
        }
    }
#endif
    struct timespec ts = {(time_t)(ns / 1000000000u), (long)(ns % 1000000000u)};
    nanosleep(&ts, NULL);
}
