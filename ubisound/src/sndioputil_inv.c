#include "include_asm.h"
#include "snditbase_inv.h"

extern char D_00020AE0[];
extern int libsd_4_inv();
extern int intrman_9_inv();
extern int intrman_6_inv();
extern void libsd_11_inv();
extern int fn_iInitSpuHeap_inv();

static inline void ClearInfo_inv(Info_inv *i)
{
    i->kind = 0;
    i->h = 0;
    i->iface = 0;
}

int fn_iInitSpu_inv(void)
{
    int r = -1;

#ifdef PC_PORT
    pc_mixer_open();
    {
        r = 0;
#else
    if (libsd_4_inv(0) == 0) {
        intrman_9_inv();
        r = 0;
        intrman_6_inv(0x24);
        intrman_6_inv(0x28);
        intrman_6_inv(9);
        libsd_11_inv(10, 0x800);
#endif
        fn_iInitSpuHeap_inv(0x32, D_00020AE0);
    }
    return r;
}

int fn_iFindChild_inv(Info_inv *out, int kind, Info_inv *in)
{
    Info_inv tmp;
    int r = -1;
    int i;
    int n;

    if (in->kind == kind) {
        out->kind = kind;
        out->h = in->h;
        r = 0;
        out->iface = in->iface;
    } else {
        ClearInfo_inv(&tmp);
        n = in->iface->GetNumChild(in->h);
        for (i = 0; i < n; i++) {
            if (r >= 0)
                break;
            in->iface->GetChild(in->h, &tmp, i);
            r = fn_iFindChild_inv(out, kind, &tmp);
        }
    }
    return r;
}
