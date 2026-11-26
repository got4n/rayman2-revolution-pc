#include "include_asm.h"
#include "sndchannelmem_inv.h"

extern int fn_u32DesallocateTdstChannelMEMData_inv();

extern int fn_iStartTdstChannelMEMData_inv();

extern int fn_iStopTdstChannelMEMData_inv();

extern int fn_iSetOwnerTdstChannelMEMData_inv();

extern int fn_iConnectTdstChannelMEMData_inv();

extern int fn_u32AllocateTdstChannelMEMData_inv();

extern int fn_iGetNumChildTdstChannelMEMData_inv();

extern int fn_iGetChildTdstChannelMEMData_inv();

extern int fn_iReadTdstChannelMEMData_inv();

extern int fn_iNotifyTdstChannelMEMData();

extern int fn_iGetPosTdstChannelMEMData_inv();

extern int fn_iConstructTdstChannelMEMData_inv();

extern int fn_iDestructTdstChannelMEMData_inv();

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstChannelMEMData_inv items[12];
} tdstChannelMEMMngr_inv;

typedef int (*ChannelMEMDataFn_inv)(tdstChannelMEMData_inv *v, void *arg);

static inline tdstChannelMEMData_inv *ValidateChan_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h)
{
    tdstChannelMEMData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[12]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstChannelMEMData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstChannelMEMData_inv *v)
{
    return !v->f.bAllocated;
}

tdstChannelMEMData_inv *fn_pstParseTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, void *arg, ChannelMEMDataFn_inv fn)
{
    tdstChannelMEMData_inv *r = 0;
    tdstChannelMEMData_inv *v;

    for (v = pool->items; v < &pool->items[12]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iCountFreeTdstChannelMEMMngr_inv(tdstChannelMEMData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iIsFreeTdstChannelMEMMngr_inv(tdstChannelMEMData_inv *v)
{
    return IsFree_inv(v);
}

int fn_iConstructOneTdstChannelMEMMngr_inv(tdstChannelMEMData_inv *c, int x)
{
    fn_iConstructTdstChannelMEMData_inv(c, x);
    return 0;
}

int fn_iDestructOneTdstChannelMEMMngr_inv(tdstChannelMEMData_inv *c)
{
    fn_iDestructTdstChannelMEMData_inv(c);
    return 0;
}

int fn_iConstructTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, int x)
{
    int r = -1;

    if (!pool->f.bConstructed) {
        fn_pstParseTdstChannelMEMMngr_inv(pool, (void *)x, (ChannelMEMDataFn_inv)fn_iConstructOneTdstChannelMEMMngr_inv);
        r = 0;
        pool->f.bConstructed = 1;
    }
    return r;
}

int fn_iDestructTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool)
{
    int r;

    if (pool->f.bConstructed) {
        fn_pstParseTdstChannelMEMMngr_inv(pool, 0, (ChannelMEMDataFn_inv)fn_iDestructOneTdstChannelMEMMngr_inv);
        r = 0;
        pool->f.bConstructed = 0;
    } else {
        r = 0;
    }
    return r;
}

int fn_iGetNumFreeVoiceTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstChannelMEMMngr_inv(pool, &n, (ChannelMEMDataFn_inv)fn_iCountFreeTdstChannelMEMMngr_inv);
    return n;
}

tdstChannelMEMData_inv *fn_u32AllocateVoiceTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, Desc_inv *desc)
{
    tdstChannelMEMData_inv *r = 0;
    tdstChannelMEMData_inv *v = fn_pstParseTdstChannelMEMMngr_inv(pool, 0, (ChannelMEMDataFn_inv)fn_iIsFreeTdstChannelMEMMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstChannelMEMData_inv(v, desc);
    return r;
}

int fn_u32DesallocateVoiceTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstChannelMEMData_inv(v);
    return r;
}

int fn_bIsAllocatedTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iStartTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstChannelMEMData_inv(v, a0);
    return r;
}

int fn_iStopTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstChannelMEMData_inv(v);
    return r;
}

int fn_iSetOwnerTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstChannelMEMData_inv(v, a0);
    return r;
}

int fn_iConnectTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstChannelMEMData_inv(v, a0);
    return r;
}

int fn_u32ReallocateVoiceTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstChannelMEMData_inv(v, a0);
    return r;
}

int fn_iGetNumChildTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetNumChildTdstChannelMEMData_inv(v);
    return r;
}

int fn_iGetChildTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0, int a1)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetChildTdstChannelMEMData_inv(v, a0, a1);
    return r;
}

int fn_iReadTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_iReadTdstChannelMEMData_inv(v, a0, a1, a2);
    return r;
}

int fn_iNotifyTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstChannelMEMData(v, a0, a1, a2);
    return r;
}

int fn_iGetPosTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h, int a0)
{
    int r;
    tdstChannelMEMData_inv *v = ValidateChan_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstChannelMEMData_inv(v, a0);
    return r;
}

int fn_iOperateCallOneTdstChannelMEMMngr_inv(tdstChannelMEMData_inv *v)
{
    func_00005BA0(v);
    return 0;
}

void fn_vOperateCallTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool)
{
    fn_pstParseTdstChannelMEMMngr_inv(pool, 0, (ChannelMEMDataFn_inv)fn_iOperateCallOneTdstChannelMEMMngr_inv);
}

tdstChannelMEMData_inv *fn_pstValidateTdstChannelMEMMngr_inv(tdstChannelMEMMngr_inv *pool, tdstChannelMEMData_inv *h)
{
    return ValidateChan_inv(pool, h);
}
