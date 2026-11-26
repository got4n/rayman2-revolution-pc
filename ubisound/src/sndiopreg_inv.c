#include "include_asm.h"
#include "sndcmd_inv.h"
#include "snditbase_inv.h"

typedef struct {
    long b0 : 1;
    long b1 : 1;
} tdstInterfaceMngrFlags_inv;

typedef struct tdstInterfaceMngr_inv tdstInterfaceMngr_inv;
typedef int (*InterfaceStateFn_inv)(tdstInterfaceMngr_inv *m);
typedef int (*InterfaceFn_inv)(Iface_inv *ifc, void *arg);

struct tdstInterfaceMngr_inv {
    tdstInterfaceMngrFlags_inv f;
    int sema0;
    int sema1;
    int thread;
    int x10;
    unsigned int state;
    InterfaceStateFn_inv handler[2];
    Iface_inv *classes[16];
};

typedef struct {
    int kind;
    void *desc;
    int c;
    InterfaceFn_inv fn;
} Find_inv;

typedef struct {
    int attr;
    int x4;
    int count;
    int max;
} SemaDesc_inv;

typedef struct {
    int attr;
    int x4;
    InterfaceStateFn_inv entry;
    int stack;
    int prio;
} ThreadDesc_inv;

typedef struct {
    int x0;
    int x4;
} RpcTmp_inv;

extern tdstInterfaceMngr_inv g_stTdstInterfaceMngr_inv;
extern CmdHandler_inv a_pfnLoadCmd_inv[10];
extern int thsemap_4_inv(SemaDesc_inv *d);
extern int thsemap_5_inv(int sema);
extern int thsemap_6_inv(int sema);
extern int thsemap_7_inv(int sema);
extern int thsemap_8_inv(int sema);
extern int thbase_4_inv(ThreadDesc_inv *d);
extern int thbase_5_inv(int thread);
extern int thbase_6_inv(int thread, void *arg);
extern int thbase_35_inv(RpcTmp_inv *t, InterfaceStateFn_inv fn, tdstInterfaceMngr_inv *m);
extern int thbase_37_inv(InterfaceStateFn_inv fn, tdstInterfaceMngr_inv *m);
extern int thbase_39_inv(int id, RpcTmp_inv *t);
extern Iface_inv *fn_pstGetSpuSampleInterface_inv(void);
extern Iface_inv *fn_pstGetPlaySampleInterface_inv(void);
extern Iface_inv *fn_pstGetChannelMEMMonoInterface_inv(void);
extern Iface_inv *fn_pstGetChannelCDInterface_inv(void);
extern Iface_inv *fn_pstGetSpuStreamInterface_inv(void);
extern Iface_inv *fn_pstGetChannelMEMInterface_inv(void);
extern Iface_inv *fn_pstGetChannelCDStereoInterface_inv(void);
extern Iface_inv *fn_pstGetSpuStreamStereoInterface_inv(void);
extern Iface_inv *fn_pstGetPlayStreamInterface_inv(void);
extern int fn_iRemoteCall();
extern int libsd_17_inv(int a, int b, void *src, void *dst, int n);
extern int sysmem_5_inv(void *p);
extern int cdvdman_7_inv(int start);
extern int cdvdman_11_inv(int x);
extern int cdvdman_13_inv(int x);
extern char *fn_pvAllocSpu_inv(unsigned int size);
extern void fn_vFreeSpu_inv(char *addr);
int fn_iLoadFileIOPLoadCmd_inv(int *out, int outsz, void *in, int insz);
int fn_iLoadFileSPULoadCmd_inv(int *out, int outsz, void *in, int insz);

static inline InterfaceStateFn_inv GetRegHandler_inv(tdstInterfaceMngr_inv *m, unsigned int i)
{
    InterfaceStateFn_inv r = 0;

    if (i < 3)
        r = m->handler[i];
    return r;
}

#define MinU_inv(a, b) ((a) < (b) ? (a) : (b))

#define Sectors_inv(x) ((x) % 2048 ? (x) / 2048 + 1 : (x) / 2048)

int fn_iDesInitInterface_inv(Iface_inv *ifc, int *ok)
{
    if (ifc != 0)
        *ok = ifc->DesInit() >= 0 && *ok;
    return 0;
}

int fn_iInitInterface_inv(Iface_inv *ifc, int *ok)
{
    int r = 1;

    if (ifc != 0) {
        *ok = ifc->Init() >= 0 && *ok;
        if (*ok)
            r = 0;
    }
    return r;
}

int fn_bIsInterface_inv(Iface_inv *ifc, Iface_inv **key)
{
    return ifc == *key;
}

int fn_bTestInterface_inv(Iface_inv *ifc, Find_inv *q)
{
    int r = 0;

    if (ifc != 0 && ifc->TestFormat(q->desc, q->kind)) {
        r = 1;
        if (q->fn != 0)
            r = q->fn(ifc, (void *)q->c);
    }
    return r;
}

int fn_iCountInterface_inv(Iface_inv *ifc, int *n)
{
    if (ifc != 0)
        (*n)++;
    return 0;
}

Iface_inv *fn_pstParseTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m, void *arg, InterfaceFn_inv fn)
{
    Iface_inv *r = 0;
    Iface_inv **p;

    for (p = m->classes; p < &m->classes[16]; p++) {
        if (*p != 0 && fn(*p, arg)) {
            r = *p;
            break;
        }
    }
    return r;
}

#ifdef PC_PORT
int fn_iAlarmTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    pc_task_signal(m->thread);
    return m->x10;
}
#else
int fn_iAlarmTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    thsemap_7_inv(m->sema1);
    return m->x10;
}
#endif

#ifdef PC_PORT
int fn_iThreadTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    InterfaceStateFn_inv fn;

    if (m->f.b1) {
        fn = GetRegHandler_inv(m, m->state);
        if (fn != 0)
            fn(m);
    }
    return 0;
}
#else
int fn_iThreadTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    RpcTmp_inv t;
    InterfaceStateFn_inv fn;

    thbase_39_inv(0x30D40, &t);
    m->x10 = t.x0;
    thbase_35_inv(&t, fn_iAlarmTdstInterfaceMngr_inv, m);
    while (m->f.b1) {
        thsemap_8_inv(m->sema1);
        fn = GetRegHandler_inv(m, m->state);
        if (fn != 0)
            fn(m);
    }
    thbase_37_inv(fn_iAlarmTdstInterfaceMngr_inv, m);
    thsemap_6_inv(m->sema0);
    return 0;
}
#endif

int fn_iStateWaitTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    m->state = 1;
    return 0;
}

int fn_iOperateCallInterface_inv(Iface_inv *ifc)
{
    ifc->OperateCall();
    return 0;
}

int fn_iStateOperateTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    fn_pstParseTdstInterfaceMngr_inv(m, 0, (InterfaceFn_inv)fn_iOperateCallInterface_inv);
    m->state = 1;
    return 0;
}

#ifdef PC_PORT
int fn_iConstructTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    int ok;
    int r;

    if (!m->f.b0) {
        ok = 1;
        m->classes[0] = fn_pstGetSpuSampleInterface_inv();
        m->classes[1] = fn_pstGetPlaySampleInterface_inv();
        m->classes[2] = fn_pstGetChannelMEMMonoInterface_inv();
        m->classes[3] = fn_pstGetChannelCDInterface_inv();
        m->classes[4] = fn_pstGetSpuStreamInterface_inv();
        m->classes[5] = fn_pstGetChannelMEMInterface_inv();
        m->classes[6] = fn_pstGetChannelCDStereoInterface_inv();
        m->classes[7] = fn_pstGetSpuStreamStereoInterface_inv();
        m->classes[8] = fn_pstGetPlayStreamInterface_inv();
        fn_pstParseTdstInterfaceMngr_inv(m, &ok, (InterfaceFn_inv)fn_iInitInterface_inv);
        r = -1;
        if (ok) {
            m->handler[0] = fn_iStateWaitTdstInterfaceMngr_inv;
            m->handler[1] = fn_iStateOperateTdstInterfaceMngr_inv;
            m->f.b0 = 1;
            m->f.b1 = 1;
            m->sema0 = 0;
            m->sema1 = 0;
            m->x10 = 0x30D40;
            m->thread = pc_task_create((PcAddress)fn_iThreadTdstInterfaceMngr_inv, (uint32_t)m, 107);
            m->state = 0;
            r = 0;
            pc_task_every(m->thread, m->x10);
        }
    } else {
        r = 0;
    }
    return r;
}
#else
int fn_iConstructTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    SemaDesc_inv sd;
    ThreadDesc_inv td;
    int ok;
    int r;

    if (!m->f.b0) {
        ok = 1;
        m->classes[0] = fn_pstGetSpuSampleInterface_inv();
        m->classes[1] = fn_pstGetPlaySampleInterface_inv();
        m->classes[2] = fn_pstGetChannelMEMMonoInterface_inv();
        m->classes[3] = fn_pstGetChannelCDInterface_inv();
        m->classes[4] = fn_pstGetSpuStreamInterface_inv();
        m->classes[5] = fn_pstGetChannelMEMInterface_inv();
        m->classes[6] = fn_pstGetChannelCDStereoInterface_inv();
        m->classes[7] = fn_pstGetSpuStreamStereoInterface_inv();
        m->classes[8] = fn_pstGetPlayStreamInterface_inv();
        fn_pstParseTdstInterfaceMngr_inv(m, &ok, (InterfaceFn_inv)fn_iInitInterface_inv);
        r = -1;
        if (ok) {
            m->handler[0] = fn_iStateWaitTdstInterfaceMngr_inv;
            m->handler[1] = fn_iStateOperateTdstInterfaceMngr_inv;
            m->f.b0 = 1;
            m->f.b1 = 1;
            sd.attr = 0;
            sd.x4 = 0;
            sd.count = 0;
            sd.max = 1;
            m->sema0 = thsemap_4_inv(&sd);
            m->sema1 = thsemap_4_inv(&sd);
            td.attr = 0x2000000;
            td.entry = fn_iThreadTdstInterfaceMngr_inv;
            td.prio = 107;
            td.stack = 0x800;
            td.x4 = 0;
            m->thread = thbase_4_inv(&td);
            m->state = 0;
            r = 0;
            thbase_6_inv(m->thread, m);
        }
    } else {
        r = 0;
    }
    return r;
}
#endif

#ifdef PC_PORT
int fn_iDestructTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    int ok;
    int r;

    fn_pstParseTdstInterfaceMngr_inv(m, &ok, (InterfaceFn_inv)fn_iDesInitInterface_inv);
    r = -1;
    if (ok) {
        m->f.b0 = 0;
        m->f.b1 = 0;
        if (m->thread != 0) {
            pc_task_delete(m->thread);
            m->thread = -1;
        }
        m->sema1 = 0;
        r = 0;
        m->sema0 = 0;
    }
    return r;
}
#else
int fn_iDestructTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    int ok;
    int r;

    fn_pstParseTdstInterfaceMngr_inv(m, &ok, (InterfaceFn_inv)fn_iDesInitInterface_inv);
    r = -1;
    if (ok) {
        m->f.b0 = 0;
        m->f.b1 = 0;
        thsemap_8_inv(m->sema0);
        if (m->thread != 0) {
            thbase_5_inv(m->thread);
            m->thread = -1;
        }
        if (m->sema1 != 0) {
            thsemap_5_inv(m->sema1);
            m->sema1 = 0;
        }
        r = 0;
        if (m->sema0 != 0) {
            thsemap_5_inv(m->sema0);
            m->sema0 = 0;
        }
    }
    return r;
}
#endif

Iface_inv *fn_pstFindTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m, Find_inv *q)
{
    return fn_pstParseTdstInterfaceMngr_inv(m, q, (InterfaceFn_inv)fn_bTestInterface_inv);
}

Iface_inv *fn_pstValidateTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m, Iface_inv *key)
{
    return fn_pstParseTdstInterfaceMngr_inv(m, &key, (InterfaceFn_inv)fn_bIsInterface_inv);
}

Iface_inv *fn_pstCountTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m)
{
    int n = 0;

    return fn_pstParseTdstInterfaceMngr_inv(m, &n, (InterfaceFn_inv)fn_iCountInterface_inv);
}

int fn_iInitInterfaceEnv_inv(void)
{
    return fn_iConstructTdstInterfaceMngr_inv(&g_stTdstInterfaceMngr_inv);
}

int fn_iDesInitInterfaceEnv_inv(void)
{
    return fn_iDestructTdstInterfaceMngr_inv(&g_stTdstInterfaceMngr_inv);
}

Iface_inv *fn_pstFindInterface_inv(int kind, void *desc, int c, int d)
{
    Find_inv q = {kind, desc, c, (InterfaceFn_inv)d};

    return fn_pstFindTdstInterfaceMngr_inv(&g_stTdstInterfaceMngr_inv, &q);
}

Iface_inv *fn_pstCountInterface_inv(void)
{
    return fn_pstCountTdstInterfaceMngr_inv(&g_stTdstInterfaceMngr_inv);
}

Iface_inv *fn_pstValidateInterface_inv(Iface_inv *key)
{
    return fn_pstValidateTdstInterfaceMngr_inv(&g_stTdstInterfaceMngr_inv, key);
}

int fn_bIsValidStateTdstInterfaceMngr_inv(unsigned int i)
{
    return i < 3;
}

void fn_vSetStateHandlerTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m, int i, InterfaceStateFn_inv fn)
{
    m->handler[i] = fn;
}

InterfaceStateFn_inv fn_pfnGetStateHandlerTdstInterfaceMngr_inv(tdstInterfaceMngr_inv *m, unsigned int i)
{
    return GetRegHandler_inv(m, i);
}

int fn_iCompareMem_inv(char *a0, char *b0, int n)
{
    int *pa;
    int *pb;
    char *pa2;
    char *pb2;
    int count = 0;
    int i;
    int m;
    char *b = b0;
    char *a = a0;

    if (((int)b & 3) == 0 && ((int)a & 3) == 0) {
        i = 0;
        pb = (int *)b;
        pa = (int *)a;
        m = (n + 3) & ~3;
        while (*pb == *pa && i < m) {
            pa++;
            pb++;
            i += 4;
        }
        count = i;
        n -= count;
        b += count;
        a += count;
    }
    i = 0;
    pb2 = b;
    pa2 = a;
    while (*pb2 == *pa2 && i < n) {
        pb2++;
        pa2++;
        i++;
    }
    return count + i;
}

int fn_iReadSpu_inv(void *dst, void *src, int n)
{
#ifdef PC_PORT
    pc_memcpy(dst, src, n);
    return n;
#else
    return libsd_17_inv(1, 0, src, dst, n);
#endif
}

int fn_iCallbackLoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if (insz == 4) {
        fn_iRemoteCall(0x30000, 0, 0, 0, in, insz);
        r = 0;
    }
    return r;
}

int fn_iCallbackFailLoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    if (insz == 4)
        fn_iRemoteCall(0x30000, 0, 0, 0, in, insz);
    return -1;
}

int fn_iLoadMemSPULoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    int src;
    int n;
    int buf;
    int r = -1;

    if (insz == 8 && outsz == insz) {
        n = q[1];
        src = q[0];
        buf = (int)fn_pvAllocSpu_inv(n);
#ifdef PC_PORT
        pc_memcpy((void *)buf, (void *)src, n);
#else
        libsd_17_inv(1, 0, (void *)src, (void *)buf, n);
#endif
        if (buf >= 0) {
            r = 0;
            out[1] = n;
            out[0] = buf;
        }
    }
    return r;
}

int fn_iUnLoadMemSPULoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if ((unsigned int)insz >= 4) {
        fn_vFreeSpu_inv(*(char **)in);
        r = 0;
    }
    return r;
}

typedef struct {
    unsigned int size;
    char *data;
} LoadSpan_inv;

typedef struct {
    unsigned char x0;
    unsigned char x1;
    unsigned char x2;
} ReadMode_inv;

extern int libsd_19_inv(int a, int b);
extern char *sysmem_4_inv(int a, unsigned int size, int c);
extern unsigned int sysmem_7_inv(void);
extern unsigned int sysmem_8_inv(void);
extern int cdvdman_6_inv(int start, unsigned int n, void *dst, ReadMode_inv *mode);

static inline unsigned int Sec_inv(unsigned int x)
{
    unsigned int n;

    if (x % 2048 == 0)
        n = x / 2048;
    else
        n = x / 2048 + 1;
    return n;
}

int fn_iLoadFileIOPLoadCmd_inv(int *out0, int outsz, void *in, int insz)
{
    int *out = out0;
    ReadMode_inv mode;
    unsigned int *q = in;
    unsigned int n;
    unsigned int size;
    char *buf;
    int r = -1;

    mode.x0 = 0;
    mode.x1 = 1;
    mode.x2 = 0;
    out[0] = 0;
    out[1] = 0;
    if ((unsigned int)insz >= 8 && outsz == 8 && q[0] != 0) {
        if (q[1] != 0) {
            n = Sec_inv(q[1]);
            size = n * 2048;
#ifdef PC_PORT
            buf = pc_mem_alloc(size);
#else
            buf = sysmem_4_inv(0, size, 0);
#endif
            if (buf != 0) {
#ifdef PC_PORT
                if (pc_file_read((const PcFile *)q, 0, buf, size) == 1) {
                    out[0] = (int)buf;
                    r = 0;
                    out[1] = q[1];
                    pc_memset((char *)out[0] + out[1], 0, size - q[1]);
                } else {
                    pc_mem_free(buf);
                    pc_log("sound: a file to load could not be read");
                }
#else
                cdvdman_13_inv(0);
                if (cdvdman_6_inv(q[0], n, buf, &mode) == 1) {
                    out[0] = (int)buf;
                    r = 0;
                    out[1] = q[1];
                }
                cdvdman_11_inv(0);
                sysclib_14_inv((char *)out[0] + out[1], 0, size - q[1]);
#endif
            }
        } else {
            r = 0;
        }
    }
    return r;
}

int fn_iUnLoadFileIOPLoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if ((unsigned int)insz >= 4) {
#ifdef PC_PORT
        pc_mem_free(*(void **)in);
#else
        sysmem_5_inv(*(void **)in);
#endif
        r = 0;
    }
    return r;
}

#ifdef PC_PORT
int fn_iLoadFileSPULoadCmd_inv(int *out0, int outsz, void *in, int insz)
{
    int r = -1;
    unsigned int *q = in;
    int *out = out0;
    char *buf;

    if ((unsigned int)outsz >= 8) {
        buf = fn_pvAllocSpu_inv(q[1]);
        if (buf != 0) {
            if (pc_file_read((const PcFile *)q, 0, buf, q[1]) != 1)
                pc_log("sound: a file to load could not be read");
            r = 0;
        }
        if (r >= 0) {
            out[1] = q[1];
            out[0] = (int)buf;
        } else if (buf != 0) {
            fn_vFreeSpu_inv(buf);
        }
    }
    return r;
}
#else
int fn_iLoadFileSPULoadCmd_inv(int *out0, int outsz, void *in, int insz)
{
    int r = -1;
    unsigned int *q = in;
    int *out = out0;
    LoadSpan_inv sp;
    ReadMode_inv mode;
    char *buf;
    char *tmp;
    unsigned int half;
    unsigned int len;
    int size;
    unsigned int off;
    int mask;
    unsigned int left;
    unsigned int n;

    if ((unsigned int)outsz >= 8) {
        mode.x0 = 0;
        mode.x1 = 1;
        mode.x2 = 0;
        buf = fn_pvAllocSpu_inv(q[1]);
        half = MinU_inv(sysmem_7_inv(), sysmem_8_inv()) / 2;
        len = half;
        if (len >= q[1])
            len = q[1];
        half = len;
        size = (int)half / 2048 * 2048;
        tmp = sysmem_4_inv(0, size + 6144, 0);
        half = 127;
        size += 2048;
        mask = ~127;
        off = 0;
        sp.size = size;
        sp.data = (char *)(((int)tmp + half) & mask);
        if (buf != 0 && tmp != 0) {
            for (left = q[1]; left != 0; left -= n, off += n) {
                n = sp.size;
                if (left < n)
                    n = left;
                cdvdman_13_inv(0);
                cdvdman_6_inv(q[0] + Sec_inv(off), Sec_inv(n), sp.data, &mode);
                cdvdman_11_inv(0);
                libsd_17_inv(1, 8, sp.data, buf + off, n);
                while (libsd_19_inv(1, 1) != 1)
                    ;
            }
            r = 0;
        }
        if (tmp != 0)
            sysmem_5_inv(tmp);
        if (r >= 0) {
            out[1] = q[1];
            out[0] = (int)buf;
        } else if (buf != 0) {
            fn_vFreeSpu_inv(buf);
        }
    }
    return r;
}
#endif

int fn_iUnLoadFileSPULoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if ((unsigned int)insz >= 4) {
        fn_vFreeSpu_inv(*(char **)in);
        r = 0;
    }
    return r;
}

int fn_iLoadFileCDLoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    int r = -1;

    out[0] = 0;
    out[1] = 0;
    if ((unsigned int)outsz >= 8) {
        out[0] = q[0];
        out[1] = q[1];
#ifdef PC_PORT
        r = 0;
#else
        cdvdman_13_inv(0);
        r = 0;
        cdvdman_7_inv(q[0]);
        cdvdman_11_inv(r);
#endif
    }
    return r;
}

int fn_iNullLoadCmd_inv(int *out, int outsz, void *in, int insz)
{
    return 0;
}

int fn_iGetLoadInterface_inv(tdstCmdModule_inv *t)
{
    CmdHandler_inv *c = a_pfnLoadCmd_inv;

    c[0] = fn_iCallbackLoadCmd_inv;
    c[1] = fn_iCallbackFailLoadCmd_inv;
    c[2] = fn_iLoadFileSPULoadCmd_inv;
    c[3] = fn_iUnLoadFileSPULoadCmd_inv;
    c[4] = fn_iLoadFileIOPLoadCmd_inv;
    c[5] = fn_iUnLoadFileIOPLoadCmd_inv;
    c[6] = fn_iLoadFileCDLoadCmd_inv;
    c[7] = fn_iNullLoadCmd_inv;
    c[8] = fn_iLoadMemSPULoadCmd_inv;
    c[9] = fn_iUnLoadMemSPULoadCmd_inv;
    t->tag = 0x64616F6C;
    t->x4 = 2;
    t->count = 10;
    t->cmds = c;
    return 0;
}
