#ifdef PC_PORT
#include "PC/PCDriver.h"
#endif
#ifdef PC_PORT
extern char *fn_pDispatchServerFunc_inv(unsigned int cmd, char *in, int insz);
#else
extern void fn_pDispatchServerFunc_inv(void);
#endif
extern int fn_iStartServer_inv(void *p);
extern int fn_iInitIOPCmdModule_inv(void);
extern void loadcore_5_inv(void);
extern int intrman_9_inv(void);

typedef struct {
    int id;
    void (*entry)(void);
} Pair_inv;

#ifdef PC_PORT
#define PC_DATA_inv(name, size) char name[size] __attribute__((aligned(16)))
PC_DATA_inv(a_ucServerReply_inv, 0x400);
PC_DATA_inv(g_stRemoteRequest_inv, 0x400);
PC_DATA_inv(g_stRemoteReply_inv, 0x400);
PC_DATA_inv(a_ucServerBuffer_inv, 0x400);
PC_DATA_inv(a_ucServerStack_inv, 0x800);
PC_DATA_inv(a_ucClientStack_inv, 0x800);
PC_DATA_inv(a_pfnPlayCmd_inv, 0x30);
PC_DATA_inv(g_stSpuHeap_inv, 0x1A0);
PC_DATA_inv(g_stTdstChannelCDMngr_inv, 0xF0);
PC_DATA_inv(g_stTdstChannelCDStereoMngr_inv, 0x80E0);
PC_DATA_inv(D_00020130, 0x400);
PC_DATA_inv(g_stTdstChannelMEMMngr_inv, 0x3D0);
PC_DATA_inv(g_stTdstSpuStreamStereoMngr_inv, 0x160);
PC_DATA_inv(a_stIOPCmdModule_inv, 0x80);
PC_DATA_inv(D_00020AE0, 0x1A0);
PC_DATA_inv(g_stTdstInterfaceMngr_inv, 0x60);
PC_DATA_inv(a_pfnLoadCmd_inv, 0x30);
PC_DATA_inv(g_stTdstChannelMEMMonoMngr_inv, 0x2B0);
PC_DATA_inv(a_stSpuCallback_inv, 0x20);
PC_DATA_inv(g_stTdstSpuSampleMngr_inv, 0x430);
PC_DATA_inv(g_stTdstPlayStreamMngr_inv, 0xB50);
PC_DATA_inv(g_stTdstPlaySampleMngr_inv, 0x850);
int g_lNbPlayAllocated_inv;
int g_lSpuCallbackInit_inv;

#endif
int fn_iInitSndIop_inv(void)
{
    int ret = -1;
    Pair_inv p;

    if (fn_iInitIOPCmdModule_inv() >= 0) {
        p.id = 0x65653269;
#ifdef PC_PORT
        p.entry = (void (*)(void))fn_pDispatchServerFunc_inv;
#else
        p.entry = fn_pDispatchServerFunc_inv;
#endif
        ret = fn_iStartServer_inv(&p);
    }
    return ret;
}

int start(void)
{
    int ret = -1;

#ifdef PC_PORT
    if (fn_iInitSndIop_inv() >= 0)
        ret = 0;
#else
    loadcore_5_inv();
    if (intrman_9_inv() == 0) {
        if (fn_iInitSndIop_inv() >= 0)
            ret = 0;
    }
#endif
    return ret;
}
