#include "common.h"

#include "snd/snd.h"

#include "ps2/kernel_types.h"
#include "sdk.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

extern s32 gi_SemaSection;

s32 CreateSema(ee_sema_t* sema);
s32 DeleteSema(s32 sema);
s32 SignalSema(s32 sema);

void SND_fn_vInitThreadSnd(void)
{
    ee_sema_t sema;
    s32 id;

    if (gi_SemaSection >= 0)
        return;
    sema.max_count = 1;
    sema.option = (s32) &gi_SemaSection;
    sema.count = 0;
    sema.init_count = 0;
#ifdef PC_PORT
    id = pc_sema_create(sema.init_count, sema.max_count);
#else
    id = CreateSema(&sema);
#endif
    gi_SemaSection = id;
#ifdef PC_PORT
    pc_sema_signal(id);
#else
    SignalSema(id);
#endif
}

void SND_fn_vDesInitThreadSnd(void)
{
    if (gi_SemaSection >= 0) {
#ifdef PC_PORT
        pc_sema_delete(gi_SemaSection);
#else
        DeleteSema(gi_SemaSection);
#endif
        gi_SemaSection = -1;
    }
}

void SND_fn_vEnterCriticalSectionThreadSnd_(void)
{
#ifdef PC_PORT
    pc_sema_wait(gi_SemaSection);
#else
    WaitSema(gi_SemaSection);
#endif
}

void SND_fn_vEnterCriticalSectionThreadSnd(void)
{
#ifdef PC_PORT
    pc_sema_wait(gi_SemaSection);
#else
    WaitSema(gi_SemaSection);
#endif
}

void func_0015F7D0(void)
{
}

s32 func_0015F7D8(void)
{
    return 0;
}

void SND_fn_vQuitCriticalSectionThreadSnd(void)
{
#ifdef PC_PORT
    pc_sema_signal(gi_SemaSection);
#else
    SignalSema(gi_SemaSection);
#endif
}

void SND_fn_vMouchardThreadsnd(char* file, s32 line)
{
}

void func_0015F7F0(void)
{
}

s32 func_0015F7F8(void)
{
    return 0;
}

void SND_fn_vEnterCriticalSectionForErrorDisplay(void)
{
}

void SND_fn_vQuitCriticalSectionForErrorDisplay(void)
{
}
