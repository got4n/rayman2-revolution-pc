#ifndef SNDPLAYSAMPLE_INV_H
#define SNDPLAYSAMPLE_INV_H

#include "snditbase_inv.h"

typedef struct {
    long bAllocated : 1;
    long bStereo : 1;
    long bStarted : 1;
    long bDesallocating : 1;
    long bReady : 1;
    long bLeftStarted : 1;
    long bRightStarted : 1;
    long bLeftStopSent : 1;
    long bRightStopSent : 1;
    long bLeftReady : 1;
    long bRightReady : 1;
    long bLeftStopped : 1;
    long bRightStopped : 1;
    long bLeftEnd : 1;
    long bRightEnd : 1;
    long bCallbackSent : 1;
    long bPaused : 1;
    long bInNotify : 1;
} tdstPlaySampleDataFlags_inv;

typedef struct {
    int lInterface;
    tdstPlaySampleDataFlags_inv f;
    Desc_inv desc;
    Quad_inv stPendingBlock;
    int lCallbackArg;
    Iface_inv *pstSpu;
    void *hLeft;
    void *hRight;
    int x4c;
    tdstPlayParam_inv stParam;
} tdstPlaySampleData_inv;

#endif
