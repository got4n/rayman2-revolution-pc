#include "include_asm.h"
#include "sndspusample_inv.h"

extern int fn_u32AllocateTdstSpuSampleData_inv();

extern int fn_u32DesallocateTdstSpuSampleData_inv();

extern int fn_iSetOwnerTdstSpuSampleData_inv();

extern int fn_iConnectTdstSpuSampleData_inv();

extern int fn_iNotifyTdstSpuSampleData_inv();

extern int fn_iStartTdstSpuSampleData_inv();

extern int fn_iStopTdstSpuSampleData_inv();

extern int fn_pSetParamTdstSpuSampleData_inv();

extern int fn_iGetPosTdstSpuSampleData_inv();

extern int fn_iPauseTdstSpuSampleData_inv();

extern int fn_iResumeTdstSpuSampleData_inv();

extern int fn_iConstructTdstSpuSampleData_inv();
extern int fn_iDestructTdstSpuSampleData_inv();
extern int libsd_5_inv();

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstSpuSampleData_inv items[24];
} tdstSpuSampleMngr_inv;

typedef int (*SpuSampleDataFn_inv)(tdstSpuSampleData_inv *v, void *arg);

typedef struct {
    int idx;
    int ok;
} IdxCtx_inv;

static inline tdstSpuSampleData_inv *ValidateElemC_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    tdstSpuSampleData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[24]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstSpuSampleData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstSpuSampleData_inv *v)
{
    return !v->f.bAllocated;
}

tdstSpuSampleData_inv *fn_pstParseTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, void *arg, SpuSampleDataFn_inv fn)
{
    tdstSpuSampleData_inv *r = 0;
    tdstSpuSampleData_inv *v;

    for (v = pool->items; v < &pool->items[24]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iCountFreeTdstSpuSampleMngr_inv(tdstSpuSampleData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iIsFreeTdstSpuSampleMngr_inv(tdstSpuSampleData_inv *v)
{
    return IsFree_inv(v);
}

int fn_iConstructOneTdstSpuSampleMngr_inv(tdstSpuSampleData_inv *e, IdxCtx_inv *c)
{
    IdxCtx_inv *w = c;

    w->ok = fn_iConstructTdstSpuSampleData_inv(e, 1, c->idx) >= 0 && c->ok;
    w->idx++;
    return 0;
}

int fn_iDestructOneTdstSpuSampleMngr_inv(tdstSpuSampleData_inv *e)
{
    fn_iDestructTdstSpuSampleData_inv(e);
    return 0;
}

int fn_iConstructTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool)
{
    IdxCtx_inv c;
    int r = -1;

    if (!pool->f.bConstructed) {
        c.idx = 0;
        fn_pstParseTdstSpuSampleMngr_inv(pool, &c, (SpuSampleDataFn_inv)fn_iConstructOneTdstSpuSampleMngr_inv);
        if (c.ok) {
            pool->f.bConstructed = 1;
#ifdef PC_PORT
#else
            libsd_5_inv(0x981, 0x3FFF);
            libsd_5_inv(0xA81, 0x3FFF);
#endif
            pool->f.bConstructed = 1;
            r = 0;
        }
    } else {
        r = 0;
    }
    return r;
}

int fn_iDestructTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool)
{
    int r;

    if (pool->f.bConstructed) {
        fn_pstParseTdstSpuSampleMngr_inv(pool, 0, (SpuSampleDataFn_inv)fn_iDestructOneTdstSpuSampleMngr_inv);
        r = 0;
        pool->f.bConstructed = 0;
    } else {
        r = 0;
    }
    return r;
}

int fn_iGetNumFreeVoiceTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstSpuSampleMngr_inv(pool, &n, (SpuSampleDataFn_inv)fn_iCountFreeTdstSpuSampleMngr_inv);
    return n;
}

tdstSpuSampleData_inv *fn_u32AllocateVoiceTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, Desc_inv *desc)
{
    tdstSpuSampleData_inv *r = 0;
    tdstSpuSampleData_inv *v = fn_pstParseTdstSpuSampleMngr_inv(pool, 0, (SpuSampleDataFn_inv)fn_iIsFreeTdstSpuSampleMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstSpuSampleData_inv(v, desc);
    return r;
}

int fn_u32ReallocateVoiceTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstSpuSampleData_inv(v, a0);
    return r;
}

int fn_u32DesallocateVoiceTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstSpuSampleData_inv(v);
    return r;
}

int fn_iSetOwnerTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstSpuSampleData_inv(v, a0);
    return r;
}

int fn_iConnectTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstSpuSampleData_inv(v, a0);
    return r;
}

int fn_bIsAllocatedTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iNotifyTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstSpuSampleData_inv(v, a0, a1, a2);
    return r;
}

int fn_iStartTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstSpuSampleData_inv(v, a0);
    return r;
}

int fn_iStopTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstSpuSampleData_inv(v);
    return r;
}

int fn_iSetParamTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_pSetParamTdstSpuSampleData_inv(v, a0);
    return r;
}

int fn_iGetPosTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h, int a0)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstSpuSampleData_inv(v, a0);
    return r;
}

int fn_iPauseTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iPauseTdstSpuSampleData_inv(v);
    return r;
}

int fn_iResumeTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    int r;
    tdstSpuSampleData_inv *v = ValidateElemC_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iResumeTdstSpuSampleData_inv(v);
    return r;
}

int fn_iOperateCallOneTdstSpuSampleMngr_inv(tdstSpuSampleData_inv *v)
{
    fn_vOperateCallTdstSpuSampleData_inv(v);
    return 0;
}

void fn_vOperateCallTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool)
{
    fn_pstParseTdstSpuSampleMngr_inv(pool, 0, (SpuSampleDataFn_inv)fn_iOperateCallOneTdstSpuSampleMngr_inv);
}

tdstSpuSampleData_inv *fn_pstValidateTdstSpuSampleMngr_inv(tdstSpuSampleMngr_inv *pool, tdstSpuSampleData_inv *h)
{
    return ValidateElemC_inv(pool, h);
}
