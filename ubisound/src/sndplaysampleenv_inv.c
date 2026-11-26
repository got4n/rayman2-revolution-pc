#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_iConstructTdstPlaySampleMngr_inv();

extern char g_stTdstPlaySampleMngr_inv[];
extern int fn_iDestructTdstPlaySampleMngr_inv();
extern int fn_iGetNumFreeVoiceTdstPlaySampleMngr_inv();
extern int fn_u32AllocateVoiceTdstPlaySampleMngr_inv();
extern int fn_u32ReallocateVoiceTdstPlaySampleMngr_inv();
extern int fn_u32DesallocateVoiceTdstPlaySampleMngr_inv();
extern int fn_iSetOwnerTdstPlaySampleMngr_inv();
extern int fn_iConnectTdstPlaySampleMngr_inv();
extern int fn_iGetNumChildTdstPlaySampleMngr_inv();
extern int fn_iGetChildTdstPlaySampleMngr_inv();
extern int fn_bIsAllocatedTdstPlaySampleMngr_inv();
extern int fn_iNotifyTdstPlaySampleMngr_inv();
extern int fn_iStartTdstPlaySampleMngr_inv();
extern int fn_iStopTdstPlaySampleMngr_inv();
extern int fn_iSetParamTdstPlaySampleMngr_inv();
extern int fn_iPushNextBlockTdstPlaySampleMngr_inv();
extern int fn_iSetCallbackArgTdstPlaySampleMngr_inv();
extern int fn_iGetPosTdstPlaySampleMngr_inv();
extern int fn_iPauseTdstPlaySampleMngr_inv();
extern int fn_iResumeTdstPlaySampleMngr_inv();
extern int fn_vOperateCallTdstPlaySampleMngr_inv();
Fn_inv *fn_pstGetPlaySampleInterface_inv(void);

int fn_iInitPlaySampleEnv_inv(void)
{
    return fn_iConstructTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, fn_pstGetPlaySampleInterface_inv());
}

int fn_iDesInitPlaySampleEnv_inv(void)
{
    return fn_iDestructTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv);
}

int fn_iGetNumFreeVoicePlaySampleEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv);
}

int fn_bTestFormatPlaySampleEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream >= 0 && d->lFormat == 4 && mode == 4;
}

int fn_u32AllocateVoicePlaySampleEnv_inv(Desc_inv *desc)
{
    return fn_u32AllocateVoiceTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, desc);
}

int fn_u32ReallocateVoicePlaySampleEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, desc);
}

int fn_u32DesallocateVoicePlaySampleEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h);
}

int fn_iSetOwnerPlaySampleEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, owner);
}

int fn_iConnectPlaySampleEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, arg);
}

int fn_iGetNumChildPlaySampleEnv_inv(void *h)
{
    return fn_iGetNumChildTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h);
}

int fn_iGetChildPlaySampleEnv_inv(void *h, Info_inv *out, int idx)
{
    return fn_iGetChildTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, out, idx);
}

int fn_bIsAllocatedPlaySampleEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h);
}

int fn_iNotifyPlaySampleEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, event, from, arg);
}

int fn_iStartPlaySampleEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, arg);
}

int fn_iStopPlaySampleEnv_inv(void *h)
{
    return fn_iStopTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h);
}

int fn_iSetParamPlaySampleEnv_inv(void *h, void *param)
{
    return fn_iSetParamTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, param);
}

int fn_iPushNextBlockPlaySampleEnv_inv(void *h, int which, Quad_inv *block)
{
    return fn_iPushNextBlockTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, which, block);
}

int fn_iSetCallbackArgPlaySampleEnv_inv(void *h, int arg)
{
    return fn_iSetCallbackArgTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, arg);
}

int fn_iGetPosPlaySampleEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h, out);
}

int fn_iPausePlaySampleEnv_inv(void *h)
{
    return fn_iPauseTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h);
}

int fn_iResumePlaySampleEnv_inv(void *h)
{
    return fn_iResumeTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv, h);
}

int fn_iOperateCallTdstPlaySampleEnv_inv(void)
{
    return fn_vOperateCallTdstPlaySampleMngr_inv(g_stTdstPlaySampleMngr_inv);
}

static const Fn_inv a_pfnTdstPlaySampleInterface_inv[22] = {
    fn_iInitPlaySampleEnv_inv,
    fn_iDesInitPlaySampleEnv_inv,
    fn_iGetNumFreeVoicePlaySampleEnv_inv,
    fn_bTestFormatPlaySampleEnv_inv,
    fn_u32AllocateVoicePlaySampleEnv_inv,
    fn_u32ReallocateVoicePlaySampleEnv_inv,
    fn_u32DesallocateVoicePlaySampleEnv_inv,
    fn_iSetOwnerPlaySampleEnv_inv,
    fn_iConnectPlaySampleEnv_inv,
    fn_iGetNumChildPlaySampleEnv_inv,
    fn_iGetChildPlaySampleEnv_inv,
    fn_bIsAllocatedPlaySampleEnv_inv,
    fn_iNotifyPlaySampleEnv_inv,
    fn_iOperateCallTdstPlaySampleEnv_inv,
    fn_iStartPlaySampleEnv_inv,
    fn_iStopPlaySampleEnv_inv,
    fn_iSetParamPlaySampleEnv_inv,
    fn_iPushNextBlockPlaySampleEnv_inv,
    fn_iSetCallbackArgPlaySampleEnv_inv,
    fn_iGetPosPlaySampleEnv_inv,
    fn_iPausePlaySampleEnv_inv,
    fn_iResumePlaySampleEnv_inv,
};

Fn_inv *fn_pstGetPlaySampleInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstPlaySampleInterface_inv;
}
