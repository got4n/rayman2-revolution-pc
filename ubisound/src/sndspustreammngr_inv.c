#include "include_asm.h"
#include "sndspustream_inv.h"

extern int fn_iCountFreeTdstPlayStreamMngr_inv();
extern int fn_iIsFreeTdstPlayStreamMngr_inv();

extern int fn_u32DesallocateTdstSpuStreamData_inv();

extern int fn_u32AllocateTdstSpuStreamData_inv();

extern int fn_iSetOwnerTdstSpuStreamData_inv();

extern int fn_iConnectTdstSpuStreamData_inv();

extern int fn_iGetNumChildTdstSpuStreamData_inv();

extern int fn_iGetChildTdstSpuStreamData_inv();

extern int fn_iStartTdstSpuStreamData_inv();

extern int fn_iStopTdstSpuStreamData_inv();

extern int fn_pSetParamTdstSpuStreamData_inv();

extern int fn_iGetPosTdstSpuStreamData_inv();

extern int fn_iPauseTdstSpuStreamData_inv();

extern int fn_iResumeTdstSpuStreamData_inv();

extern int fn_bIsCompatibleTdstSpuStreamData_inv();

extern int fn_iSendFirstTdstSpuStreamData_inv();

extern int fn_iSendNextTdstSpuStreamData_inv();

extern int fn_vConstructTdstSpuStreamData_inv();

extern int fn_iSetupVoiceTdstSpuStreamData_inv();

extern void libsd_11_inv();

extern int thsemap_6_inv();

extern int thsemap_7_inv();

extern void libsd_7_inv();

extern int libsd_9_inv();

extern int libsd_27_inv();

extern int libsd_26_inv();

extern int thsemap_8_inv();

extern int thbase_5_inv();

extern int thsemap_5_inv();

extern int fn_iDestructTdstSpuStreamData_inv();

extern int fn_iConstructTdstSpuStreamData_inv();

extern int libsd_5_inv();

extern int thsemap_4_inv();

extern int thbase_4_inv();

extern int thbase_6_inv();

typedef struct {
    long bConstructed : 1;
    long bStopping : 1;
    long bRunning : 1;
} tdstSpuStreamMngrFlags_inv;

typedef struct {
    Buf_inv buf0;
    Buf_inv buf1;
    tdstSpuStreamMngrFlags_inv f;
    int sema;
    int x18;
    int x1c;
    int state;
    int val[7];
    tdstSpuStreamData_inv *cur;
    tdstSpuStreamData_inv items[24];
} tdstSpuStreamMngr_inv;

typedef struct {
    unsigned short op;
    unsigned short reg;
    int value;
} SdBatch_inv;
#ifdef PC_PORT
static unsigned char a_ucSpuStreamBuffer_inv[0x800] __attribute__((aligned(16)));
tdstSpuStreamMngr_inv g_stTdstSpuStreamMngr_inv = {
    { 0x800, a_ucSpuStreamBuffer_inv },
    { 0x400, a_ucSpuStreamBuffer_inv },
};

#define PC_VOICE_inv(st) ((st)->lVoice * 24 + (st)->lCore)
extern void fn_vPlayTdstSpuStreamData_inv(tdstSpuStreamData_inv *st);
extern void fn_vStopTdstSpuStreamData_inv(tdstSpuStreamData_inv *st);

#endif

typedef int (*SpuStreamDataFn_inv)(tdstSpuStreamData_inv *v, void *arg);

extern int libsd_15_inv(SdBatch_inv *b, int a, int n);

typedef int (*SpuStreamStateFn_inv)(tdstSpuStreamMngr_inv *pool);

typedef struct {
    int a;
    int b;
    int c;
    int d;
} SemaParam_inv;

typedef struct {
    int attr;
    int option;
    void *entry;
    int stack;
    int prio;
} ThreadParam_inv;

static inline SpuStreamStateFn_inv GetHandler_inv(tdstSpuStreamMngr_inv *pool, unsigned int i)
{
    SpuStreamStateFn_inv r = 0;

    if (i < 7)
        r = (SpuStreamStateFn_inv)pool->val[i];
    return r;
}

typedef struct {
    int n;
    DescHead_inv *d;
} CountCtx_inv;

int fn_iCountFreeTdstPlayStreamMngr_inv(tdstSpuStreamData_inv *v, int *n);
int fn_iIsFreeTdstPlayStreamMngr_inv(tdstSpuStreamData_inv *v);

static inline tdstSpuStreamData_inv *ValidateStream_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    tdstSpuStreamData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[24]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstSpuStreamData_inv) == 0)
            found = h;
    }
    return found;
}

tdstSpuStreamData_inv *fn_pstParseTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, void *arg, SpuStreamDataFn_inv fn)
{
    tdstSpuStreamData_inv *r = 0;
    tdstSpuStreamData_inv *v;

    for (v = pool->items; v < &pool->items[24]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iStateNoneTdstSpuStreamMngr_inv(void)
{
    return 0;
}

int fn_iStateFirstTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    fn_iSendFirstTdstSpuStreamData_inv(pool->cur, &pool->buf0);
#ifdef PC_PORT
    pc_task_signal(pool->x1c);
#endif
    pool->cur++;
    if (pool->cur == &pool->items[24]) {
        pool->cur = pool->items;
        pool->state = 2;
    }
    return 0;
}

#ifdef PC_PORT
int fn_iStateStartTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    pool->state = 3;
    pc_channel_notify(PC_VOICE_inv(&pool->items[0]), (const void *)(pool->items[0].lSpuAddr + 0x400), pool->x1c);
    for (st = pool->items; st < &pool->items[24]; st++)
        if (pool->val[6] & (1 << (st - pool->items)))
            fn_vPlayTdstSpuStreamData_inv(st);
    return 0;
}
#else
int fn_iStateStartTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    SdBatch_inv b[3] = {
        { 3, 0x1F00, pool->items[0].lSpuAddr + 0x400 },
        { 4, 4, 1 },
        { 2, 0x1500, pool->val[6] },
    };

    pool->state = 3;
    libsd_15_inv(b, 0, 3);
    return 0;
}
#endif

#ifdef PC_PORT
int fn_iStateNextTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    if (pool->cur == pool->items)
        pc_channel_notify(PC_VOICE_inv(&pool->items[0]), 0, 0);
    if (!fn_iSendNextTdstSpuStreamData_inv(pool->cur, &pool->buf1))
        pc_task_signal(pool->x1c);
    pool->cur++;
    if (pool->cur == &pool->items[24]) {
        pool->cur = pool->items;
        pool->state = 4;
    }
    return 0;
}
#else
int fn_iStateNextTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    if (pool->cur == pool->items)
        libsd_11_inv(4, 0);
    if (!fn_iSendNextTdstSpuStreamData_inv(pool->cur, &pool->buf1))
        thsemap_6_inv(pool->sema);
    pool->cur++;
    if (pool->cur == &pool->items[24]) {
        pool->cur = pool->items;
        pool->state = 4;
    }
    return 0;
}
#endif

#ifdef PC_PORT
int fn_iStateSwapTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;
    int at = pool->items[0].lSpuAddr + pool->items[0].lBufOffset;

    for (st = pool->items; st < &pool->items[24]; st++)
        fn_vConstructTdstSpuStreamData_inv(st);
    pc_channel_notify(PC_VOICE_inv(&pool->items[0]), (const void *)at, pool->x1c);
    if (pool->f.bStopping)
        pool->state = 5;
    else
        pool->state = 3;
    return 0;
}
#else
int fn_iStateSwapTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    libsd_9_inv(0x1F00, pool->items[0].lSpuAddr + pool->items[0].lBufOffset);
    for (st = pool->items; st < &pool->items[24]; st++)
        fn_vConstructTdstSpuStreamData_inv(st);
    libsd_11_inv(4, 1);
    if (pool->f.bStopping)
        pool->state = 5;
    else
        pool->state = 3;
    return 0;
}
#endif

#ifdef PC_PORT
int fn_iStateStopTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    pc_channel_notify(PC_VOICE_inv(&pool->items[0]), 0, 0);
    for (st = pool->items; st < &pool->items[24]; st++)
        if (pool->val[6] & (1 << (st - pool->items)))
            fn_vStopTdstSpuStreamData_inv(st);
    pool->state = 0;
    pool->f.bRunning = 0;
    return -1;
}
#else
int fn_iStateStopTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    libsd_11_inv(4, 0);
    libsd_7_inv(0x1600, pool->val[6]);
    pool->state = 0;
    pool->f.bRunning = 0;
    return -1;
}
#endif

#ifdef PC_PORT
int fn_iStateInitTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    for (st = pool->items; st < &pool->items[24]; st++)
        fn_iSetupVoiceTdstSpuStreamData_inv(st, 0x5622);
    pool->state = 1;
    pool->items[0].f.bAllocated = 1;
    pc_task_signal(pool->x1c);
    return 0;
}
#else
int fn_iStateInitTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    for (st = pool->items; st < &pool->items[24]; st++)
        fn_iSetupVoiceTdstSpuStreamData_inv(st, 0x5622);
    pool->state = 1;
    pool->items[0].f.bAllocated = 1;
    thsemap_6_inv(pool->sema);
    return 0;
}
#endif

#ifdef PC_PORT
int fn_iTransferCallbackTdstSpuStreamMngr_inv(int x, tdstSpuStreamMngr_inv *pool)
{
    if (x & 1)
        pc_task_signal(pool->x1c);
    return 1;
}
#else
int fn_iTransferCallbackTdstSpuStreamMngr_inv(int x, tdstSpuStreamMngr_inv *pool)
{
    if (x & 1)
        thsemap_7_inv(pool->sema);
    return 1;
}
#endif

#ifdef PC_PORT
int fn_iIrqCallbackTdstSpuStreamMngr_inv(int x, tdstSpuStreamMngr_inv *pool)
{
    if (!x)
        pc_task_signal(pool->x1c);
    return 1;
}
#else
int fn_iIrqCallbackTdstSpuStreamMngr_inv(int x, tdstSpuStreamMngr_inv *pool)
{
    if (!x)
        thsemap_7_inv(pool->sema);
    return 1;
}
#endif

#ifdef PC_PORT
int fn_iThreadTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    SpuStreamStateFn_inv fn;

    if (pool->f.bRunning) {
        fn = GetHandler_inv(pool, pool->state);
        if (fn != 0)
            fn(pool);
    }
    return 0;
}
#else
int fn_iThreadTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    int a;
    int b;
    SpuStreamStateFn_inv fn;

    a = libsd_27_inv(fn_iTransferCallbackTdstSpuStreamMngr_inv, pool);
    b = libsd_26_inv(0, fn_iIrqCallbackTdstSpuStreamMngr_inv, pool);
    fn_iStateInitTdstSpuStreamMngr_inv(pool);
    while (pool->f.bRunning) {
        thsemap_8_inv(pool->sema);
        fn = GetHandler_inv(pool, pool->state);
        if (fn != 0)
            fn(pool);
    }
    libsd_27_inv(a, 0);
    libsd_26_inv(0, b, 0);
    thsemap_6_inv(pool->x18);
    return 0;
}
#endif

#ifdef PC_PORT
int fn_iConstructTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, int x)
{
    tdstSpuStreamData_inv *st;
    int i;
    int bit;
    int r = 0;

    if (!pool->f.bConstructed) {
        i = 0;
        bit = 1;
        pool->val[0] = (int)fn_iStateNoneTdstSpuStreamMngr_inv;
        pool->val[1] = (int)fn_iStateFirstTdstSpuStreamMngr_inv;
        pool->val[2] = (int)fn_iStateStartTdstSpuStreamMngr_inv;
        pool->val[3] = (int)fn_iStateNextTdstSpuStreamMngr_inv;
        pool->val[4] = (int)fn_iStateSwapTdstSpuStreamMngr_inv;
        pool->val[5] = (int)fn_iStateStopTdstSpuStreamMngr_inv;
        pool->val[6] = 0;
        pool->cur = pool->items;
        for (st = pool->items; st < &pool->items[24]; st++) {
            fn_iConstructTdstSpuStreamData_inv(st, 1024, 0, i, x);
            i++;
            pool->val[6] |= bit;
            bit <<= 1;
        }
        pool->state = 0;
        pool->f.bStopping = 0;
        pool->f.bRunning = 1;
        pool->sema = 0;
        pool->x18 = 0;
        pool->x1c = pc_task_create((PcAddress)fn_iThreadTdstSpuStreamMngr_inv, (uint32_t)pool, 29);
        fn_iStateInitTdstSpuStreamMngr_inv(pool);
        pool->f.bConstructed = 1;
        r = 0;
    }
    return r;
}
#else
int fn_iConstructTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, int x)
{
    tdstSpuStreamData_inv *st;
    SemaParam_inv sp;
    ThreadParam_inv tp;
    int i;
    int bit;
    int tid;
    int r = 0;

    if (!pool->f.bConstructed) {
        i = 0;
        bit = 1;
        pool->val[0] = (int)fn_iStateNoneTdstSpuStreamMngr_inv;
        pool->val[1] = (int)fn_iStateFirstTdstSpuStreamMngr_inv;
        pool->val[2] = (int)fn_iStateStartTdstSpuStreamMngr_inv;
        pool->val[3] = (int)fn_iStateNextTdstSpuStreamMngr_inv;
        pool->val[4] = (int)fn_iStateSwapTdstSpuStreamMngr_inv;
        pool->val[5] = (int)fn_iStateStopTdstSpuStreamMngr_inv;
        pool->val[6] = 0;
        pool->cur = pool->items;
        for (st = pool->items; st < &pool->items[24]; st++) {
            fn_iConstructTdstSpuStreamData_inv(st, 1024, 0, i, x);
            i++;
            pool->val[6] |= bit;
            bit <<= 1;
        }
        libsd_5_inv(0x980, 0x3FFF);
        libsd_5_inv(0xA80, 0x3FFF);
        libsd_5_inv(0xB80, 0);
        libsd_5_inv(0xC80, 0);
        pool->state = 0;
        pool->f.bStopping = 0;
        pool->f.bRunning = 1;
        sp.a = 0;
        sp.b = 0;
        sp.c = 0;
        sp.d = 1;
        pool->x18 = thsemap_4_inv(&sp);
        pool->sema = thsemap_4_inv(&sp);
        tp.attr = 0x2000000;
        tp.entry = fn_iThreadTdstSpuStreamMngr_inv;
        tp.prio = 29;
        tp.stack = 2048;
        tp.option = 0;
        tid = thbase_4_inv(&tp);
        pool->x1c = tid;
        thbase_6_inv(tid, pool);
        pool->f.bConstructed = 1;
        r = 0;
    }
    return r;
}
#endif

#ifdef PC_PORT
int fn_iDestructTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    if (pool->f.bConstructed) {
        pool->f.bStopping = 1;
        if (pool->f.bRunning)
            fn_iStateStopTdstSpuStreamMngr_inv(pool);
        if (pool->x1c != 0) {
            pc_task_delete(pool->x1c);
            pool->x1c = 0;
        }
        pool->sema = 0;
        pool->x18 = 0;
        for (st = pool->items; st < &pool->items[24]; st++)
            fn_iDestructTdstSpuStreamData_inv(st);
        pool->f.bConstructed = 0;
    }
    return 0;
}
#else
int fn_iDestructTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    tdstSpuStreamData_inv *st;

    if (pool->f.bConstructed) {
        pool->f.bStopping = 1;
        thsemap_8_inv(pool->x18);
        if (pool->x1c != 0) {
            thbase_5_inv(pool->x1c);
            pool->x1c = 0;
        }
        if (pool->sema != 0) {
            thsemap_5_inv(pool->sema);
            pool->sema = 0;
        }
        if (pool->x18 != 0) {
            thsemap_5_inv(pool->x18);
            pool->x18 = 0;
        }
        for (st = pool->items; st < &pool->items[24]; st++)
            fn_iDestructTdstSpuStreamData_inv(st);
        pool->f.bConstructed = 0;
    }
    return 0;
}
#endif

int fn_iCountFreeTdstSpuStreamMngr_inv(tdstSpuStreamData_inv *st, CountCtx_inv *c)
{
    if (fn_bIsCompatibleTdstSpuStreamData_inv(st, c->d))
        c->n++;
    return 0;
}

int fn_iIsFreeTdstSpuStreamMngr_inv(tdstSpuStreamData_inv *st, DescHead_inv *d)
{
    return fn_bIsCompatibleTdstSpuStreamData_inv(st, d);
}

int fn_iGetNumFreeVoiceTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, DescHead_inv *d)
{
    CountCtx_inv c;

    c.d = d;
    c.n = 0;
    fn_pstParseTdstSpuStreamMngr_inv(pool, &c, (SpuStreamDataFn_inv)fn_iCountFreeTdstSpuStreamMngr_inv);
    return c.n;
}

tdstSpuStreamData_inv *fn_u32AllocateVoiceTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, DescHead_inv *d)
{
    tdstSpuStreamData_inv *r = 0;
    tdstSpuStreamData_inv *v = fn_pstParseTdstSpuStreamMngr_inv(pool, d, (SpuStreamDataFn_inv)fn_iIsFreeTdstSpuStreamMngr_inv);

    if (v != 0)
        r = (tdstSpuStreamData_inv *)fn_u32AllocateTdstSpuStreamData_inv(v, d);
    return r;
}

int fn_u32DesallocateVoiceTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstSpuStreamData_inv(v);
    return r;
}

int fn_bIsAllocatedTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = 0;
    if (v != 0)
        r = v->f.bAllocated;
    return r;
}

int fn_u32ReallocateVoiceTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstSpuStreamData_inv(v, a0);
    return r;
}

int fn_iSetOwnerTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstSpuStreamData_inv(v, a0);
    return r;
}

int fn_iConnectTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstSpuStreamData_inv(v, a0);
    return r;
}

int fn_iGetNumChildTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetNumChildTdstSpuStreamData_inv(v);
    return r;
}

int fn_iGetChildTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0, int a1)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetChildTdstSpuStreamData_inv(v, a0, a1);
    return r;
}

int fn_iStartTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstSpuStreamData_inv(v, a0);
    return r;
}

int fn_iStopTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstSpuStreamData_inv(v);
    return r;
}

int fn_iSetParamTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_pSetParamTdstSpuStreamData_inv(v, a0);
    return r;
}

int fn_iGetPosTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h, int a0)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstSpuStreamData_inv(v, a0);
    return r;
}

int fn_iPauseTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iPauseTdstSpuStreamData_inv(v);
    return r;
}

int fn_iResumeTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    int r;
    tdstSpuStreamData_inv *v = ValidateStream_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iResumeTdstSpuStreamData_inv(v);
    return r;
}

int fn_iOperateCallOneTdstSpuStreamMngr_inv(tdstSpuStreamData_inv *v)
{
    fn_vOperateCallTdstSpuStreamData_inv(v);
    return 0;
}

void fn_vOperateCallTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool)
{
    fn_pstParseTdstSpuStreamMngr_inv(pool, 0, (SpuStreamDataFn_inv)fn_iOperateCallOneTdstSpuStreamMngr_inv);
}

int fn_bIsValidValueTdstSpuStreamMngr_inv(unsigned int i)
{
    return i < 7;
}

void fn_vSetValueTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, int i, int v)
{
    pool->val[i] = v;
}

int fn_iGetValueTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, unsigned int i)
{
    int r = 0;

    if (i < 7)
        r = pool->val[i];
    return r;
}

tdstSpuStreamData_inv *fn_pstValidateTdstSpuStreamMngr_inv(tdstSpuStreamMngr_inv *pool, tdstSpuStreamData_inv *h)
{
    return ValidateStream_inv(pool, h);
}
