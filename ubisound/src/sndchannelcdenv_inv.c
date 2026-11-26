typedef int (*Fn_inv)();

typedef struct {
    unsigned int pad0 : 5;
    long b5 : 1;
    long b6 : 1;
    unsigned int pad7 : 1;
    long b8 : 1;
} Flags_inv;

typedef struct {
    char pad0[8];
    signed char c8;
    char pad9[11];
    char x20[1];
} Req_inv;

extern char g_stTdstChannelCDMngr_inv[];

extern int fn_iConstructTdstChannelCDMngr_inv(void *state, void *table);
extern int fn_iDestructTdstChannelCDMngr_inv(void *state);
extern int fn_iGetNumFreeVoiceTdstChannelCDMngr_inv(void *state);
extern int fn_u32AllocateVoiceTdstChannelCDMngr_inv(void *state, void *p);
extern int fn_u32DesallocateVoiceTdstChannelCDMngr_inv(void *state, int a);
extern int fn_bIsAllocatedTdstChannelCDMngr_inv(void *state, int a);
extern int fn_iReadTdstChannelCDMngr_inv(void *state, int a, int b, int c, int d);
extern int fn_iStartTdstChannelCDMngr_inv(void *state, int a, int b);
extern int fn_iStopTdstChannelCDMngr_inv(void *state, int a);
extern int fn_iSetOwnerTdstChannelCDMngr_inv(void *state, int a, int b);
extern int fn_iConnectTdstChannelCDMngr_inv(void *state, int a, int b);
extern int fn_u32ReallocateVoiceTdstChannelCDMngr_inv(void *state, int a, void *p);
extern int fn_iOperateCallTdstChannelCDMngr_inv(void *state);
extern int fn_iGetPosTdstChannelCDMngr_inv(void *state, int a, int b);

Fn_inv *fn_pstGetChannelCDInterface_inv(void);

int fn_iInitChannelCDEnv_inv(void)
{
    return fn_iConstructTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, fn_pstGetChannelCDInterface_inv());
}

int fn_iDesInitChannelCDEnv_inv(void)
{
    return fn_iDestructTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv);
}

int fn_iGetNumFreeVoiceChannelCDEnv_inv(void)
{
    return fn_iGetNumFreeVoiceTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv);
}

int fn_bTestFormatChannelCDEnv_inv(Flags_inv *f, int mode)
{
    int r = f->b5;
    r = f->b6 && r;
    r = !f->b8 && r;
    return mode == 1 && r;
}

int fn_u32AllocateVoiceChannelCDEnv_inv(Req_inv *req)
{
    int r = 0;
    int n = req->c8;
    if (fn_iGetNumFreeVoiceTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv) >= n)
        r = fn_u32AllocateVoiceTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, req->x20);
    return r;
}

int fn_u32DesallocateVoiceChannelCDEnv_inv(int a)
{
    return fn_u32DesallocateVoiceTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a);
}

int fn_bIsAllocatedChannelCDEnv_inv(int a)
{
    return fn_bIsAllocatedTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a);
}

int fn_iReadChannelCDEnv_inv(int a, int b, int c, int d)
{
    return fn_iReadTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a, b, c, d);
}

int fn_iStartChannelCDEnv_inv(int a, int b)
{
    return fn_iStartTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a, b);
}

int fn_iStopChannelCDEnv_inv(int a)
{
    return fn_iStopTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a);
}

int fn_iSetOwnerChannelCDEnv_inv(int a, int b)
{
    return fn_iSetOwnerTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a, b);
}

int fn_iConnectChannelCDEnv_inv(int a, int b)
{
    return fn_iConnectTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a, b);
}

int fn_u32ReallocateVoiceChannelCDEnv_inv(int a, Req_inv *req)
{
    return fn_u32ReallocateVoiceTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a, req->x20);
}

int fn_iGetNumChildChannelCDEnv_inv(void)
{
    return 0;
}

int fn_iGetChildChannelCDEnv_inv(void)
{
    return -1;
}

int fn_iNotifyChannelCDEnv_inv(void)
{
    return 0;
}

int fn_iOperateCallTdstChannelCDEnv_inv(void)
{
    return fn_iOperateCallTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv);
}

int fn_iGetPosChannelCDEnv_inv(int a, int b)
{
    return fn_iGetPosTdstChannelCDMngr_inv(g_stTdstChannelCDMngr_inv, a, b);
}

static const Fn_inv a_pfnTdstChannelCDInterface_inv[18] = {
    fn_iInitChannelCDEnv_inv, fn_iDesInitChannelCDEnv_inv, fn_iGetNumFreeVoiceChannelCDEnv_inv, fn_bTestFormatChannelCDEnv_inv,
    fn_u32AllocateVoiceChannelCDEnv_inv, fn_u32ReallocateVoiceChannelCDEnv_inv, fn_u32DesallocateVoiceChannelCDEnv_inv, fn_iSetOwnerChannelCDEnv_inv,
    fn_iConnectChannelCDEnv_inv, fn_iGetNumChildChannelCDEnv_inv, fn_iGetChildChannelCDEnv_inv, fn_bIsAllocatedChannelCDEnv_inv,
    fn_iNotifyChannelCDEnv_inv, fn_iOperateCallTdstChannelCDEnv_inv, fn_iReadChannelCDEnv_inv, fn_iStartChannelCDEnv_inv,
    fn_iStopChannelCDEnv_inv, fn_iGetPosChannelCDEnv_inv,
};

Fn_inv *fn_pstGetChannelCDInterface_inv(void)
{
    return (Fn_inv *)a_pfnTdstChannelCDInterface_inv;
}
