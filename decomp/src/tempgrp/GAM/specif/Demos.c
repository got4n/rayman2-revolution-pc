#include "common.h"

#include "GAM/GAM.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"
#include "FMN/FMN.h"
#include "AI/AI.h"
#include "INO/INO.h"

#define DEMO_MAX_ATTEMPTS 100
#define FMN_MAX_HANDLE 0xF

typedef struct DemoPlaybackState {
    void* allocationReference;
    PS2_tdstControllerData_* records;
    u16 recordCount;
    u16 recordCursor;
} DemoPlaybackState;

extern s32 inv_DEMO_ulPendingState;
extern DemoPlaybackState inv_DEMO_playback;
extern char inv_DEMO_szSavedLevelName[];

s32 FMN_Read(s32 handle, void* buffer, s32 count, s32 size);

void inv_DEMO_fn_vResetSeedAndInitSound(void)
{
    srand(0);
    inv_SIF_fn_vInitSoundInfos();
}

s32 inv_DEMO_fn_bFindAndLoadNextDemo(void)
{
    char name[0x20];
    char path[0x40];
    s32 handle;
    s32 attempt = 0;

    do {
        attempt++;
        if (attempt > DEMO_MAX_ATTEMPTS) {
            return 0;
        }
        inv_fn_vGetRandomLevelName(name);
        sprintf(path, "%s/%s.dmo", name, name);
        handle = inv_FMN_fn_uOpenVirtualOrCD(path, 1);
    } while ((u32) handle >= FMN_MAX_HANDLE);

    FMN_Close(handle);
    strcpy(inv_DEMO_szSavedLevelName, inv_fn_p_szGetFullLevelName());
    GAM_fn_vAskToChangeLevel(name);
    return 1;
}

void inv_DEMO_fn_vLoadDemoFile(void)
{
    char path[0x40];
    char name[0x40];
    s32 handle;

    strcpy(name, fn_p_szGetLevelName());
    sprintf(path, "%s/%s.dmo", name, name);
    handle = inv_FMN_fn_uOpenVirtualOrCD(path, 1);
    if ((u32) handle < FMN_MAX_HANDLE) {
        FMN_Read(handle, &inv_DEMO_playback.recordCount, 1, sizeof(u16));
        inv_DEMO_playback.records = AI_fn_p_vTrueAlloc(
            gp_6F74, inv_DEMO_playback.recordCount * sizeof(PS2_tdstControllerData_));
        FMN_Read(handle, inv_DEMO_playback.records, 1,
                 inv_DEMO_playback.recordCount * sizeof(PS2_tdstControllerData_));
        FMN_Close(handle);
        inv_DEMO_playback.recordCursor = 0;
        inv_DEMO_fn_vResetSeedAndInitSound();
    }
}

void inv_DEMO_fn_vFreeDemoBuffer(void)
{
    if (inv_DEMO_bPlaying == 0 || inv_DEMO_playback.records == NULL) {
        return;
    }
    inv_MEM_fn_vFreeInHeap(gp_6F74, inv_DEMO_playback.allocationReference);
    inv_DEMO_playback.allocationReference = NULL;
    inv_DEMO_playback.records = NULL;
}

void inv_DEMO_fn_vStartAutoPlay(void)
{
    if (inv_DEMO_bPlaying != 0) {
        return;
    }
    if (inv_DEMO_fn_bFindAndLoadNextDemo() != 0) {
        inv_DEMO_bPlaying = 1;
    }
    inv_DEMO_ulPendingState = 5;
}

void inv_DEMO_fn_vEndDemoPlayback(void)
{
    if (inv_DEMO_bPlaying == 0) {
        return;
    }
    inv_DEMO_fn_vFreeDemoBuffer();
    inv_DEMO_bPlaying = 0;
    fn_vSetLevelName(inv_DEMO_szSavedLevelName);
    GAM_fn_vAskToChangeLevel("");
    inv_DEMO_ulPendingState = 0x46;
}

s32 inv_DEMO_fn_vReadNextDemoRecord(PS2_tdstControllerData_* packet, s16* status)
{
    DemoPlaybackState* demo;
    u32 next;
    s32 result;
    u16 value;

    if (inv_DEMO_bPlaying != 0) {
        demo = &inv_DEMO_playback;
        if (demo->records == NULL)
            inv_DEMO_fn_vLoadDemoFile();
        do {
            *packet = demo->records[demo->recordCursor];
        } while (0);
        value = packet->u_inv.stDemo_inv.inv_uwReadStatus;
        do {
            *status = value;
            next = demo->recordCursor + 1;
            result = (u16) next < demo->recordCount;
        } while (0);
        demo->recordCursor = next;
        if (!result) {
            inv_DEMO_fn_vEndDemoPlayback();
            result = 5;
            inv_DEMO_ulPendingState = result;
        }
    } else {
        result = -1;
        *status = result;
    }
    return result;
}

u32 inv_DEMO_fn_uGetCurrentRecordFlags(void)
{
    if (inv_DEMO_bPlaying != 0) {
        if (inv_DEMO_playback.records == NULL) {
            inv_DEMO_fn_vLoadDemoFile();
        }
        return inv_DEMO_playback.records[inv_DEMO_playback.recordCursor]
            .u_inv.stDemo_inv.inv_uwStepCount;
    }
    return 0;
}

u32 inv_DEMO_fn_lGetPendingState(void)
{
    return inv_DEMO_ulPendingState;
}
