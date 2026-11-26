#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_u32AllocateVoiceTdstChannelMEMMonoMngr_inv();

extern int fn_iConstructTdstChannelMEMMonoMngr_inv();

extern char g_stTdstChannelMEMMonoMngr_inv[];
extern int fn_iDestructTdstChannelMEMMonoMngr_inv();
extern int fn_iGetNumFreeVoiceTdstChannelMEMMonoMngr_inv();
extern int fn_u32ReallocateVoiceTdstChannelMEMMonoMngr_inv();
extern int fn_u32DesallocateVoiceTdstChannelMEMMonoMngr_inv();
extern int fn_iSetOwnerTdstChannelMEMMonoMngr_inv();
extern int fn_iConnectTdstChannelMEMMonoMngr_inv();
extern int fn_bIsAllocatedTdstChannelMEMMonoMngr_inv();
extern int fn_iNotifyTdstChannelMEMMonoMngr_inv();
extern int fn_iStartTdstChannelMEMMonoMngr_inv();
extern int fn_iStopTdstChannelMEMMonoMngr_inv();
extern int fn_iReadTdstChannelMEMMonoMngr_inv();
extern int fn_iGetPosTdstChannelMEMMonoMngr_inv();
extern int fn_vOperateCallTdstChannelMEMMonoMngr_inv();
Fn_inv *fn_pstGetChannelMEMMonoInterface_inv(void);

int fn_iInitChannelMEMMonoEnv_inv(void)
{
    return fn_iConstructTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, fn_pstGetChannelMEMMonoInterface_inv());
}

int fn_iDesInitChannelMEMMonoEnv_inv(void)
{
    return fn_iDestructTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv);
}

int fn_iGetNumFreeVoiceChannelMEMMonoEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv);
}

int fn_bTestFormatChannelMEMMonoEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream < 0 && d->f.bFromCD >= 0 && d->cNbChannels == 1 && mode == 1;
}

int fn_u32AllocateVoiceChannelMEMMonoEnv_inv(Desc_inv *d)
{
    int r = 0;
    int n = d->h.cNbChannels;

    if (fn_iGetNumFreeVoiceTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv) >= n)
        r = fn_u32AllocateVoiceTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, &d->u);
    return r;
}

int fn_u32DesallocateVoiceChannelMEMMonoEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h);
}

int fn_bIsAllocatedChannelMEMMonoEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h);
}

int fn_iReadChannelMEMMonoEnv_inv(void *h, void *reader, int more, void *dst)
{
    return fn_iReadTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, reader, more, dst);
}

int fn_iStartChannelMEMMonoEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, arg);
}

int fn_iStopChannelMEMMonoEnv_inv(void *h)
{
    return fn_iStopTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h);
}

int fn_iSetOwnerChannelMEMMonoEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, owner);
}

int fn_iConnectChannelMEMMonoEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, arg);
}

int fn_u32ReallocateVoiceChannelMEMMonoEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, &desc->u);
}

int fn_iGetNumChildChannelMEMMonoEnv_inv(void)
{
    return 0;
}

int fn_iGetChildChannelMEMMonoEnv_inv(void)
{
    return -1;
}

int fn_iNotifyChannelMEMMonoEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, event, from, arg);
}

int fn_iGetPosChannelMEMMonoEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv, h, out);
}

int fn_iOperateCallTdstChannelMEMMonoEnv_inv(void)
{
    return fn_vOperateCallTdstChannelMEMMonoMngr_inv(g_stTdstChannelMEMMonoMngr_inv);
}

static const Fn_inv a_pfnTdstChannelMEMMonoInterface_inv[18] = {
    fn_iInitChannelMEMMonoEnv_inv,
    fn_iDesInitChannelMEMMonoEnv_inv,
    fn_iGetNumFreeVoiceChannelMEMMonoEnv_inv,
    fn_bTestFormatChannelMEMMonoEnv_inv,
    fn_u32AllocateVoiceChannelMEMMonoEnv_inv,
    fn_u32ReallocateVoiceChannelMEMMonoEnv_inv,
    fn_u32DesallocateVoiceChannelMEMMonoEnv_inv,
    fn_iSetOwnerChannelMEMMonoEnv_inv,
    fn_iConnectChannelMEMMonoEnv_inv,
    fn_iGetNumChildChannelMEMMonoEnv_inv,
    fn_iGetChildChannelMEMMonoEnv_inv,
    fn_bIsAllocatedChannelMEMMonoEnv_inv,
    fn_iNotifyChannelMEMMonoEnv_inv,
    fn_iOperateCallTdstChannelMEMMonoEnv_inv,
    fn_iReadChannelMEMMonoEnv_inv,
    fn_iStartChannelMEMMonoEnv_inv,
    fn_iStopChannelMEMMonoEnv_inv,
    fn_iGetPosChannelMEMMonoEnv_inv,
};

Fn_inv *fn_pstGetChannelMEMMonoInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstChannelMEMMonoInterface_inv;
}
