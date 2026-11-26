#ifndef SNDSPUSAMPLE_INV_H
#define SNDSPUSAMPLE_INV_H

#include "snditbase_inv.h"

typedef struct {
    long bAllocated : 1;
    long bStarted : 1;
    long bReady : 1;
    long bEndReached : 1;
    long bStopped : 1;
    long bReadyNotified : 1;
    long bStopNotified : 1;
    long bEndNotified : 1;
    long bPaused : 1;
} tdstSpuSampleDataFlags_inv;

typedef struct {
    int pitch;
    int left;
    int right;
} ElemCTrip_inv;

typedef struct {
    int lVoice;
    int lCore;
    tdstSpuSampleDataFlags_inv f;
    Iface_inv *pstOwner;
    void *hOwner;
    int lRate;
    ElemCTrip_inv stParam;
    int x24;
    int lSpuAddr;
} tdstSpuSampleData_inv;

#endif
