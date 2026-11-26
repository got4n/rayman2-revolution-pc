#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_u32AllocateVoiceTdstSpuStreamStereoMngr_inv();

extern int fn_iConstructTdstSpuStreamStereoMngr_inv();

extern char g_stTdstSpuStreamStereoMngr_inv[];
extern int fn_iDestructTdstSpuStreamStereoMngr_inv();
extern int fn_iGetNumFreeVoiceTdstSpuStreamStereoMngr_inv();
extern int fn_u32DesallocateVoiceTdstSpuStreamStereoMngr_inv();
extern int fn_bIsAllocatedTdstSpuStreamStereoMngr_inv();
extern int fn_iSetOwnerTdstSpuStreamStereoMngr_inv();
extern int fn_iConnectTdstSpuStreamStereoMngr_inv();
extern int fn_u32ReallocateVoiceTdstSpuStreamStereoMngr_inv();
extern int fn_iGetNumChildTdstSpuStreamStereoMngr_inv();
extern int fn_iGetChildTdstSpuStreamStereoMngr_inv();
extern int fn_iStartTdstSpuStreamStereoMngr_inv();
extern int fn_iStopTdstSpuStreamStereoMngr_inv();
extern int fn_iSetParamTdstSpuStreamStereoMngr_inv();
extern int fn_iGetPosTdstSpuStreamStereoMngr_inv();
extern int fn_iNotifyTdstSpuStreamStereoMngr_inv();
extern int fn_iPauseTdstSpuStreamStereoMngr_inv();
extern int fn_iResumeTdstSpuStreamStereoMngr_inv();
extern int fn_vOperateCallTdstSpuStreamStereoMngr_inv();
Fn_inv *fn_pstGetSpuStreamStereoInterface_inv(void);

int fn_iInitSpuStreamStereoEnv_inv(void)
{
    return fn_iConstructTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, fn_pstGetSpuStreamStereoInterface_inv());
}

int fn_iDesInitSpuStreamStereoEnv_inv(void)
{
    return fn_iDestructTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv);
}

int fn_iGetNumFreeVoiceSpuStreamStereoEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv);
}

int fn_bTestFormatSpuStreamStereoEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream < 0 && d->lFormat == 4 && d->cNbChannels == 2 && mode == 2;
}

int fn_u32AllocateVoiceSpuStreamStereoEnv_inv(DescHead_inv *d)
{
    int r = 0;
    int n = d->cNbChannels;

    if (n < fn_iGetNumFreeVoiceTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv))
        r = fn_u32AllocateVoiceTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, d);
    return r;
}

int fn_u32DesallocateVoiceSpuStreamStereoEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h);
}

int fn_bIsAllocatedSpuStreamStereoEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h);
}

int fn_iSetOwnerSpuStreamStereoEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, owner);
}

int fn_iConnectSpuStreamStereoEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, arg);
}

int fn_u32ReallocateVoiceSpuStreamStereoEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, desc);
}

int fn_iGetNumChildSpuStreamStereoEnv_inv(void *h)
{
    return fn_iGetNumChildTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h);
}

int fn_iGetChildSpuStreamStereoEnv_inv(void *h, Info_inv *out, int idx)
{
    return fn_iGetChildTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, out, idx);
}

int fn_iGetPosSpuStreamStereoEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, out);
}

int fn_iNotifySpuStreamStereoEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, event, from, arg);
}

int fn_iStartSpuStreamStereoEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, arg);
}

int fn_iStopSpuStreamStereoEnv_inv(void *h)
{
    return fn_iStopTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h);
}

int fn_iSetParamSpuStreamStereoEnv_inv(void *h, void *param)
{
    return fn_iSetParamTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h, param);
}

int fn_iPauseSpuStreamStereoEnv_inv(void *h)
{
    return fn_iPauseTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h);
}

int fn_iResumeSpuStreamStereoEnv_inv(void *h)
{
    return fn_iResumeTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv, h);
}

int fn_iOperateCallTdstSpuStreamStereoEnv_inv(void)
{
    return fn_vOperateCallTdstSpuStreamStereoMngr_inv(g_stTdstSpuStreamStereoMngr_inv);
}

static const Fn_inv a_pfnTdstSpuStreamStereoInterface_inv[20] = {
    fn_iInitSpuStreamStereoEnv_inv,
    fn_iDesInitSpuStreamStereoEnv_inv,
    fn_iGetNumFreeVoiceSpuStreamStereoEnv_inv,
    fn_bTestFormatSpuStreamStereoEnv_inv,
    fn_u32AllocateVoiceSpuStreamStereoEnv_inv,
    fn_u32ReallocateVoiceSpuStreamStereoEnv_inv,
    fn_u32DesallocateVoiceSpuStreamStereoEnv_inv,
    fn_iSetOwnerSpuStreamStereoEnv_inv,
    fn_iConnectSpuStreamStereoEnv_inv,
    fn_iGetNumChildSpuStreamStereoEnv_inv,
    fn_iGetChildSpuStreamStereoEnv_inv,
    fn_bIsAllocatedSpuStreamStereoEnv_inv,
    fn_iNotifySpuStreamStereoEnv_inv,
    fn_iOperateCallTdstSpuStreamStereoEnv_inv,
    fn_iStartSpuStreamStereoEnv_inv,
    fn_iStopSpuStreamStereoEnv_inv,
    fn_iSetParamSpuStreamStereoEnv_inv,
    fn_iGetPosSpuStreamStereoEnv_inv,
    fn_iPauseSpuStreamStereoEnv_inv,
    fn_iResumeSpuStreamStereoEnv_inv,
};

Fn_inv *fn_pstGetSpuStreamStereoInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstSpuStreamStereoInterface_inv;
}
