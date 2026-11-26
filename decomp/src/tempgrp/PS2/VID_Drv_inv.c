#include "common.h"

#include "ps2/libc.h"
#include "ps2/r5900.h"
#include "renderware_fwd.h"
#include "sdk.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "GLI/GLI.h"
#include "ps2/PS2.h"
#include "GAM/GAM.h"

extern u8 dword_39A6F0[];

typedef void (*ThreadEntryFn)(void);
typedef s32 (*InterruptHandlerFn)(void);

extern char* off_3703F8[];
extern s32 inv_MPEG_interruptHandlerId;
extern s32 inv_MPEG_semaphoreId;
extern s32 inv_MPEG_threadId;
extern u32 inv_VID_ulMovieIndex;
extern u8 inv_MPEG_threadStack[];

char* strupr(char* s);
s32 AddIntcHandler(s32 cause, InterruptHandlerFn handler, s32 next);
s32 CreateSema(s32* params);
s32 DeleteSema(s32 semaphore);
s32 DeleteThread(s32 thread);
s32 RemoveIntcHandler(s32 cause, s32 handler);
s32 StartThread(s32 thread, s32 argument);
s32 TerminateThread(s32 thread);
s32 iSignalSema(s32 semaphore);
s32 scePrintf(char* format, ...);
void EnableIntc(s32 cause);
s32 inv_VID_fn_iCreateThread(ThreadEntryFn entry, u8* stack, s32 stackSize, s32 priority);

#ifndef PC_PORT
s32 inv_VID_fn_iMpegInterruptHandler(void)
{
    iSignalSema(inv_MPEG_semaphoreId);
    R5900_SYNC();
    R5900_EI();
    return 0;
}
#endif

void inv_VID_fn_vMpegThread(void)
{
    for (;;) {
#ifdef PC_PORT
        pc_sema_wait(inv_MPEG_semaphoreId);
#else
        WaitSema(inv_MPEG_semaphoreId);
#endif
        if (func_00215840() == 0) {
            func_00215668();
        }
    }
}

typedef struct ThreadDescriptor {
    s32 status;
    ThreadEntryFn entry;
    u8* stack;
    s32 stackSize;
    void* globalData;
    s32 priority;
    u8 _18[0x18];
} ThreadDescriptor;

#ifdef PC_PORT
s32 inv_VID_fn_iCreateThread(ThreadEntryFn entry, u8* stack, s32 stackSize, s32 priority)
{
    memset(stack, 0, stackSize);
    return pc_thread_create((PcAddress) entry, (PcAddress) stack, stackSize, priority);
}
#else
s32 inv_VID_fn_iCreateThread(ThreadEntryFn entry, u8* stack, s32 stackSize, s32 priority)
{
    ThreadDescriptor descriptor;

    memset(stack, 0, stackSize);
    do {
        descriptor.globalData = dword_39A6F0;
        descriptor.entry = entry;
        descriptor.stack = stack;
        descriptor.stackSize = stackSize;
        descriptor.priority = priority;
        return CreateThread(&descriptor);
    } while (0);
}
#endif

#ifdef PC_PORT
void inv_VID_fn_vStartMovie(u8 priority)
{
    char path[0x80];

    inv_MPEG_semaphoreId = pc_sema_create(0, 1);
    inv_VID_ulMovieIndex = (inv_VID_ulMovieIndex + 1) % 10;
    sprintf(path, "%s\\%s%s", "\\VIDEO", strupr(off_3703F8[inv_VID_ulMovieIndex]), ".ANI;1");
    D_0039378C = inv_VID_fn_bPlayMovie(path) == 0;
    pc_frame_vblank_signal(inv_MPEG_semaphoreId);
    inv_MPEG_threadId = inv_VID_fn_iCreateThread(inv_VID_fn_vMpegThread, inv_MPEG_threadStack, 0x4000, priority);
    pc_thread_set_priority(pc_thread_self(), priority + 1);
    pc_thread_start(inv_MPEG_threadId, 0);
}

void inv_VID_fn_vStopMovie(u8 priority)
{
    pc_thread_stop(inv_MPEG_threadId);
    pc_frame_vblank_signal(0);
    if (!pc_thread_delete(inv_MPEG_threadId)) {
        printf("mpeg_thread_id != DeleteThread\n");
    }
    pc_sema_delete(inv_MPEG_semaphoreId);
    pc_thread_set_priority(pc_thread_self(), priority);
    if (D_0039378C == 1) {
        inv_VID_fn_vFreeMpegVideo();
    }
    inv_VID_fn_vRestoreDisplay();
    inv_VID_fn_vClearScreen(inv_GLI_currentCamera);
    inv_VID_bMovieJustEnded = 1;
}
#else
void inv_VID_fn_vStartMovie(u8 priority)
{
    s32 semaphore[8];
    char path[0x80];

    semaphore[1] = 1;
    semaphore[2] = 0;
    inv_MPEG_semaphoreId = CreateSema(semaphore);
    inv_VID_ulMovieIndex = (inv_VID_ulMovieIndex + 1) % 10;
    sprintf(path, "%s\\%s%s", "\\VIDEO", strupr(off_3703F8[inv_VID_ulMovieIndex]), ".ANI;1");
    D_0039378C = inv_VID_fn_bPlayMovie(path) == 0;
    inv_MPEG_interruptHandlerId = AddIntcHandler(2, inv_VID_fn_iMpegInterruptHandler, 0);
    EnableIntc(2);
    inv_MPEG_threadId = inv_VID_fn_iCreateThread(inv_VID_fn_vMpegThread, inv_MPEG_threadStack, 0x4000, priority);
    ChangeThreadPriority(GetThreadId(), priority + 1);
    StartThread(inv_MPEG_threadId, 0);
}

void inv_VID_fn_vStopMovie(u8 priority)
{
    TerminateThread(inv_MPEG_threadId);
    RemoveIntcHandler(2, inv_MPEG_interruptHandlerId);
    if (inv_MPEG_threadId != DeleteThread(inv_MPEG_threadId)) {
        scePrintf("mpeg_thread_id != DeleteThread\n");
    }
    DeleteSema(inv_MPEG_semaphoreId);
    ChangeThreadPriority(GetThreadId(), priority);
    if (D_0039378C == 1) {
        inv_VID_fn_vFreeMpegVideo();
    }
    inv_VID_fn_vRestoreDisplay();
    inv_VID_fn_vClearScreen(inv_GLI_currentCamera);
    inv_VID_bMovieJustEnded = 1;
}
#endif
