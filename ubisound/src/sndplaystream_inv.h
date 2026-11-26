#ifndef SNDPLAYSTREAM_INV_H
#define SNDPLAYSTREAM_INV_H

#include "snditbase_inv.h"

typedef struct {
    long bAllocated : 1;
    long bStarted : 1;
    long bDesallocating : 1;
    long bChannelStarted : 1;
    long bSpuStarted : 1;
    long bStopping : 1;
    long b6 : 1;
    long bRestartChannel : 1;
    long bRestartSpu : 1;
    long bCallbackSent : 1;
    long bPaused : 1;
    long bInNotify : 1;
    long bChannelReady : 1;
    long bSpuReady : 1;
    long bChannelStopped : 1;
    long bSpuStopped : 1;
    long bNoNextBlock : 1;
    long bSpuEndReached : 1;
    long bFading : 1;
    long bFadeDone : 1;
    long bFadeActive : 1;
    long bBlockChained : 1;
} tdstPlayStreamDataFlags_inv;

typedef struct {
    int lInterface;
    tdstPlayStreamDataFlags_inv f;
    Desc_inv desc;
    Quad_inv stPendingBlock;
    Quad_inv stFadeBlock;
    int lCallbackArg;
    int u32FadeLength;
    int lChannelFadePos;
    int lSpuFadePos;
    Iface_inv *pstChannel;
    void *hChannel;
    Iface_inv *pstSpu;
    void *hSpu;
    int lPos;
    tdstPlayParam_inv stParam;
} tdstPlayStreamData_inv;

#endif
