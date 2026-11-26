#include "include_asm.h"

typedef struct {
    unsigned int size;
    char *data;
} Span_inv;

typedef struct {
    int x0;
    int x4;
    void *stackbuf;
    int stack;
} ThreadCfg_inv;

typedef struct {
    int attr;
    int option;
    void *entry;
    int stack;
    int prio;
} ThreadParam_inv;

typedef struct {
    ThreadCfg_inv *cfg;
    int caller;
    int result;
} StartArg_inv;

typedef struct {
    int x0;
    int x4;
} TidPair_inv;

typedef struct {
    unsigned int outsz;
    unsigned int insz;
    char data[0x3F8];
} SendBuf_inv;

typedef struct {
    int result;
    char data[0x3FC];
} RecvBuf_inv;

extern SendBuf_inv g_stRemoteRequest_inv;
extern RecvBuf_inv g_stRemoteReply_inv;
typedef struct {
    char x0[0x24];
    int server;
} RpcClient_inv;

typedef struct {
    int x[6];
} RpcQueue_inv;

typedef struct {
    int x[18];
} RpcServer_inv;

extern RpcClient_inv g_stRemoteClient_inv;
extern char a_ucServerBuffer_inv[];
int g_lNbServerCall_inv = 0;
int g_lNbServerActive_inv = 0;
char D_00013E58[0x1018] = { 0 };
extern int sifcmd_14_inv(int a);
extern int sifcmd_15_inv(RpcClient_inv *cd, int id, int mode);
extern int sifcmd_17_inv(RpcServer_inv *sd, int id, void *fn, void *buf, int a, int b, RpcQueue_inv *q);
extern int sifcmd_19_inv(RpcQueue_inv *q, int thread);
extern void *sifcmd_20_inv(RpcQueue_inv *q);
extern int sifcmd_21_inv(void *sd);
extern int thbase_14_inv(int thread, int prio);
extern int thbase_25_inv(int thread);
extern int sifcmd_16_inv(RpcClient_inv *cd, int id, int mode, void *send, unsigned int sendsz, void *recv, unsigned int recvsz, int a, int b);
extern int sifcmd_18_inv(RpcClient_inv *cd);
extern void *sysclib_14_inv(void *p, int c, int n);
extern char a_ucServerStack_inv[];
extern char a_ucClientStack_inv[];
extern void *sysclib_12_inv(void *dst, void *src, int n);
extern int thbase_4_inv();
extern int thbase_5_inv();
extern int thbase_6_inv();
extern int thbase_20_inv();
extern int thbase_22_inv();
extern int thbase_24_inv();

int fn_u32FormatCommandChunk(int a, Span_inv *dst, Span_inv *src, int b)
{
    int r = 0;

    if (dst->size >= src->size + 12) {
        int *h = (int *)dst->data;

        h[0] = a;
        h[1] = b;
        h[2] = src->size;
        if (src->size != 0)
#ifdef PC_PORT
            pc_memcpy(h + 3, src->data, src->size);
#else
            sysclib_12_inv(h + 3, src->data, src->size);
#endif
        r = src->size + 12;
        dst->size -= r;
        dst->data += r;
    }
    return r;
}

int fn_iRemoteCall(int id, int mode, void *out, unsigned int outsz, void *in, unsigned int insz)
{
    unsigned int inal;
    unsigned int outal;
    unsigned int sendsz;
    unsigned int recvsz;
    int r = -1;

    inal = (insz + 3) & ~3;
    outal = (outsz + 3) & ~3;
    sendsz = inal + 8;
    recvsz = outal + 4;
    if (sendsz <= 1024 && recvsz <= 1024) {
#ifdef PC_PORT
#else
        while (sifcmd_18_inv(&g_stRemoteClient_inv) != 0)
            ;
#endif
        g_stRemoteRequest_inv.outsz = outsz;
        g_stRemoteRequest_inv.insz = insz;
        if (insz != 0) {
#ifdef PC_PORT
            pc_memcpy(g_stRemoteRequest_inv.data, in, insz);
#else
            sysclib_12_inv(g_stRemoteRequest_inv.data, in, insz);
#endif
            if (insz != inal)
#ifdef PC_PORT
                pc_memset(g_stRemoteRequest_inv.data + insz, 0, inal - insz);
#else
                sysclib_14_inv(g_stRemoteRequest_inv.data + insz, 0, inal - insz);
#endif
        }
        if (outsz != 0)
#ifdef PC_PORT
            pc_memset(g_stRemoteReply_inv.data, 0, outal);
#else
            sysclib_14_inv(g_stRemoteReply_inv.data, 0, outal);
#endif
#ifdef PC_PORT
        (void)recvsz;
        if (pc_game_message(id, &g_stRemoteRequest_inv, sendsz)) {
#else
        if (sifcmd_16_inv(&g_stRemoteClient_inv, id, mode != 1, &g_stRemoteRequest_inv, sendsz, &g_stRemoteReply_inv, recvsz, 0, 0) >= 0) {
#endif
            r = g_stRemoteReply_inv.result;
            if (outsz != 0 && out != 0)
#ifdef PC_PORT
                pc_memcpy(out, g_stRemoteReply_inv.data, outsz);
#else
                sysclib_12_inv(out, g_stRemoteReply_inv.data, outsz);
#endif
        }
    }
    return r;
}

#ifndef PC_PORT
void fn_iServerLoop(StartArg_inv *a)
{
    RpcQueue_inv q;
    RpcServer_inv sd;
    ThreadCfg_inv *c = a->cfg;
    int caller = a->caller;
    int id = c->x0;
    void *req;

    sifcmd_14_inv(0);
    sifcmd_19_inv(&q, thbase_20_inv());
    sifcmd_17_inv(&sd, id, (void *)c->x4, a_ucServerBuffer_inv, 0, 0, &q);
    a->result = 0;
    thbase_14_inv(thbase_20_inv(), 105);
    thbase_25_inv(caller);
    thbase_14_inv(caller, 105);
    for (;;) {
        req = sifcmd_20_inv(&q);
        if (req != 0) {
            g_lNbServerActive_inv++;
            g_lNbServerCall_inv++;
            sifcmd_21_inv(req);
            g_lNbServerActive_inv--;
        } else {
            thbase_24_inv();
        }
    }
}

int fn_iClientLoop(StartArg_inv *a)
{
    volatile int i;
    int caller = a->caller;
    int id = a->cfg->x0;

    do {
        if (sifcmd_15_inv(&g_stRemoteClient_inv, id, 0) < 0)
            for (;;)
                ;
        i = 10000;
        while (--i != -1)
            ;
    } while (g_stRemoteClient_inv.server == 0);
    thbase_25_inv(caller);
    a->result = 0;
    return 0;
}

int fn_iGetCurrentThreadPriority(void)
{
    int info[18];

    thbase_22_inv(thbase_20_inv(), info);
    return info[8];
}

int fn_iCreateRemoteThread_inv(ThreadCfg_inv *c, void *entry)
{
    ThreadParam_inv tp;
    int r = -1;
    int tid;
    int prio = fn_iGetCurrentThreadPriority();

    tp.attr = 0x2000000;
    tp.option = 0;
    tp.entry = entry;
    tp.stack = c->stack;
    tp.prio = prio;
    tid = thbase_4_inv(&tp);
    if (tid >= 0) {
        StartArg_inv a = { c, thbase_20_inv(), r };

        thbase_6_inv(tid, &a);
        thbase_24_inv();
        r = a.result;
    }
    return r;
}

int fn_iStartServer_inv(TidPair_inv *p)
{
    ThreadCfg_inv c;

    c.x0 = p->x0;
    c.x4 = p->x4;
    c.stackbuf = a_ucServerStack_inv;
    c.stack = 0x800;
    return fn_iCreateRemoteThread_inv(&c, fn_iServerLoop);
}

int fn_iStartClient_inv(TidPair_inv *p)
{
    ThreadCfg_inv c;

    c.x0 = p->x0;
    c.x4 = p->x4;
    c.stackbuf = a_ucClientStack_inv;
    c.stack = 0x800;
    return fn_iCreateRemoteThread_inv(&c, fn_iClientLoop);
}
#else
int fn_iStartServer_inv(TidPair_inv *p)
{
    pc_serve((PcAddress)p->x4, a_ucServerBuffer_inv);
    return 0;
}

int fn_iStartClient_inv(TidPair_inv *p)
{
    (void)p;
    return 0;
}
#endif

int fn_iStartServerClient_inv(TidPair_inv *out, TidPair_inv *a, TidPair_inv *b)
{
    int r = -1;

    out->x4 = fn_iStartServer_inv(a);
    out->x0 = fn_iStartClient_inv(b);
    if (out->x4 >= 0 && out->x0 >= 0)
        r = 0;
    return r;
}

int fn_iRecover(int tid)
{
#ifdef PC_PORT
    (void)tid;
#else
    thbase_5_inv(tid);
#endif
    return 0;
}

int fn_iRecoverServerClient(TidPair_inv *p)
{
    fn_iRecover(p->x0);
    fn_iRecover(p->x4);
    return 0;
}
