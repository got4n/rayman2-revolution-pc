#include "PC.h"
#include "pc_host.h"

int32_t pc_thread_create(PcAddress entry, PcAddress stack, uint32_t stackSize, int32_t priority)
{
    return revo_thread_create(entry, stack, stackSize, priority);
}

int32_t pc_thread_start(int32_t thread, uint32_t argument)
{
    return revo_thread_start(thread, argument);
}

int32_t pc_thread_stop(int32_t thread)
{
    return revo_thread_stop(thread);
}

int32_t pc_thread_delete(int32_t thread)
{
    return revo_thread_delete(thread);
}

int32_t pc_thread_self(void)
{
    return revo_thread_self();
}

int32_t pc_thread_priority(int32_t thread)
{
    return revo_thread_priority(thread);
}

void pc_thread_set_priority(int32_t thread, int32_t priority)
{
    revo_thread_set_priority(thread, priority);
}

void pc_thread_sleep(void)
{
    revo_thread_sleep();
}

void pc_thread_wake(int32_t thread)
{
    revo_thread_wake(thread);
}

int32_t pc_sema_create(int32_t count, int32_t max)
{
    return revo_sema_create(count, max);
}

void pc_sema_delete(int32_t sema)
{
    revo_sema_delete(sema);
}

void pc_sema_wait(int32_t sema)
{
    revo_sema_wait(sema);
}

void pc_sema_signal(int32_t sema)
{
    revo_sema_signal(sema);
}

uint64_t pc_time_ticks(uint32_t frequency)
{
    return revo_time_ticks(frequency);
}
