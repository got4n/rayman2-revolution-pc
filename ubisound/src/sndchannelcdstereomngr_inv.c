#include "include_asm.h"
#include "sndchannelcdstereo_inv.h"

extern int fn_u32DesallocateTdstChannelCDStereoData_inv();

extern int fn_iStartTdstChannelCDStereoData_inv();

extern int fn_iStopTdstChannelCDStereoData_inv();

extern int fn_u32ReadTdstChannelCDStereoData_inv();

extern int fn_iSetOwnerTdstChannelCDStereoData_inv();

extern int fn_iConnectTdstChannelCDStereoData_inv();

extern int fn_u32AllocateTdstChannelCDStereoData_inv();

extern int fn_iGetNumChildTdstChannelCDStereoData_inv();

extern int fn_iGetChildTdstChannelCDStereoData_inv();

extern int fn_iNotifyTdstChannelCDStereoData_inv();

extern int fn_iGetPosTdstChannelCDStereoData_inv();

extern int fn_iDestructTdstChannelCDStereoData_inv();

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstChannelCDStereoData_inv items[2];
} tdstChannelCDStereoMngr_inv;

typedef int (*ChannelCDStereoDataFn_inv)(tdstChannelCDStereoData_inv *v, void *arg);

typedef struct {
    int x;
    int n;
    void *buf;
} OpenCfg_inv;

extern char D_00020130[];
extern int fn_iConstructTdstChannelCDStereoData_inv();

static inline tdstChannelCDStereoData_inv *ValidateElemD_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h)
{
    tdstChannelCDStereoData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[2]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstChannelCDStereoData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstChannelCDStereoData_inv *v)
{
    return !v->f.bAllocated;
}

tdstChannelCDStereoData_inv *fn_pstParseTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, void *arg, ChannelCDStereoDataFn_inv fn)
{
    tdstChannelCDStereoData_inv *r = 0;
    tdstChannelCDStereoData_inv *v;

    for (v = pool->items; v < &pool->items[2]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iCountFreeTdstChannelCDStereoMngr_inv(tdstChannelCDStereoData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iIsFreeTdstChannelCDStereoMngr_inv(tdstChannelCDStereoData_inv *v)
{
    return IsFree_inv(v);
}

int fn_iConstructOneTdstChannelCDStereoMngr_inv(tdstChannelCDStereoData_inv *e, OpenCfg_inv *c)
{
    fn_iConstructTdstChannelCDStereoData_inv(e, c->x, &c->n);
    return 0;
}

int fn_iDestructOneTdstChannelCDStereoMngr_inv(tdstChannelCDStereoData_inv *e, int x)
{
    fn_iDestructTdstChannelCDStereoData_inv(e, x);
    return 0;
}

int fn_iConstructTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, int x)
{
    OpenCfg_inv c;
    int r = -1;

    if (!pool->f.bConstructed) {
        c.x = x;
        c.n = 1024;
        c.buf = D_00020130;
        fn_pstParseTdstChannelCDStereoMngr_inv(pool, &c, (ChannelCDStereoDataFn_inv)fn_iConstructOneTdstChannelCDStereoMngr_inv);
        r = 0;
        pool->f.bConstructed = 1;
    }
    return r;
}

int fn_iDestructTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool)
{
    int r;

    if (pool->f.bConstructed) {
        fn_pstParseTdstChannelCDStereoMngr_inv(pool, 0, (ChannelCDStereoDataFn_inv)fn_iDestructOneTdstChannelCDStereoMngr_inv);
        r = 0;
        pool->f.bConstructed = 0;
    } else {
        r = 0;
    }
    return r;
}

int fn_iGetNumFreeVoiceTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstChannelCDStereoMngr_inv(pool, &n, (ChannelCDStereoDataFn_inv)fn_iCountFreeTdstChannelCDStereoMngr_inv);
    return n;
}

tdstChannelCDStereoData_inv *fn_u32AllocateVoiceTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, Desc_inv *desc)
{
    tdstChannelCDStereoData_inv *r = 0;
    tdstChannelCDStereoData_inv *v = fn_pstParseTdstChannelCDStereoMngr_inv(pool, 0, (ChannelCDStereoDataFn_inv)fn_iIsFreeTdstChannelCDStereoMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstChannelCDStereoData_inv(v, desc);
    return r;
}

int fn_u32DesallocateVoiceTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstChannelCDStereoData_inv(v);
    return r;
}

int fn_bIsAllocatedTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iStartTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstChannelCDStereoData_inv(v, a0);
    return r;
}

int fn_iStopTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstChannelCDStereoData_inv(v);
    return r;
}

int fn_iReadTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_u32ReadTdstChannelCDStereoData_inv(v, a0, a1, a2);
    return r;
}

int fn_iSetOwnerTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstChannelCDStereoData_inv(v, a0);
    return r;
}

int fn_iConnectTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstChannelCDStereoData_inv(v, a0);
    return r;
}

int fn_u32ReallocateVoiceTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstChannelCDStereoData_inv(v, a0);
    return r;
}

int fn_iGetNumChildTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetNumChildTdstChannelCDStereoData_inv(v);
    return r;
}

int fn_iGetChildTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0, int a1)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetChildTdstChannelCDStereoData_inv(v, a0, a1);
    return r;
}

int fn_iNotifyTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstChannelCDStereoData_inv(v, a0, a1, a2);
    return r;
}

int fn_iGetPosTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h, int a0)
{
    int r;
    tdstChannelCDStereoData_inv *v = ValidateElemD_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstChannelCDStereoData_inv(v, a0);
    return r;
}

int fn_iOperateCallOneTdstChannelCDStereoMngr_inv(tdstChannelCDStereoData_inv *v)
{
    fn_vOperateCallTdstChannelCDStereoData_inv(v);
    return 0;
}

void fn_vOperateCallTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool)
{
    fn_pstParseTdstChannelCDStereoMngr_inv(pool, 0, (ChannelCDStereoDataFn_inv)fn_iOperateCallOneTdstChannelCDStereoMngr_inv);
}

tdstChannelCDStereoData_inv *fn_pstValidateTdstChannelCDStereoMngr_inv(tdstChannelCDStereoMngr_inv *pool, tdstChannelCDStereoData_inv *h)
{
    return ValidateElemD_inv(pool, h);
}
