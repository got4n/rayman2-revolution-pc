#ifndef SNDCHANNELMEMMONO_INV_H
#define SNDCHANNELMEMMONO_INV_H

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
} tdstChannelMEMMonoDataFlags_inv;

typedef struct {
    tdstChannelMEMMonoDataFlags_inv f;
    Iface_inv *pstOwner;
    void *hOwner;
    unsigned int u32Pos;
    unsigned int u32Total;
    unsigned int u32Size;
    char *pData;
} tdstChannelMEMMonoData_inv;

#endif
