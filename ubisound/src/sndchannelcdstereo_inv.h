#ifndef SNDCHANNELCDSTEREO_INV_H
#define SNDCHANNELCDSTEREO_INV_H

#include "snditbase_inv.h"
#include "sndiopbuf_inv.h"

typedef struct {
    long bAllocated : 1;
    long bStarted : 1;
    long b2 : 1;
    long bReady : 1;
    long b4 : 1;
    long bStopping : 1;
    long bStopped : 1;
    long bFillLeft : 1;
    long bReadyNotified : 1;
    long bStopNotified : 1;
    long b10 : 1;
    long bEndReached : 1;
    long bReadLeft : 1;
} tdstChannelCDStereoDataFlags_inv;

typedef struct {
    int lInterface;
    int u32ReadSize;
    char *pReadBuf;
    tdstChannelCDStereoDataFlags_inv f;
    Iface_inv *pstOwner;
    void *hOwner;
    Iface_inv *pstChannel;
    void *hChannel;
    Iface_inv *pstSpu;
    int hSpuLeft;
    int hSpuRight;
    int u32BlockSize;
    int u32HalfLength;
    tdstRingBuffer_inv stLeft;
    tdstRingBuffer_inv stRight;
    char a_ucLeft[0x2000];
    char a_ucRight[0x2000];
} tdstChannelCDStereoData_inv;

#endif
