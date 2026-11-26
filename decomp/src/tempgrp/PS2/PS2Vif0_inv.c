#include "common.h"

#include "ps2/dma.h"
#include "sdk.h"

#ifdef PC_PORT
#include "PC/PC.h"


PcVu0 pc_vu0 = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
unsigned char pc_scratchpad[0x4000] __attribute__((aligned(16)));

u32 inv_PS2_fn_bIsVu0Busy(void)
{
    return 0;
}

void inv_PS2_fn_vSendVif0Dma(void* tag)
{
    pc_vu0_load((PcAddress) tag);
}
#else
extern u32* inv_PS2_p_vVif0DmaChannel;

void inv_PS2_fn_vInitVif0Dma(void)
{
    inv_PS2_p_vVif0DmaChannel = sceDmaGetChan(0);
}

u32 inv_PS2_fn_bIsVu0Busy(void)
{
    return inv_sceVuGetControl29Masked5();
}

void inv_PS2_fn_vSendVif0Dma(void* tag)
{
    u32* channel;

    sceDmaSync((s32) inv_PS2_p_vVif0DmaChannel, 0, 0x7FFFFFFF);
    channel = inv_PS2_p_vVif0DmaChannel;
    *channel |= 0x40;
    inv_eeSync();
    sceDmaSend(channel, tag);
}

void inv_PS2_fn_vSendVif0DmaNoSync(void* tag)
{
    extern void* inv_PS2_p_vLastVif0Tag;

    inv_PS2_p_vLastVif0Tag = tag;
    sceDmaSend(inv_PS2_p_vVif0DmaChannel, tag);
}

s32 inv_PS2_fn_lIsVif0DmaBusy(void)
{
    return sceDmaSync((s32) inv_PS2_p_vVif0DmaChannel, 1, 0) & 0xFF;
}

void inv_PS2_fn_vWaitVif0Dma(void)
{
    sceDmaSync((s32) inv_PS2_p_vVif0DmaChannel, 0, 0);
}

s32 inv_PS2_fn_lWatchVif0Dma(void)
{
    extern void* const inv_PS2_p_vLastVif0Tag;
    u32* channel = inv_PS2_p_vVif0DmaChannel;

    *channel |= 0x100;
    return sceDmaWatch(channel, inv_PS2_p_vLastVif0Tag, 1, 0) & 0xFF;
}
#endif

#ifdef PC_PORT
void inv_PS2_fn_vCallVu0Microprogram(u32 address)
{
    pc_vu0_call(address, &pc_vu0);
}
#else
void inv_PS2_fn_vCallVu0Microprogram(u32 address)
{
    inv_sceVu0CallMs(address);
}
#endif
