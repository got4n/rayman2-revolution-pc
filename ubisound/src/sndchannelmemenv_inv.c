#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_u32AllocateVoiceTdstChannelMEMMngr_inv();

extern int fn_iConstructTdstChannelMEMMngr_inv();

extern char g_stTdstChannelMEMMngr_inv[];
extern int fn_iDestructTdstChannelMEMMngr_inv();
extern int fn_iGetNumFreeVoiceTdstChannelMEMMngr_inv();
extern int fn_u32DesallocateVoiceTdstChannelMEMMngr_inv();
extern int fn_bIsAllocatedTdstChannelMEMMngr_inv();
extern int fn_iStartTdstChannelMEMMngr_inv();
extern int fn_iStopTdstChannelMEMMngr_inv();
extern int fn_iSetOwnerTdstChannelMEMMngr_inv();
extern int fn_iConnectTdstChannelMEMMngr_inv();
extern int fn_u32ReallocateVoiceTdstChannelMEMMngr_inv();
extern int fn_iGetNumChildTdstChannelMEMMngr_inv();
extern int fn_iGetChildTdstChannelMEMMngr_inv();
extern int fn_iReadTdstChannelMEMMngr_inv();
extern int fn_iNotifyTdstChannelMEMMngr_inv();
extern int fn_iGetPosTdstChannelMEMMngr_inv();
extern int fn_vOperateCallTdstChannelMEMMngr_inv();
Fn_inv *fn_pstGetChannelMEMInterface_inv(void);

int fn_iInitChannelMEMEnv_inv(void)
{
    return fn_iConstructTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, fn_pstGetChannelMEMInterface_inv());
}

int fn_iDesInitChannelMEMEnv_inv(void)
{
    return fn_iDestructTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv);
}

int fn_iGetNumFreeVoiceChannelMEMEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv);
}

int fn_bTestFormatChannelMEMEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream < 0 && d->f.bFromCD >= 0 && d->cNbChannels == 2 && mode == 1;
}

int fn_u32AllocateVoiceChannelMEMEnv_inv(DescHead_inv *d)
{
    int r = 0;
    int n = d->cNbChannels;

    if (n < fn_iGetNumFreeVoiceTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv))
        r = fn_u32AllocateVoiceTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, d);
    return r;
}

int fn_u32DesallocateVoiceChannelMEMEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h);
}

int fn_bIsAllocatedChannelMEMEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h);
}

int fn_iReadChannelMEMEnv_inv(void *h, void *reader, int more, void *dst)
{
    return fn_iReadTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, reader, more, dst);
}

int fn_iStartChannelMEMEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, arg);
}

int fn_iStopChannelMEMEnv_inv(void *h)
{
    return fn_iStopTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h);
}

int fn_iSetOwnerChannelMEMEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, owner);
}

int fn_iConnectChannelMEMEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, arg);
}

int fn_u32ReallocateVoiceChannelMEMEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, desc);
}

int fn_iGetNumChildChannelMEMEnv_inv(void *h)
{
    return fn_iGetNumChildTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h);
}

int fn_iGetChildChannelMEMEnv_inv(void *h, Info_inv *out, int idx)
{
    return fn_iGetChildTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, out, idx);
}

int fn_iNotifyChannelMEMEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, event, from, arg);
}

int fn_iGetPosChannelMEMEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv, h, out);
}

int fn_iOperateCallTdstChannelMEMEnv_inv(void)
{
    return fn_vOperateCallTdstChannelMEMMngr_inv(g_stTdstChannelMEMMngr_inv);
}

static const Fn_inv a_pfnTdstChannelMEMInterface_inv[18] = {
    fn_iInitChannelMEMEnv_inv,
    fn_iDesInitChannelMEMEnv_inv,
    fn_iGetNumFreeVoiceChannelMEMEnv_inv,
    fn_bTestFormatChannelMEMEnv_inv,
    fn_u32AllocateVoiceChannelMEMEnv_inv,
    fn_u32ReallocateVoiceChannelMEMEnv_inv,
    fn_u32DesallocateVoiceChannelMEMEnv_inv,
    fn_iSetOwnerChannelMEMEnv_inv,
    fn_iConnectChannelMEMEnv_inv,
    fn_iGetNumChildChannelMEMEnv_inv,
    fn_iGetChildChannelMEMEnv_inv,
    fn_bIsAllocatedChannelMEMEnv_inv,
    fn_iNotifyChannelMEMEnv_inv,
    fn_iOperateCallTdstChannelMEMEnv_inv,
    fn_iReadChannelMEMEnv_inv,
    fn_iStartChannelMEMEnv_inv,
    fn_iStopChannelMEMEnv_inv,
    fn_iGetPosChannelMEMEnv_inv,
};

Fn_inv *fn_pstGetChannelMEMInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstChannelMEMInterface_inv;
}
