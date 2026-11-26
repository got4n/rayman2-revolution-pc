#include "include_asm.h"
#include "snditbase_inv.h"

typedef int (*Fn_inv)();

extern int fn_iConstructTdstPlayStreamMngr_inv();

extern char g_stTdstPlayStreamMngr_inv[];
extern int fn_iDestructTdstPlayStreamMngr_inv();
extern int fn_iGetNumFreeVoiceTdstPlayStreamMngr_inv();
extern int fn_u32AllocateVoiceTdstPlayStreamMngr_inv();
extern int fn_u32ReallocateVoiceTdstPlayStreamMngr_inv();
extern int fn_u32DesallocateVoiceTdstPlayStreamMngr_inv();
extern int fn_iSetOwnerTdstPlayStreamMngr_inv();
extern int fn_iConnectTdstPlayStreamMngr_inv();
extern int fn_iGetNumChildTdstPlayStreamMngr_inv();
extern int fn_iGetChildTdstPlayStreamMngr_inv();
extern int fn_bIsAllocatedTdstPlayStreamMngr_inv();
extern int fn_iNotifyTdstPlayStreamMngr_inv();
extern int fn_iStartTdstPlayStreamMngr_inv();
extern int fn_iStopTdstPlayStreamMngr_inv();
extern int fn_iSetParamTdstPlayStreamMngr_inv();
extern int fn_iPushNextBlockTdstPlayStreamMngr_inv();
extern int fn_iSetCallbackArgTdstPlayStreamMngr_inv();
extern int fn_iGetPosTdstPlayStreamMngr_inv();
extern int fn_iPauseTdstPlayStreamMngr_inv();
extern int fn_iResumeTdstPlayStreamMngr_inv();
extern int fn_vOperateCallTdstPlayStreamMngr_inv();
Fn_inv *fn_pstGetPlayStreamInterface_inv(void);

int fn_iInitPlayStreamEnv_inv(void)
{
    return fn_iConstructTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, fn_pstGetPlayStreamInterface_inv());
}

int fn_iDesInitPlayStreamEnv_inv(void)
{
    return fn_iDestructTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv);
}

int fn_iGetNumFreeVoicePlayStreamEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv);
}

int fn_bTestFormatPlayStreamEnv_inv(DescHead_inv *d, int mode)
{
    return d->f.bStream < 0 && mode == 4;
}

int fn_u32AllocateVoicePlayStreamEnv_inv(Desc_inv *desc)
{
    return fn_u32AllocateVoiceTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, desc);
}

int fn_u32ReallocateVoicePlayStreamEnv_inv(void *h, Desc_inv *desc)
{
    return fn_u32ReallocateVoiceTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, desc);
}

int fn_u32DesallocateVoicePlayStreamEnv_inv(void *h)
{
    return fn_u32DesallocateVoiceTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h);
}

int fn_iSetOwnerPlayStreamEnv_inv(void *h, Info_inv *owner)
{
    return fn_iSetOwnerTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, owner);
}

int fn_iConnectPlayStreamEnv_inv(void *h, void *arg)
{
    return fn_iConnectTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, arg);
}

int fn_iGetNumChildPlayStreamEnv_inv(void *h)
{
    return fn_iGetNumChildTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h);
}

int fn_iGetChildPlayStreamEnv_inv(void *h, Info_inv *out, int idx)
{
    return fn_iGetChildTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, out, idx);
}

int fn_bIsAllocatedPlayStreamEnv_inv(void *h)
{
    return fn_bIsAllocatedTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h);
}

int fn_iNotifyPlayStreamEnv_inv(void *h, int event, Info_inv *from, int arg)
{
    return fn_iNotifyTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, event, from, arg);
}

int fn_iStartPlayStreamEnv_inv(void *h, void *arg)
{
    return fn_iStartTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, arg);
}

int fn_iStopPlayStreamEnv_inv(void *h)
{
    return fn_iStopTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h);
}

int fn_iSetParamPlayStreamEnv_inv(void *h, void *param)
{
    return fn_iSetParamTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, param);
}

int fn_iPushNextBlockPlayStreamEnv_inv(void *h, int which, Quad_inv *block)
{
    return fn_iPushNextBlockTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, which, block);
}

int fn_iSetCallbackArgPlayStreamEnv_inv(void *h, int arg)
{
    return fn_iSetCallbackArgTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, arg);
}

int fn_iGetPosPlayStreamEnv_inv(void *h, int *out)
{
    return fn_iGetPosTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h, out);
}

int fn_iPausePlayStreamEnv_inv(void *h)
{
    return fn_iPauseTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h);
}

int fn_iResumePlayStreamEnv_inv(void *h)
{
    return fn_iResumeTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv, h);
}

int fn_iOperateCallTdstPlayStreamEnv_inv(void)
{
    return fn_vOperateCallTdstPlayStreamMngr_inv(g_stTdstPlayStreamMngr_inv);
}

static const Fn_inv a_pfnTdstPlayStreamInterface_inv[22] = {
    fn_iInitPlayStreamEnv_inv,
    fn_iDesInitPlayStreamEnv_inv,
    fn_iGetNumFreeVoicePlayStreamEnv_inv,
    fn_bTestFormatPlayStreamEnv_inv,
    fn_u32AllocateVoicePlayStreamEnv_inv,
    fn_u32ReallocateVoicePlayStreamEnv_inv,
    fn_u32DesallocateVoicePlayStreamEnv_inv,
    fn_iSetOwnerPlayStreamEnv_inv,
    fn_iConnectPlayStreamEnv_inv,
    fn_iGetNumChildPlayStreamEnv_inv,
    fn_iGetChildPlayStreamEnv_inv,
    fn_bIsAllocatedPlayStreamEnv_inv,
    fn_iNotifyPlayStreamEnv_inv,
    fn_iOperateCallTdstPlayStreamEnv_inv,
    fn_iStartPlayStreamEnv_inv,
    fn_iStopPlayStreamEnv_inv,
    fn_iSetParamPlayStreamEnv_inv,
    fn_iPushNextBlockPlayStreamEnv_inv,
    fn_iSetCallbackArgPlayStreamEnv_inv,
    fn_iGetPosPlayStreamEnv_inv,
    fn_iPausePlayStreamEnv_inv,
    fn_iResumePlayStreamEnv_inv,
};

Fn_inv *fn_pstGetPlayStreamInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstPlayStreamInterface_inv;
}
