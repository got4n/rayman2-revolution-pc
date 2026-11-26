#include "common.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"
#include "ps2/r5900.h"
#include "sdk.h"
#include "FMN/FMN.h"
#include "GLI/GLI.h"
#include "SVG/SVG.h"
#include "GAM/GAM.h"
#include "TMR/TMR.h"
#include "clib/clib.h"
#include "VIG/VIG.h"
#include "ps2/PS2.h"

#ifdef PC_PORT
#include "AI/AI.h"
#include "PC/PC.h"
#endif
typedef struct GifPacket {
    u128* current;
    u128* base;
    u32 tag;
} GifPacket;

typedef s32 (*InterruptHandler)(void);
extern char aIrx1[];
extern u8 off_362928[];

char* strupr(char* string);
s32 AddIntcHandler(s32 cause, InterruptHandler handler, s32 next);
#ifndef PC_PORT
s32 sceSifRebootIop(char* image);
s32 sceCdDiskReady(s32 mode);
s32 sceCdInit(s32 mode);
s32 sceCdMmode(s32 media);
s32 sceFsReset(void);
s32 sceGifPkSize(GifPacket* packet);
s32 sceMcGetDir(void* port, void* slot, void* name, s32 mode, s32 maxEntries, void* table);
s32 sceSifLoadModule(char* path, s32 argumentSize, char* arguments);
s32 sceSifSyncIop(void);
#endif
void EnableIntc(s32 cause);
void freemem(void* block);
void sceMpegInit(void);
#ifndef PC_PORT
void sceMcSync(s32 mode, s32* command, s32* result);
void sceSifInitRpc(s32 mode);
#endif
void* memalign(s32 alignment, s32 size);

#ifdef PC_PORT
s32 inv_SVG_fn_lListCard(char* path, s32 maxEntries, void* table);

s32 inv_SVG_fn_lEnumerateSaveDirectory(void* port, void* slot, void* name)
{
    void* table = (void*) (((u32) __builtin_alloca(0x80) + 0x3F) & ~0x3F);

    if (inv_SVG_fn_lListCard(name, 1, table) != 1) {
        return 0;
    }
    return inv_SVG_fn_lGetModificationTimeInSeconds(table);
}
#else
s32 inv_SVG_fn_lEnumerateSaveDirectory(void* port, void* slot, void* name)
{
    s32 status[4];
    void* table;
    s32 tries = 3;
    s32 count = 0;

    table = (void*) (((u32) __builtin_alloca(0x80) + 0x3F) & ~0x3F);
    gs_bSceMcOpIsSuccessful = 0;
    while (tries != 0) {
        gs_bSceMcOpIsSuccessful = sceMcGetDir(port, slot, name, 0, 1, table) == 0;
        if (gs_bSceMcOpIsSuccessful) {
            break;
        }
        if (--tries != 0) {
            fn_vTimerWait(1);
        }
    }
    if (gs_bSceMcOpIsSuccessful) {
        sceMcSync(0, NULL, status);
        if (status[0] == 1) {
            count = inv_SVG_fn_lGetModificationTimeInSeconds(table);
        }
    }
    return count;
}
#endif

#ifdef PC_PORT
void inv_PS2_fn_bLoadIopModuleFromDisc(char* name, char* arguments)
{
}
#else
void inv_PS2_fn_bLoadIopModuleFromDisc(char* name, char* arguments)
{
    s32 argumentSize;
    char path[0x80];

    if (arguments != NULL) {
        argumentSize = strlen(arguments) + 1;
    } else {
        argumentSize = 0;
    }
    strupr(name);
    strcpy(path, "cdrom0:\\MODS\\");
    Strcat(path, name);
    Strcat(path, aIrx1);
    sceCdDiskReady(0);
    while (sceSifLoadModule(path, argumentSize, arguments) < 0) {
    }
}
#endif

#ifndef PC_PORT
s32 func_00141400(void)
{
    inv_PS2_vblankCount++;
    R5900_SYNC();
    R5900_EI();
    return 0;
}
#endif

#ifdef PC_PORT
void ClearVRAM(u8 red, u8 green, u8 blue, u8 alpha)
{
}

s32 PS2_fn_lInit(void)
{
    inv_AI_parameterPoolCursor = PC_SCRATCH(0x70003E80);
    srand(0);
    inv_RW3_fn_vInitDefaultHeap(0x7C0419);
    inv_RW3_fn_vCreateWorldHeap(0x59999);
    inv_fn_vInitEngineClockLibrary();

    if (GLI_RW_bInitAPI(NULL, off_362928, NULL) == 0) {
        return 0;
    }

    inv_FMN_fn_lOpenGameArchive();
    GLI_xInitGli();

    inv_VIG_fn_vDisplayStartupImage("ubisoft", g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, 1);
    if (!pc_sound_start()) {
        printf("Can't start the sound driver\n");
    }

    if (SVG_fn_s32Init() != 1) {
        return 0;
    }

    return 1;
}
#else
void ClearVRAM(u8 r, u8 g, u8 b, u8 a)
{
    GifPacket packet;
    u64 blueGreen;
    u64 color;
    void* channel = sceDmaGetChan(2);
    void* buffer = memalign(0x80, 0xF0);

    if (buffer == NULL) {
        return;
    }
    sceGifPkInit(&packet, buffer);
    sceGifPkReset(&packet);

    sceGifPkAddGsData(&packet, ((u64) 0x10000000 << 32) | 0x8009);
    sceGifPkAddGsData(&packet, 0xE);
    sceGifPkAddGsData(&packet, 1);
    sceGifPkAddGsData(&packet, 0x1A);
    sceGifPkAddGsData(&packet, 0x100000);
    sceGifPkAddGsData(&packet, 0x4C);
    sceGifPkAddGsData(&packet, 0);
    sceGifPkAddGsData(&packet, 0x18);
    sceGifPkAddGsData(&packet, ((u64) 0x04000000 << 32) | 0x04000000);
    sceGifPkAddGsData(&packet, 0x40);
    sceGifPkAddGsData(&packet, 0);
    sceGifPkAddGsData(&packet, 0x47);
    sceGifPkAddGsData(&packet, 6);
    sceGifPkAddGsData(&packet, 0);
    blueGreen = ((u64) b << 16) | ((u64) g << 8);
    color = ((u64) 0x3F800000 << 32) | r;
    color |= blueGreen;
    sceGifPkAddGsData(&packet, color | ((u64) a << 24));
    sceGifPkAddGsData(&packet, 1);
    sceGifPkAddGsData(&packet, 0);
    sceGifPkAddGsData(&packet, 4);
    sceGifPkAddGsData(&packet, 0x40004000);
    sceGifPkAddGsData(&packet, 4);
    sceGifPkTerminate(&packet);
    FlushCache(0);

    sceDmaSendN(channel, (void*) ((u32) packet.base & 0x0FFFFFFF), sceGifPkSize(&packet));
    sceGsSyncPath(0, 0);
    freemem(buffer);
}

s32 PS2_fn_lInit(void)
{
    ClearVRAM(0, 0, 0, 0);

    if (sceCdInit(0) == 0) {
        printf("Can't initialize CDROM\n");
    }

    sceSifInitRpc(0);
    sceCdMmode(2);
    sceCdDiskReady(0);
    while (sceSifRebootIop("cdrom0:\\MODS\\IOPRP205.IMG;1") == 0) {
    }
    while (sceSifSyncIop() == 0) {
    }

    sceSifInitRpc(0);
    sceCdInit(0);
    sceCdMmode(2);
    sceFsReset();
    sceCdDiskReady(0);
    srand(0);
    inv_RW3_fn_vInitDefaultHeap(0x7C0419);
    inv_RW3_fn_vCreateWorldHeap(0x59999);
    inv_fn_vInitEngineClockLibrary();

    if (GLI_RW_bInitAPI(NULL, off_362928, NULL) == 0) {
        return 0;
    }

    AddIntcHandler(2, func_00141400, 0);
    EnableIntc(2);
    inv_FMN_fn_lOpenGameArchive();
    GLI_xInitGli();

    inv_VIG_fn_vDisplayStartupImage("ubisoft", g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, 1);
    inv_PS2_fn_bLoadIopModuleFromDisc("sio2man", NULL);
    inv_PS2_fn_bLoadIopModuleFromDisc("padman", NULL);
    inv_PS2_fn_bLoadIopModuleFromDisc("mcman", NULL);
    inv_PS2_fn_bLoadIopModuleFromDisc("mcserv", NULL);
    inv_PS2_fn_bLoadIopModuleFromDisc("libsd", NULL);
    inv_PS2_fn_bLoadIopModuleFromDisc("ubisound", NULL);

    if (SVG_fn_s32Init() != 1) {
        return 0;
    }

    sceMpegInit();
    return 1;
}
#endif
