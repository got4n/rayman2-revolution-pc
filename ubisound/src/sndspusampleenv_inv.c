#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_u32AllocateVoiceTdstSpuSampleMngr_inv();

extern int fn_iConstructTdstSpuSampleMngr_inv();

extern char g_stTdstSpuSampleMngr_inv[];
extern int fn_iDestructTdstSpuSampleMngr_inv();
extern int fn_iGetNumFreeVoiceTdstSpuSampleMngr_inv();
extern int fn_u32ReallocateVoiceTdstSpuSampleMngr_inv();
extern int fn_u32DesallocateVoiceTdstSpuSampleMngr_inv();
extern int fn_iSetOwnerTdstSpuSampleMngr_inv();
extern int fn_iConnectTdstSpuSampleMngr_inv();
extern int fn_bIsAllocatedTdstSpuSampleMngr_inv();
extern int fn_iNotifyTdstSpuSampleMngr_inv();
extern int fn_iStartTdstSpuSampleMngr_inv();
extern int fn_iStopTdstSpuSampleMngr_inv();
extern int fn_iSetParamTdstSpuSampleMngr_inv();
extern int fn_iGetPosTdstSpuSampleMngr_inv();
extern int fn_iPauseTdstSpuSampleMngr_inv();
extern int fn_iResumeTdstSpuSampleMngr_inv();
extern int fn_vOperateCallTdstSpuSampleMngr_inv();
Fn_inv *fn_pstGetSpuSampleInterface_inv(void);

int fn_iInitSpuSampleEnv_inv(void)
{
    return fn_iConstructTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, fn_pstGetSpuSampleInterface_inv());
}

int fn_iDesInitSpuSampleEnv_inv(void)
{
    return fn_iDestructTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv);
}

int fn_iGetNumFreeVoiceSpuSampleEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv);
}

int fn_bTestFormatSpuSampleEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream >= 0 && d->cNbChannels == 1 && mode == 2;
}

int fn_u32AllocateVoiceSpuSampleEnv_inv(DescHead_inv *d)
{
    int r = 0;
    int n = d->cNbChannels;

    if (fn_iGetNumFreeVoiceTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv) >= n)
        r = fn_u32AllocateVoiceTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, d);
    return r;
}

int fn_u32ReallocateVoiceSpuSampleEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, desc);
}

int fn_u32DesallocateVoiceSpuSampleEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h);
}

int fn_iSetOwnerSpuSampleEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, owner);
}

int fn_iConnectSpuSampleEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, arg);
}

int fn_iGetNumChildSpuSampleEnv_inv(void)
{
    return 0;
}

int fn_iGetChildSpuSampleEnv_inv(void)
{
    return -1;
}

int fn_bIsAllocatedSpuSampleEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h);
}

int fn_iNotifySpuSampleEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, event, from, arg);
}

int fn_iStartSpuSampleEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, arg);
}

int fn_iStopSpuSampleEnv_inv(void *h)
{
    return fn_iStopTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h);
}

int fn_iSetParamSpuSampleEnv_inv(void *h, void *param)
{
    return fn_iSetParamTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, param);
}

int fn_iGetPosSpuSampleEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h, out);
}

int fn_iPauseSpuSampleEnv_inv(void *h)
{
    return fn_iPauseTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h);
}

int fn_iResumeSpuSampleEnv_inv(void *h)
{
    return fn_iResumeTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv, h);
}

int fn_iOperateCallTdstSpuSampleEnv_inv(void)
{
    return fn_vOperateCallTdstSpuSampleMngr_inv(g_stTdstSpuSampleMngr_inv);
}

static const Fn_inv a_pfnTdstSpuSampleInterface_inv[20] = {
    fn_iInitSpuSampleEnv_inv,
    fn_iDesInitSpuSampleEnv_inv,
    fn_iGetNumFreeVoiceSpuSampleEnv_inv,
    fn_bTestFormatSpuSampleEnv_inv,
    fn_u32AllocateVoiceSpuSampleEnv_inv,
    fn_u32ReallocateVoiceSpuSampleEnv_inv,
    fn_u32DesallocateVoiceSpuSampleEnv_inv,
    fn_iSetOwnerSpuSampleEnv_inv,
    fn_iConnectSpuSampleEnv_inv,
    fn_iGetNumChildSpuSampleEnv_inv,
    fn_iGetChildSpuSampleEnv_inv,
    fn_bIsAllocatedSpuSampleEnv_inv,
    fn_iNotifySpuSampleEnv_inv,
    fn_iOperateCallTdstSpuSampleEnv_inv,
    fn_iStartSpuSampleEnv_inv,
    fn_iStopSpuSampleEnv_inv,
    fn_iSetParamSpuSampleEnv_inv,
    fn_iGetPosSpuSampleEnv_inv,
    fn_iPauseSpuSampleEnv_inv,
    fn_iResumeSpuSampleEnv_inv,
};

Fn_inv *fn_pstGetSpuSampleInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstSpuSampleInterface_inv;
}
