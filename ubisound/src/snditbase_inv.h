#ifndef SNDITBASE_INV_H
#define SNDITBASE_INV_H

typedef struct Iface_inv Iface_inv;
struct Iface_inv {
    int (*Init)();
    int (*DesInit)();
    int (*GetNumFreeVoice)(void);
    int (*TestFormat)();
    void *(*AllocateVoice)(void *desc);
    void *(*ReallocateVoice)(void *h, void *desc);
    int (*DesallocateVoice)(void *h);
    int (*SetOwner)(void *h, void *owner);
    int (*Connect)(void *h, void *arg);
    int (*GetNumChild)(void *h);
    int (*GetChild)(void *h, void *out, int idx);
    int (*IsAllocated)();
    int (*Notify)(void *h, int event, void *from, int arg);
    int (*OperateCall)();
    union {
        struct {
            int (*Read)();
            int (*Start)();
            int (*Stop)();
            int (*GetPos)(void *h, int *out);
        } channel;
        struct {
            int (*Start)();
            int (*Stop)();
            int (*SetParam)();
            int (*GetPos)(void *h, int *out);
            int (*Pause)(void *h);
            int (*Resume)(void *h);
        } spu;
        struct {
            int (*Start)();
            int (*Stop)();
            int (*SetParam)();
            int (*PushNextBlock)();
            int (*SetCallbackArg)();
            int (*GetPos)();
            int (*Pause)();
            int (*Resume)();
        } play;
    } u;
};

typedef struct {
    long bParamPitch : 1;
    long bParamVolume : 1;
    long bParamPan : 1;
    long b3 : 1;
    long b4 : 1;
    long bStream : 1;
    long bFromCD : 1;
    long b7 : 1;
    long bInterleaved : 1;
} DescFlags_inv;

typedef struct {
    DescFlags_inv f;
    int lFormat;
    signed char cNbChannels;
    unsigned char x9;
    unsigned short uwRate;
    int lVolume;
    int lStartPos;
} DescHead_inv;

typedef struct {
    int lPitch;
    unsigned char ucVolume;
    unsigned char ucPan;
    unsigned char c6;
    unsigned char c7;
} tdstPlayParam_inv;

typedef struct {
    int pitch;
    unsigned int left;
    unsigned int right;
} tdstOutParam_inv;

typedef enum {
    E_SNDITBASE_ENDREACHED,
    E_SNDITBASE_READY_inv,
    E_SNDITBASE_STOPPED_inv,
    E_SNDITBASE_BLOCKREAD_inv
} SndItBaseEvent_inv;

typedef struct {
    int w0;
    int w1;
    int w2;
    int w3;
} Quad_inv;

typedef struct {
    unsigned int u32Size;
    void *pvVag;
} SampleChan_inv;

typedef struct {
    SampleChan_inv stLeft;
    SampleChan_inv stRight;
} SampleMemory_inv;

typedef struct {
    DescHead_inv h;
    union {
        Quad_inv a;
        SampleMemory_inv b;
    } u;
} Desc_inv;

typedef struct {
    int kind;
    void *h;
    int lInterface;
} Msg_inv;

typedef struct {
    int kind;
    void *h;
    Iface_inv *iface;
} Info_inv;

static inline void CopyQuad_inv(Quad_inv *d, Quad_inv *s)
{
    d->w0 = s->w0;
    d->w1 = s->w1;
    d->w2 = s->w2;
    d->w3 = s->w3;
}

static inline void CopyHead_inv(DescHead_inv *dst, DescHead_inv *src)
{
    dst->f.bParamPitch = src->f.bParamPitch;
    dst->f.bParamVolume = src->f.bParamVolume;
    dst->f.bParamPan = src->f.bParamPan;
    dst->f.b3 = src->f.b3;
    dst->f.b4 = src->f.b4;
    dst->f.bStream = src->f.bStream;
    dst->f.bFromCD = src->f.bFromCD;
    dst->f.b7 = src->f.b7;
    dst->f.bInterleaved = src->f.bInterleaved;
    dst->lFormat = src->lFormat;
    dst->cNbChannels = src->cNbChannels;
    dst->x9 = src->x9;
    dst->uwRate = src->uwRate;
    dst->lVolume = src->lVolume;
    dst->lStartPos = src->lStartPos;
}

static inline void ClearHead_inv(DescHead_inv *d)
{
    d->f.bParamPitch = 0;
    d->f.bParamVolume = 0;
    d->f.bParamPan = 0;
    d->f.b3 = 0;
    d->f.b4 = 0;
    d->f.bStream = 0;
    d->f.bFromCD = 0;
    d->f.b7 = 0;
    d->f.bInterleaved = 0;
    d->lFormat = 0;
    d->cNbChannels = 0;
    d->x9 = 0;
    d->uwRate = 0;
    d->lVolume = 0;
    d->lStartPos = 0;
}

static inline void CopyDesc_inv(Desc_inv *dst, Desc_inv *src)
{
    CopyHead_inv(&dst->h, &src->h);
    if (src->h.f.bStream < 0 && src->h.f.bFromCD < 0) {
        CopyQuad_inv(&dst->u.a, &src->u.a);
    } else {
        dst->u.b.stLeft.u32Size = src->u.b.stLeft.u32Size;
        dst->u.b.stLeft.pvVag = src->u.b.stLeft.pvVag;
        dst->u.b.stRight.u32Size = src->u.b.stRight.u32Size;
        dst->u.b.stRight.pvVag = src->u.b.stRight.pvVag;
    }
}

static inline void InitMsg_inv(Msg_inv *m, int kind, void *obj, int x4)
{
    m->kind = kind;
    m->h = obj;
    m->lInterface = x4;
}

extern void fn_vSplitTdstDesc_inv(Desc_inv *left, Desc_inv *right, Desc_inv *src);
extern Iface_inv *fn_pstFindInterface_inv(int kind, void *desc, int c, int d);
extern void *sysclib_14_inv(void *p, int c, int n);

#endif
