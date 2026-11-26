#include "include_asm.h"
#include "sndspustreamstereo_inv.h"

extern int fn_u32DesallocateTdstSpuStreamStereoData_inv();

extern int fn_iSetOwnerTdstSpuStreamStereoData_inv();

extern int fn_iConnectTdstSpuStreamStereoData_inv();

extern int fn_u32AllocateTdstSpuStreamStereoData_inv();

extern int fn_iGetNumChildTdstSpuStreamStereoData_inv();

extern int fn_iGetChildTdstSpuStreamStereoData_inv();

extern int fn_iStartTdstSpuStreamStereoData_inv();

extern int fn_iStopTdstSpuStreamStereoData_inv();

extern int fn_pSetParamTdstSpuStreamStereoData_inv();

extern int fn_iGetPosTdstSpuStreamStereoData_inv();

extern int fn_iNotifyTdstSpuStreamStereoData_inv();

extern int fn_iPauseTdstSpuStreamStereoData_inv();

extern int fn_iResumeTdstSpuStreamStereoData_inv();

extern int fn_iConstructTdstSpuStreamStereoData_inv();
extern int fn_iDestructTdstSpuStreamStereoData_inv();

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstSpuStreamStereoData_inv items[12];
} tdstSpuStreamStereoMngr_inv;

typedef int (*SpuStreamStereoDataFn_inv)(tdstSpuStreamStereoData_inv *v, void *arg);

static inline tdstSpuStreamStereoData_inv *ValidateElemA_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    tdstSpuStreamStereoData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[12]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstSpuStreamStereoData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstSpuStreamStereoData_inv *v)
{
    return !v->f.bAllocated;
}

tdstSpuStreamStereoData_inv *fn_pstParseTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, void *arg, SpuStreamStereoDataFn_inv fn)
{
    tdstSpuStreamStereoData_inv *r = 0;
    tdstSpuStreamStereoData_inv *v;

    for (v = pool->items; v < &pool->items[12]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iCountFreeTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iIsFreeTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoData_inv *v)
{
    return IsFree_inv(v);
}

int fn_iConstructOneTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoData_inv *e, int x)
{
    fn_iConstructTdstSpuStreamStereoData_inv(e, x);
    return 0;
}

int fn_iDestructOneTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoData_inv *e)
{
    fn_iDestructTdstSpuStreamStereoData_inv(e);
    return 0;
}

int fn_iConstructTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, int x)
{
    int r = -1;

    if (!pool->f.bConstructed) {
        fn_pstParseTdstSpuStreamStereoMngr_inv(pool, (void *)x, (SpuStreamStereoDataFn_inv)fn_iConstructOneTdstSpuStreamStereoMngr_inv);
        r = 0;
        pool->f.bConstructed = 1;
    }
    return r;
}

int fn_iDestructTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool)
{
    int r;

    if (pool->f.bConstructed) {
        fn_pstParseTdstSpuStreamStereoMngr_inv(pool, 0, (SpuStreamStereoDataFn_inv)fn_iDestructOneTdstSpuStreamStereoMngr_inv);
        r = 0;
        pool->f.bConstructed = 0;
    } else {
        r = 0;
    }
    return r;
}

int fn_iGetNumFreeVoiceTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstSpuStreamStereoMngr_inv(pool, &n, (SpuStreamStereoDataFn_inv)fn_iCountFreeTdstSpuStreamStereoMngr_inv);
    return n;
}

tdstSpuStreamStereoData_inv *fn_u32AllocateVoiceTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, Desc_inv *desc)
{
    tdstSpuStreamStereoData_inv *r = 0;
    tdstSpuStreamStereoData_inv *v = fn_pstParseTdstSpuStreamStereoMngr_inv(pool, 0, (SpuStreamStereoDataFn_inv)fn_iIsFreeTdstSpuStreamStereoMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstSpuStreamStereoData_inv(v, desc);
    return r;
}

int fn_u32DesallocateVoiceTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstSpuStreamStereoData_inv(v);
    return r;
}

int fn_bIsAllocatedTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iSetOwnerTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstSpuStreamStereoData_inv(v, a0);
    return r;
}

int fn_iConnectTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstSpuStreamStereoData_inv(v, a0);
    return r;
}

int fn_u32ReallocateVoiceTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstSpuStreamStereoData_inv(v, a0);
    return r;
}

int fn_iGetNumChildTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetNumChildTdstSpuStreamStereoData_inv(v);
    return r;
}

int fn_iGetChildTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0, int a1)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetChildTdstSpuStreamStereoData_inv(v, a0, a1);
    return r;
}

int fn_iStartTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstSpuStreamStereoData_inv(v, a0);
    return r;
}

int fn_iStopTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstSpuStreamStereoData_inv(v);
    return r;
}

int fn_iSetParamTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_pSetParamTdstSpuStreamStereoData_inv(v, a0);
    return r;
}

int fn_iGetPosTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstSpuStreamStereoData_inv(v, a0);
    return r;
}

int fn_iNotifyTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstSpuStreamStereoData_inv(v, a0, a1, a2);
    return r;
}

int fn_iPauseTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iPauseTdstSpuStreamStereoData_inv(v);
    return r;
}

int fn_iResumeTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    int r;
    tdstSpuStreamStereoData_inv *v = ValidateElemA_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iResumeTdstSpuStreamStereoData_inv(v);
    return r;
}

int fn_iOperateCallOneTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoData_inv *v)
{
    fn_vOperateCallTdstSpuStreamStereoData_inv(v);
    return 0;
}

void fn_vOperateCallTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool)
{
    fn_pstParseTdstSpuStreamStereoMngr_inv(pool, 0, (SpuStreamStereoDataFn_inv)fn_iOperateCallOneTdstSpuStreamStereoMngr_inv);
}

tdstSpuStreamStereoData_inv *fn_pstValidateTdstSpuStreamStereoMngr_inv(tdstSpuStreamStereoMngr_inv *pool, tdstSpuStreamStereoData_inv *h)
{
    return ValidateElemA_inv(pool, h);
}
