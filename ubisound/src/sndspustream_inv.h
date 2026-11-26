#ifndef SNDSPUSTREAM_INV_H
#define SNDSPUSTREAM_INV_H

#include "snditbase_inv.h"

typedef struct {
    int pitch;
    int left;
    int right;
} Trip_inv;

typedef struct {
    long bAllocated : 1;
    long bDesallocating : 1;
    long b2 : 1;
    long bStarted : 1;
    long bReady : 1;
    long bStopped : 1;
    long b6 : 1;
    long b7 : 1;
    long bEndReached : 1;
    long bStopRequested : 1;
    long bReadyNotified : 1;
    long bStopNotified : 1;
    long bEndNotified : 1;
    long bPaused : 1;
} tdstSpuStreamDataFlags_inv;

typedef struct {
    tdstSpuStreamDataFlags_inv f;
    Iface_inv *pstOwner;
    void *hOwner;
    int u32Fetched;
    unsigned int u32LastPos;
    int lVoice;
    int lCore;
    unsigned short x1c;
    unsigned short uwBlockTicks;
    int lBufOffset;
    int lHalfSize;
    void *hChannel;
    Iface_inv *pstChannel;
    Trip_inv stParam;
    int lBufSize;
    int lSpuAddr;
} tdstSpuStreamData_inv;

typedef struct {
    int size;
    unsigned char *data;
} Buf_inv;

#endif
