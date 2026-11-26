#ifndef REVO_SDK_H
#define REVO_SDK_H

#include "ps2/libc.h"
#include "ps2/types.h"

typedef struct OsdParams {
    u32 param;
    u8 param2;
} OsdParams;

typedef struct ThreadParam {
    int status;
    void (*entry)(void*);
    void* stack;
    int stackSize;
    void* gpReg;
    int initPriority;
    int currentPriority;
    unsigned int attr;
    unsigned int option;
    int waitType;
    int waitId;
    int wakeupCount;
} ThreadParam;

typedef void (*sceSifEndFunc)(void*);

typedef struct sceSifRpcData {
    void* paddr;
    u32 pid;
    int tid;
    u32 mode;
} sceSifRpcData;

typedef struct sceSifClientData {
    sceSifRpcData rpcd;
    u32 command;
    void* buff;
    void* cbuff;
    sceSifEndFunc func;
    void* para;
    struct _sif_serve_data* serve;
} sceSifClientData;

extern void Exit(int code);
extern void SleepThread(void);
extern void WaitSema(s32 sema);

extern void _Error(const char* msg);
extern void __main(void);
extern int CreateThread(ThreadParam*);
extern int ReferThreadStatus(int, ThreadParam*);
extern s32 ChangeThreadPriority(s32, s32);
extern s32 GetThreadId(void);
extern int fflush(void*);
extern int _fwalk(int (*)(void*));
extern void* _free_r(void*, void*);

extern void* sceDmaGetChan(s32 chan);
extern void sceDmaSend(void* chan, void* addr);
extern void sceDmaSendN(void* chan, void* addr, s32 size);
extern s32 sceDmaSync(s32, s32, s32);
extern s32 sceDmaWatch(void* chan, void* tag, s32 mode, s32 timeout);
extern void sceGifPkInit(void* pk, void* buf);
extern void sceGifPkReset(void* pk);
extern void sceGifPkAddGsData(void* pk, u64 data);
extern void sceGifPkTerminate(void* pk);
extern void FlushCache(s32 mode);
extern void sceGsSyncPath(s32 a0, s32 a1);

#ifndef PC_PORT
static inline u32 inv_sceVuGetControl29Masked5(void)
{
    u32 value;
    __asm__ volatile("cfc2.ni %0, $vi29\n\t"
                     "andi %0, %0, 5\n\t"
                     "addiu $3, $0, 0"
                     : "=r"(value)
                     :
                     : "$3");
    return value;
}

static inline void inv_sceVu0CallMs(u32 addr)
{
    __asm__ volatile("ctc2.ni %0, $vi27\n\t"
                     "vnop\n\t"
                     "vnop\n\t"
                     "vcallmsr $vi27"
                     :
                     : "r"(addr)
                     : "memory");
}
#endif

#ifdef PC_PORT
#define SCE_VU0_LQC2(n, ptr) __builtin_memcpy(pc_vu0.vf[n], (ptr), 16)
#define SCE_VU0_CALLMS_IMM(addr) pc_vu0_call((addr) >> 3, &pc_vu0)
#else
#define SCE_VU0_LQC2(vf, ptr)                                                                      \
    __asm__ volatile("lqc2 $vf" #vf ", 0(%0)\n\t"                                                  \
                     ".set noreorder\n\t"                                                          \
                     ".set reorder"                                                                \
                     :                                                                             \
                     : "r"(ptr))

#define SCE_VU0_CALLMS_IMM(addr)                                                                   \
    __asm__ volatile("vcallms " #addr "\n\t"                                                       \
                     ".set noreorder\n\t"                                                          \
                     ".set reorder")
#endif

#define SCE_DEV_GIF_WRITE_CTRL8()                                                                  \
    __asm__ volatile(".set noreorder\n\t"                                                          \
                     "lui $3, 0x1000\n\t"                                                          \
                     "addiu $4, $0, 8\n\t"                                                         \
                     "ori $3, $3, 0x3000\n\t"                                                      \
                     "addiu $2, $0, 1\n\t"                                                         \
                     "jr $31\n\t"                                                                  \
                     "sw $4, 0($3)\n\t"                                                            \
                     ".set reorder"                                                                \
                     :                                                                             \
                     :                                                                             \
                     : "memory", "$2", "$3", "$4")

#define SCE_DEV_VIF0_WRITE_STAT(a0, a1, a2)                                                        \
    __asm__ volatile(".set noreorder\n\t"                                                          \
                     "sll $5, %1, 1\n\t"                                                           \
                     "sll $6, %2, 2\n\t"                                                           \
                     "or $2, %0, $5\n\t"                                                           \
                     "lui $3, 0x1000\n\t"                                                          \
                     "or $2, $2, $6\n\t"                                                           \
                     "ori $3, $3, 0x3820\n\t"                                                      \
                     "jr $31\n\t"                                                                  \
                     "sw $2, 0($3)\n\t"                                                            \
                     ".set reorder"                                                                \
                     :                                                                             \
                     : "r"(a0), "r"(a1), "r"(a2)                                                   \
                     : "memory", "$2", "$3", "$5", "$6")

#define SCE_DEV_VIF1_WRITE_STAT(a0, a1, a2)                                                        \
    __asm__ volatile(".set noreorder\n\t"                                                          \
                     "sll $5, %1, 1\n\t"                                                           \
                     "sll $6, %2, 2\n\t"                                                           \
                     "or $2, %0, $5\n\t"                                                           \
                     "lui $3, 0x1000\n\t"                                                          \
                     "or $2, $2, $6\n\t"                                                           \
                     "ori $3, $3, 0x3C20\n\t"                                                      \
                     "jr $31\n\t"                                                                  \
                     "sw $2, 0($3)\n\t"                                                            \
                     ".set reorder"                                                                \
                     :                                                                             \
                     : "r"(a0), "r"(a1), "r"(a2)                                                   \
                     : "memory", "$2", "$3", "$5", "$6")

#ifndef PC_PORT
static inline void inv_eeSync(void)
{
    __asm__ volatile("sync");
}
#endif

#endif
