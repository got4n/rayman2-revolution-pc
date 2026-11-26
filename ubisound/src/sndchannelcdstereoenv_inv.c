#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_u32AllocateVoiceTdstChannelCDStereoMngr_inv();

extern int fn_iConstructTdstChannelCDStereoMngr_inv();

extern char g_stTdstChannelCDStereoMngr_inv[];
extern int fn_iDestructTdstChannelCDStereoMngr_inv();
extern int fn_iGetNumFreeVoiceTdstChannelCDStereoMngr_inv();
extern int fn_u32DesallocateVoiceTdstChannelCDStereoMngr_inv();
extern int fn_bIsAllocatedTdstChannelCDStereoMngr_inv();
extern int fn_iStartTdstChannelCDStereoMngr_inv();
extern int fn_iStopTdstChannelCDStereoMngr_inv();
extern int fn_iReadTdstChannelCDStereoMngr_inv();
extern int fn_iSetOwnerTdstChannelCDStereoMngr_inv();
extern int fn_iConnectTdstChannelCDStereoMngr_inv();
extern int fn_u32ReallocateVoiceTdstChannelCDStereoMngr_inv();
extern int fn_iGetNumChildTdstChannelCDStereoMngr_inv();
extern int fn_iGetChildTdstChannelCDStereoMngr_inv();
extern int fn_iNotifyTdstChannelCDStereoMngr_inv();
extern int fn_iGetPosTdstChannelCDStereoMngr_inv();
extern int fn_vOperateCallTdstChannelCDStereoMngr_inv();
Fn_inv *fn_pstGetChannelCDStereoInterface_inv(void);

int fn_iInitChannelCDStereoEnv_inv(void)
{
    return fn_iConstructTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, fn_pstGetChannelCDStereoInterface_inv());
}

int fn_iDesInitChannelCDStereoEnv_inv(void)
{
    return fn_iDestructTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv);
}

int fn_iGetNumFreeVoiceChannelCDStereoEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv);
}

int fn_bTestFormatChannelCDStereoEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream < 0 && d->f.bFromCD < 0 && d->f.bInterleaved < 0 && mode == 1;
}

int fn_u32AllocateVoiceChannelCDStereoEnv_inv(DescHead_inv *d)
{
    int r = 0;

    if (d->cNbChannels == 2 && fn_iGetNumFreeVoiceTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv) > 0)
        r = fn_u32AllocateVoiceTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, d);
    return r;
}

int fn_u32DesallocateVoiceChannelCDStereoEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h);
}

int fn_bIsAllocatedChannelCDStereoEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h);
}

int fn_iReadChannelCDStereoEnv_inv(void *h, void *reader, int more, void *dst)
{
    return fn_iReadTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, reader, more, dst);
}

int fn_iStartChannelCDStereoEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, arg);
}

int fn_iStopChannelCDStereoEnv_inv(void *h)
{
    return fn_iStopTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h);
}

int fn_iSetOwnerChannelCDStereoEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, owner);
}

int fn_iConnectChannelCDStereoEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, arg);
}

int fn_u32ReallocateVoiceChannelCDStereoEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, desc);
}

int fn_iGetNumChildChannelCDStereoEnv_inv(void *h)
{
    return fn_iGetNumChildTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h);
}

int fn_iGetChildChannelCDStereoEnv_inv(void *h, Info_inv *out, int idx)
{
    return fn_iGetChildTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, out, idx);
}

int fn_iNotifyChannelCDStereoEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, event, from, arg);
}

int fn_iOperateCallTdstChannelCDStereoEnv_inv(void)
{
    return fn_vOperateCallTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv);
}

int fn_iGetPosChannelCDStereoEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstChannelCDStereoMngr_inv(g_stTdstChannelCDStereoMngr_inv, h, out);
}

static const Fn_inv a_pfnTdstChannelCDStereoInterface_inv[18] = {
    fn_iInitChannelCDStereoEnv_inv,
    fn_iDesInitChannelCDStereoEnv_inv,
    fn_iGetNumFreeVoiceChannelCDStereoEnv_inv,
    fn_bTestFormatChannelCDStereoEnv_inv,
    fn_u32AllocateVoiceChannelCDStereoEnv_inv,
    fn_u32ReallocateVoiceChannelCDStereoEnv_inv,
    fn_u32DesallocateVoiceChannelCDStereoEnv_inv,
    fn_iSetOwnerChannelCDStereoEnv_inv,
    fn_iConnectChannelCDStereoEnv_inv,
    fn_iGetNumChildChannelCDStereoEnv_inv,
    fn_iGetChildChannelCDStereoEnv_inv,
    fn_bIsAllocatedChannelCDStereoEnv_inv,
    fn_iNotifyChannelCDStereoEnv_inv,
    fn_iOperateCallTdstChannelCDStereoEnv_inv,
    fn_iReadChannelCDStereoEnv_inv,
    fn_iStartChannelCDStereoEnv_inv,
    fn_iStopChannelCDStereoEnv_inv,
    fn_iGetPosChannelCDStereoEnv_inv,
};

Fn_inv *fn_pstGetChannelCDStereoInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstChannelCDStereoInterface_inv;
}
