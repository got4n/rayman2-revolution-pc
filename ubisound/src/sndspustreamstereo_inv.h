#ifndef SNDSPUSTREAMSTEREO_INV_H
#define SNDSPUSTREAMSTEREO_INV_H

#include "snditbase_inv.h"

typedef struct {
    long bAllocated : 1;
    long b1 : 1;
    long b2 : 1;
    long b3 : 1;
    long b4 : 1;
    long b5 : 1;
    long b6 : 1;
    long b7 : 1;
    long b8 : 1;
    long b9 : 1;
    long b10 : 1;
    long b11 : 1;
    long b12 : 1;
    long b13 : 1;
} tdstSpuStreamStereoDataFlags_inv;

typedef struct {
    tdstSpuStreamStereoDataFlags_inv f;
    int lInterface;
    Iface_inv *pstOwner;
    void *hOwner;
    Iface_inv *pstSpu;
    void *hLeft;
    void *hRight;
} tdstSpuStreamStereoData_inv;

#endif
