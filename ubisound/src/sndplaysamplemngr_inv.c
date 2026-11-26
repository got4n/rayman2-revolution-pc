#include "include_asm.h"
#include "sndplaysample_inv.h"

extern int fn_iConstructTdstPlaySampleData_inv();
extern int fn_iDestructTdstPlaySampleData_inv();

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstPlaySampleData_inv items[24];
} tdstPlaySampleMngr_inv;

typedef int (*PlaySampleDataFn_inv)(tdstPlaySampleData_inv *v, void *arg);

typedef struct {
    int x;
    int *ok;
} OpenCtx_inv;

static inline tdstPlaySampleData_inv *ValidateObj_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    tdstPlaySampleData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[24]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstPlaySampleData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstPlaySampleData_inv *v)
{
    return !v->f.bAllocated;
}

tdstPlaySampleData_inv *fn_pstParseTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, void *arg, PlaySampleDataFn_inv fn)
{
    tdstPlaySampleData_inv *r = 0;
    tdstPlaySampleData_inv *v;

    for (v = pool->items; v < &pool->items[24]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iConstructOneTdstPlaySampleMngr_inv(tdstPlaySampleData_inv *v, void *arg)
{
    OpenCtx_inv *c = arg;
    int *ok = c->ok;

    *ok = fn_iConstructTdstPlaySampleData_inv(v, c->x) >= 0 && *ok;
    return 0;
}

int fn_iConstructTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, int x)
{
    int ok;
    OpenCtx_inv c = { x, &ok };
    int r = -1;

    ok = 1;
    fn_pstParseTdstPlaySampleMngr_inv(pool, &c, fn_iConstructOneTdstPlaySampleMngr_inv);
    if (ok) {
        r = 0;
        pool->f.bConstructed = 1;
    }
    return r;
}

int fn_iDestructOneTdstPlaySampleMngr_inv(tdstPlaySampleData_inv *v)
{
    fn_iDestructTdstPlaySampleData_inv(v);
    return 0;
}

int fn_iDestructTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool)
{
    if (pool->f.bConstructed) {
        fn_pstParseTdstPlaySampleMngr_inv(pool, 0, (PlaySampleDataFn_inv)fn_iDestructOneTdstPlaySampleMngr_inv);
        pool->f.bConstructed = 0;
    }
    return 0;
}

int fn_iCountFreeTdstPlaySampleMngr_inv(tdstPlaySampleData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iGetNumFreeVoiceTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstPlaySampleMngr_inv(pool, &n, (PlaySampleDataFn_inv)fn_iCountFreeTdstPlaySampleMngr_inv);
    return n;
}

int fn_iIsFreeTdstPlaySampleMngr_inv(tdstPlaySampleData_inv *v)
{
    return IsFree_inv(v);
}

tdstPlaySampleData_inv *fn_u32AllocateVoiceTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, Desc_inv *desc)
{
    tdstPlaySampleData_inv *r = 0;
    tdstPlaySampleData_inv *v = fn_pstParseTdstPlaySampleMngr_inv(pool, 0, (PlaySampleDataFn_inv)fn_iIsFreeTdstPlaySampleMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstPlaySampleData_inv(v, desc);
    return r;
}

int fn_u32ReallocateVoiceTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_u32DesallocateVoiceTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstPlaySampleData_inv(v);
    return r;
}

int fn_iSetOwnerTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_iConnectTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_iGetNumChildTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetNumChildTdstPlaySampleData_inv(v);
    return r;
}

int fn_iGetChildTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0, int a1)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetChildTdstPlaySampleData_inv(v, a0, a1);
    return r;
}

int fn_bIsAllocatedTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = 0;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iNotifyTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstPlaySampleData_inv(v, a0, a1, a2);
    return r;
}

int fn_iStartTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_iStopTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstPlaySampleData_inv(v);
    return r;
}

int fn_iSetParamTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_pSetParamTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_iPushNextBlockTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0, int a1)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_pPushNextBlockTdstPlaySampleData_inv(v, a0, a1);
    return r;
}

int fn_iSetCallbackArgTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetCallbackArgTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_iGetPosTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h, int a0)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstPlaySampleData_inv(v, a0);
    return r;
}

int fn_iPauseTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iPauseTdstPlaySampleData_inv(v);
    return r;
}

int fn_iResumeTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    int r;
    tdstPlaySampleData_inv *v = ValidateObj_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iResumeTdstPlaySampleData_inv(v);
    return r;
}

int fn_iOperateCallOneTdstPlaySampleMngr_inv(tdstPlaySampleData_inv *v)
{
    fn_vOperateCallTdstPlaySampleData_inv(v);
    return 0;
}

void fn_vOperateCallTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool)
{
    fn_pstParseTdstPlaySampleMngr_inv(pool, 0, (PlaySampleDataFn_inv)fn_iOperateCallOneTdstPlaySampleMngr_inv);
}

tdstPlaySampleData_inv *fn_pstValidateTdstPlaySampleMngr_inv(tdstPlaySampleMngr_inv *pool, tdstPlaySampleData_inv *h)
{
    return ValidateObj_inv(pool, h);
}
