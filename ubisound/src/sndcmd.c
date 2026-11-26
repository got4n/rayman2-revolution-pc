#include "include_asm.h"

typedef int (*CmdHandler_inv)(void *out, int a, void *in, int b);

typedef struct {
    int x0;
    unsigned int id;
    int count;
    CmdHandler_inv *fns;
} tdstCmdModule_inv;

typedef struct {
    tdstCmdModule_inv *base;
    tdstCmdModule_inv *end;
} tdstCmdInterface_inv;

extern tdstCmdInterface_inv g_stServerFunc_inv;
extern int fn_iExecServerFunc_inv();

int fn_iInitServerFunc(tdstCmdModule_inv *base, tdstCmdModule_inv *end)
{
    tdstCmdInterface_inv *t = &g_stServerFunc_inv;

    t->base = base;
    t->end = end;
    return 0;
}

#ifdef PC_PORT
tdstCmdInterface_inv g_stServerFunc_inv;
#define g_pstServerFuncBegin_inv (g_stServerFunc_inv.base)
#define g_pstServerFuncEnd_inv (g_stServerFunc_inv.end)
#else
extern tdstCmdModule_inv *g_pstServerFuncBegin_inv asm("g_stServerFunc_inv");
extern tdstCmdModule_inv *g_pstServerFuncEnd_inv asm("D_00013E44");
#endif

int fn_iExecServerFunc_inv(unsigned int cmd, int *out, int outsize, int *in, int insize)
{
    int r = -1;
    CmdHandler_inv fn = 0;
    int idx = (short)cmd;
    unsigned int id = cmd >> 16;
    tdstCmdModule_inv *e;
    int i;

    i = idx;

    if (i >= 0) {
        for (e = g_pstServerFuncBegin_inv; e < g_pstServerFuncEnd_inv; e++) {
            if (e->id == id && e->count >= i) {
                fn = e->fns[i];
                break;
            }
        }
    }
    if (fn != 0) {
        int n = in[0];
        int sz = in[1];

        if (n <= outsize - 8) {
            *out = fn(out + 1, n, in + 2, sz);
            r = 0;
        }
    }
    return r;
}

extern char a_ucServerReply_inv[];

char *fn_pDispatchServerFunc_inv(unsigned int cmd, char *in, int insz)
{
    char *p;
    unsigned int id;
    int multi;
    char *out;
    unsigned int count;
    int room;
    char *ret;
    unsigned int *hdr;
    int ok;
    int n;

    ret = 0;
    hdr = 0;
    id = cmd;
    room = 0x400;
    out = a_ucServerReply_inv;
    n = insz;
    p = in;
    count = id & 0xFFFF;
    multi = (unsigned short)(id >> 16) == 0xFFFF;

    do {
        if (multi) {
            p += 8;
            hdr = (unsigned int *)p;
            id = *(unsigned int *)p;
            n -= 12;
            p += 4;
        }
        ok = fn_iExecServerFunc_inv(id, out, room, p, n) >= 0;
        if (!multi)
            break;
        count--;
        room -= hdr[1];
        out += hdr[1];
        n -= hdr[2];
        p += hdr[2];
        if (count == 0)
            break;
    } while (ok);
    if (ok)
        ret = a_ucServerReply_inv;
    return ret;
}
