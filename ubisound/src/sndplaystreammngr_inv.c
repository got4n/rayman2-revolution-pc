#include "include_asm.h"
#include "sndplaystream_inv.h"

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstPlayStreamData_inv voices[24];
} tdstPlayStreamMngr_inv;

typedef int (*PlayStreamDataFn_inv)(tdstPlayStreamData_inv *v, void *arg);

typedef struct {
    int x;
    int *ok;
} OpenCtx_inv;

extern int fn_iConstructTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, int x);
extern void fn_vOperateCallTdstPlayStreamData_inv(tdstPlayStreamData_inv *v);
extern tdstPlayStreamData_inv *fn_u32AllocateTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, Desc_inv *desc);
extern int fn_u32DesallocateTdstPlayStreamData_inv();
extern int fn_iSetOwnerTdstPlayStreamData_inv();
extern int fn_iConnectTdstPlayStreamData_inv();
extern int fn_iGetNumChildTdstPlayStreamData_inv();
extern int fn_iGetChildTdstPlayStreamData_inv();
extern int fn_iNotifyTdstPlayStreamData_inv();
extern int fn_iStartTdstPlayStreamData_inv();
extern int fn_iStopTdstPlayStreamData_inv();
extern int fn_pSetParamTdstPlayStreamData_inv();
extern int fn_pPushNextBlockTdstPlayStreamData_inv();
extern int fn_iSetCallbackArgTdstPlayStreamData_inv();
extern int fn_iGetPosTdstPlayStreamData_inv();
extern int fn_iPauseTdstPlayStreamData_inv();
extern int fn_iResumeTdstPlayStreamData_inv();

static inline tdstPlayStreamData_inv *ValidateVoice_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    tdstPlayStreamData_inv *found = 0;

    if (h >= pool->voices && h < &pool->voices[24]) {
        if (((char *)h - (char *)pool->voices) % sizeof(tdstPlayStreamData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstPlayStreamData_inv *v)
{
    return !v->f.bAllocated;
}

tdstPlayStreamData_inv *fn_pstParseTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, void *arg, PlayStreamDataFn_inv fn)
{
    tdstPlayStreamData_inv *r = 0;
    tdstPlayStreamData_inv *v;

    for (v = pool->voices; v < &pool->voices[24]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iConstructOneTdstPlayStreamMngr_inv(tdstPlayStreamData_inv *v, void *arg)
{
    OpenCtx_inv *c = arg;
    int *ok = c->ok;

    *ok = fn_iConstructTdstPlayStreamData_inv(v, c->x) >= 0 && *ok;
    return 0;
}

int fn_iConstructTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, int x)
{
    int ok;
    OpenCtx_inv c = { x, &ok };
    int r = -1;

    ok = 1;
    fn_pstParseTdstPlayStreamMngr_inv(pool, &c, fn_iConstructOneTdstPlayStreamMngr_inv);
    if (ok) {
        r = 0;
        pool->f.bConstructed = 1;
    }
    return r;
}

int fn_iDestructOneTdstPlayStreamMngr_inv(tdstPlayStreamData_inv *v)
{
    fn_iConstructTdstPlayStreamData_inv(v, 0);
    return 0;
}

int fn_iDestructTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool)
{
    if (pool->f.bConstructed) {
        fn_pstParseTdstPlayStreamMngr_inv(pool, 0, (PlayStreamDataFn_inv)fn_iDestructOneTdstPlayStreamMngr_inv);
        pool->f.bConstructed = 0;
    }
    return 0;
}

int fn_iCountFreeTdstPlayStreamMngr_inv(tdstPlayStreamData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iGetNumFreeVoiceTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstPlayStreamMngr_inv(pool, &n, (PlayStreamDataFn_inv)fn_iCountFreeTdstPlayStreamMngr_inv);
    return n;
}

int fn_iIsFreeTdstPlayStreamMngr_inv(tdstPlayStreamData_inv *v)
{
    return IsFree_inv(v);
}

tdstPlayStreamData_inv *fn_u32AllocateVoiceTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, Desc_inv *desc)
{
    tdstPlayStreamData_inv *r = 0;
    tdstPlayStreamData_inv *v = fn_pstParseTdstPlayStreamMngr_inv(pool, 0, (PlayStreamDataFn_inv)fn_iIsFreeTdstPlayStreamMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstPlayStreamData_inv(v, desc);
    return r;
}

int fn_u32ReallocateVoiceTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_u32DesallocateVoiceTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstPlayStreamData_inv(v);
    return r;
}

int fn_iSetOwnerTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_iConnectTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_iGetNumChildTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetNumChildTdstPlayStreamData_inv(v);
    return r;
}

int fn_iGetChildTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0, int a1)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetChildTdstPlayStreamData_inv(v, a0, a1);
    return r;
}

int fn_bIsAllocatedTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = 0;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iNotifyTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstPlayStreamData_inv(v, a0, a1, a2);
    return r;
}

int fn_iStartTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_iStopTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstPlayStreamData_inv(v);
    return r;
}

int fn_iSetParamTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_pSetParamTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_iPushNextBlockTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0, int a1)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_pPushNextBlockTdstPlayStreamData_inv(v, a0, a1);
    return r;
}

int fn_iSetCallbackArgTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetCallbackArgTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_iGetPosTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h, int a0)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstPlayStreamData_inv(v, a0);
    return r;
}

int fn_iPauseTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iPauseTdstPlayStreamData_inv(v);
    return r;
}

int fn_iResumeTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    int r;
    tdstPlayStreamData_inv *v = ValidateVoice_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iResumeTdstPlayStreamData_inv(v);
    return r;
}

int fn_iOperateCallOneTdstPlayStreamMngr_inv(tdstPlayStreamData_inv *v)
{
    fn_vOperateCallTdstPlayStreamData_inv(v);
    return 0;
}

void fn_vOperateCallTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool)
{
    fn_pstParseTdstPlayStreamMngr_inv(pool, 0, (PlayStreamDataFn_inv)fn_iOperateCallOneTdstPlayStreamMngr_inv);
}

tdstPlayStreamData_inv *fn_pstValidateTdstPlayStreamMngr_inv(tdstPlayStreamMngr_inv *pool, tdstPlayStreamData_inv *h)
{
    return ValidateVoice_inv(pool, h);
}
