#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_u32AllocateVoiceTdstSpuStreamMngr_inv();

extern int fn_iConstructTdstSpuStreamMngr_inv();

extern char g_stTdstSpuStreamMngr_inv[];
extern int fn_iDestructTdstSpuStreamMngr_inv();
extern int fn_iGetNumFreeVoiceTdstSpuStreamMngr_inv();
extern int fn_u32DesallocateVoiceTdstSpuStreamMngr_inv();
extern int fn_bIsAllocatedTdstSpuStreamMngr_inv();
extern int fn_u32ReallocateVoiceTdstSpuStreamMngr_inv();
extern int fn_iSetOwnerTdstSpuStreamMngr_inv();
extern int fn_iConnectTdstSpuStreamMngr_inv();
extern int fn_iGetNumChildTdstSpuStreamMngr_inv();
extern int fn_iGetChildTdstSpuStreamMngr_inv();
extern int fn_iStartTdstSpuStreamMngr_inv();
extern int fn_iStopTdstSpuStreamMngr_inv();
extern int fn_iSetParamTdstSpuStreamMngr_inv();
extern int fn_iGetPosTdstSpuStreamMngr_inv();
extern int fn_iPauseTdstSpuStreamMngr_inv();
extern int fn_iResumeTdstSpuStreamMngr_inv();
extern int fn_vOperateCallTdstSpuStreamMngr_inv();
Fn_inv *fn_pstGetSpuStreamInterface_inv(void);

int fn_iInitSpuStreamEnv_inv(void)
{
    return fn_iConstructTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, fn_pstGetSpuStreamInterface_inv());
}

int fn_iDesInitSpuStreamEnv_inv(void)
{
    return fn_iDestructTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv);
}

int fn_iGetNumFreeVoiceSpuStreamEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, 0);
}

int fn_bTestFormatSpuStreamEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream < 0 && d->lFormat == 4 && d->cNbChannels == 1 && mode == 2;
}

int fn_u32AllocateVoiceSpuStreamEnv_inv(DescHead_inv *d)
{
    int r = 0;
    int n = d->cNbChannels;

    if (fn_iGetNumFreeVoiceTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, d) >= n)
        r = fn_u32AllocateVoiceTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, d);
    return r;
}

int fn_u32ReallocateVoiceSpuStreamEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, desc);
}

int fn_u32DesallocateVoiceSpuStreamEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h);
}

int fn_iSetOwnerSpuStreamEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, owner);
}

int fn_iConnectSpuStreamEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, arg);
}

int fn_iGetNumChildSpuStreamEnv_inv(void *h)
{
    return fn_iGetNumChildTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h);
}

int fn_iGetChildSpuStreamEnv_inv(void *h, Info_inv *out, int idx)
{
    return fn_iGetChildTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, out, idx);
}

int fn_bIsAllocatedSpuStreamEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h);
}

int fn_iStartSpuStreamEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, arg);
}

int fn_iStopSpuStreamEnv_inv(void *h)
{
    return fn_iStopTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h);
}

int fn_iSetParamSpuStreamEnv_inv(void *h, void *param)
{
    return fn_iSetParamTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, param);
}

int fn_iGetPosSpuStreamEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h, out);
}

int fn_iPauseSpuStreamEnv_inv(void *h)
{
    return fn_iPauseTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h);
}

int fn_iResumeSpuStreamEnv_inv(void *h)
{
    return fn_iResumeTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv, h);
}

int fn_iNotifySpuStreamEnv_inv(void)
{
    return 0;
}

int fn_iOperateCallTdstSpuStreamEnv_inv(void)
{
    return fn_vOperateCallTdstSpuStreamMngr_inv(g_stTdstSpuStreamMngr_inv);
}

static const Fn_inv a_pfnTdstSpuStreamInterface_inv[20] = {
    fn_iInitSpuStreamEnv_inv,
    fn_iDesInitSpuStreamEnv_inv,
    fn_iGetNumFreeVoiceSpuStreamEnv_inv,
    fn_bTestFormatSpuStreamEnv_inv,
    fn_u32AllocateVoiceSpuStreamEnv_inv,
    fn_u32ReallocateVoiceSpuStreamEnv_inv,
    fn_u32DesallocateVoiceSpuStreamEnv_inv,
    fn_iSetOwnerSpuStreamEnv_inv,
    fn_iConnectSpuStreamEnv_inv,
    fn_iGetNumChildSpuStreamEnv_inv,
    fn_iGetChildSpuStreamEnv_inv,
    fn_bIsAllocatedSpuStreamEnv_inv,
    fn_iNotifySpuStreamEnv_inv,
    fn_iOperateCallTdstSpuStreamEnv_inv,
    fn_iStartSpuStreamEnv_inv,
    fn_iStopSpuStreamEnv_inv,
    fn_iSetParamSpuStreamEnv_inv,
    fn_iGetPosSpuStreamEnv_inv,
    fn_iPauseSpuStreamEnv_inv,
    fn_iResumeSpuStreamEnv_inv,
};

Fn_inv *fn_pstGetSpuStreamInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstSpuStreamInterface_inv;
}
