#include "include_asm.h"
#include "sndcmd_inv.h"
#include "snditbase_inv.h"

typedef struct {
    int x0;
    int x4;
} Name_inv;

typedef struct {
    int x0;
    int x4;
    int x8;
    char desc[36];
    char x30[16];
} OpenReq_inv;

typedef struct {
    unsigned int size;
    char *addr;
} tdstSpuBlock_inv;

typedef struct {
    tdstSpuBlock_inv blocks[50];
    unsigned int avail;
    char *next;
    tdstSpuBlock_inv *last;
} tdstSpuHeap_inv;

typedef struct {
    Iface_inv *iface;
    void *h;
} HandleOut_inv;

extern int g_lNbPlayAllocated_inv;
extern CmdHandler_inv a_pfnPlayCmd_inv[12];
extern tdstSpuHeap_inv g_stSpuHeap_inv;
extern int sysmem_7_inv(void);
extern int sysmem_8_inv(void);
extern int fn_iInitInterfaceEnv_inv(void);
extern int fn_iDesInitInterfaceEnv_inv(void);
extern Iface_inv *fn_pstValidateInterface_inv(void *id);
extern int fn_iStartClient_inv(Name_inv *p);
extern int fn_iRemoteCall();
extern void fn_vSetStereo_inv(int x);
extern int fn_bGetStereo_inv(void);

int fn_iInitPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    Name_inv name;
    int a;
    int b;
    int r = -1;

    if (insz == 4 && outsz == insz) {
#ifdef PC_PORT
        a = pc_mem_available();
        b = pc_mem_largest();
#else
        a = sysmem_7_inv();
        b = sysmem_8_inv();
#endif
        if (fn_iInitInterfaceEnv_inv() >= 0) {
            r = 0;
            name.x0 = 0x696F3265;
            name.x4 = 0;
            fn_iStartClient_inv(&name);
            fn_iRemoteCall(0x30000, 0, 0, 0, in, outsz);
        }
        *out = a < b ? a : b;
    }
    return r;
}

int fn_iDesInitPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if (insz == 4) {
        r = fn_iDesInitInterfaceEnv_inv();
        fn_iRemoteCall(0x30000, 0, 0, 0, in, insz);
    }
    return r;
}

int fn_iAllocatePlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    OpenReq_inv *q = in;
    HandleOut_inv *o = (HandleOut_inv *)out;
    Iface_inv *ifc;
    void *h;
    void *h2;
    int r = -1;

    if (insz == 64 && outsz == 8) {
        ifc = fn_pstFindInterface_inv(4, q->desc, 0, 0);
        if (ifc != 0) {
            h = ifc->AllocateVoice(q->desc);
            if (h != 0) {
                h2 = (void *)ifc->u.play.PushNextBlock(h, 0, q->x30);
                if (h2 == h && ifc->u.play.SetCallbackArg(h2, q->x8) >= 0 && ifc->u.play.Start(h2, q) >= 0) {
                    r = 0;
                    o->iface = ifc;
                    o->h = h2;
                    g_lNbPlayAllocated_inv++;
                }
            }
        }
    }
    return r;
}

int fn_iDesallocatePlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 8) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0 && ifc->DesallocateVoice((void *)q[1]) == 0) {
            r = 0;
            g_lNbPlayAllocated_inv--;
        }
    }
    return r;
}

int fn_iIsAllocatedPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 8 && outsz == 4) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0) {
            r = 0;
            *out = ifc->IsAllocated((void *)q[1]);
        }
    }
    return r;
}

int fn_iGetPosPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 8 && outsz == 4) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0) {
            if (ifc->u.play.GetPos((void *)q[1], out) >= 0)
                r = 0;
        }
    }
    return r;
}

int fn_iPushNextBlockPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 28 && outsz == 8) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0) {
            r = 0;
            out[0] = (int)ifc;
            out[1] = ifc->u.play.PushNextBlock((void *)q[1], q[2], &q[3]);
        }
    }
    return r;
}

int fn_iPausePlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 8) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0) {
            if (ifc->u.play.Pause((void *)q[1]) < 0)
                return r;
            r = 0;
        }
    }
    return r;
}

int fn_iResumePlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 8) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0) {
            if (ifc->u.play.Resume((void *)q[1]) < 0)
                return r;
            r = 0;
        }
    }
    return r;
}

int fn_iSetParamPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int *q = in;
    Iface_inv *ifc;
    int r = -1;

    if (insz == 16 && outsz == 8) {
        ifc = fn_pstValidateInterface_inv((void *)q[0]);
        if (ifc != 0) {
            r = 0;
            out[0] = (int)ifc;
            out[1] = ifc->u.play.SetParam((void *)q[1], &q[2]);
        }
    }
    return r;
}

int fn_iSetStereoPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if (insz == 4) {
        fn_vSetStereo_inv(*(int *)in);
        r = 0;
    }
    return r;
}

int fn_iGetStereoPlayCmd_inv(int *out, int outsz, void *in, int insz)
{
    int r = -1;

    if (outsz == 4) {
        *out = fn_bGetStereo_inv();
        r = 0;
    }
    return r;
}

int fn_iGetPlayInterface_inv(tdstCmdModule_inv *t)
{
    CmdHandler_inv *c = a_pfnPlayCmd_inv;

    c[0] = fn_iInitPlayCmd_inv;
    c[1] = fn_iDesInitPlayCmd_inv;
    c[2] = fn_iAllocatePlayCmd_inv;
    c[3] = fn_iDesallocatePlayCmd_inv;
    c[4] = fn_iPausePlayCmd_inv;
    c[5] = fn_iResumePlayCmd_inv;
    c[6] = fn_iSetParamPlayCmd_inv;
    c[7] = fn_iIsAllocatedPlayCmd_inv;
    c[8] = fn_iGetPosPlayCmd_inv;
    c[9] = fn_iPushNextBlockPlayCmd_inv;
    c[10] = fn_iSetStereoPlayCmd_inv;
    c[11] = fn_iGetStereoPlayCmd_inv;
    t->tag = 0x79616C70;
    t->x4 = 1;
    t->count = 12;
    t->cmds = c;
    return 0;
}

#ifdef PC_PORT
static char a_ucSoundMemory_inv[0x1FAFF0] __attribute__((aligned(64)));

#endif
void fn_vConstructTdstSpuHeap_inv(tdstSpuHeap_inv *p)
{
#ifdef PC_PORT
    pc_memset(p, 0, sizeof(p->blocks));
#else
    sysclib_14_inv(p, 0, sizeof(p->blocks));
#endif
#ifdef PC_PORT
    p->avail = sizeof(a_ucSoundMemory_inv);
    p->next = a_ucSoundMemory_inv;
#else
    p->avail = 0x1FAFF0;
    p->next = (char *)0x5010;
#endif
    p->last = 0;
}

tdstSpuBlock_inv *fn_pstAllocTdstSpuHeap_inv(tdstSpuHeap_inv *p, unsigned int size)
{
    tdstSpuBlock_inv *r = 0;
    tdstSpuBlock_inv *b;
    unsigned int n = (size + 15) & ~15;

    if (p->avail >= size) {
        for (b = p->blocks; b < &p->blocks[50]; b++) {
            if (b->size == 0) {
                b->size = n;
                b->addr = p->next;
                r = b;
                p->last = r;
                p->avail -= n;
                p->next += n;
                break;
            }
        }
    }
    return r;
}

tdstSpuBlock_inv *fn_pstFreeTdstSpuHeap_inv(tdstSpuHeap_inv *p, char *addr)
{
    tdstSpuBlock_inv *r = 0;
    tdstSpuBlock_inv *b;
    unsigned int n;
    tdstSpuBlock_inv *end;

    b = p->blocks;
    end = b + 50;
    if (b < end) {
    next:
        if (b->addr == addr) {
            r = b;
            n = r->size;
            r->size = 0;
            r->addr = 0;
            p->last = r - 1;
            p->avail += n;
            p->next -= n;
            goto out;
        }
        b++;
        if (b < end)
            goto next;
    }
out:
    return r;
}

int fn_iInitSpuHeap_inv(void)
{
    fn_vConstructTdstSpuHeap_inv(&g_stSpuHeap_inv);
    return 1;
}

char *fn_pvAllocSpu_inv(unsigned int size)
{
    return fn_pstAllocTdstSpuHeap_inv(&g_stSpuHeap_inv, size)->addr;
}

void fn_vFreeSpu_inv(char *addr)
{
    fn_pstFreeTdstSpuHeap_inv(&g_stSpuHeap_inv, addr);
}
