#ifndef SNDCHANNELCD_INV_H
#define SNDCHANNELCD_INV_H

#include "snditbase_inv.h"
#include "sndiopbuf_inv.h"

typedef struct {
    long bAllocated : 1;
    long bStarted : 1;
    long bFull : 1;
    long b3 : 1;
    long bStopped : 1;
    long bEndReached : 1;
    long bSeeked : 1;
    long bReadPending : 1;
    long bReadyNotified : 1;
    long bStopNotified : 1;
    long bEndNotified : 1;
} tdstChannelCDDataFlags_inv;

typedef struct tdstChannelCDData_inv {
    unsigned int u32BufSize;
    char *pBuf;
    unsigned int u32HalfSize;
    int lSema;
    tdstChannelCDDataFlags_inv f;
    tdstFifo_inv stFifo;
    Iface_inv *pstOwner;
    void *hOwner;
    int lSector;
    int lSeekSector;
    unsigned int u32Pos;
    unsigned int u32Total;
    unsigned int u32SectorOffset;
    unsigned int u32Length;
    int lStartSector;
} tdstChannelCDData_inv;

#endif
