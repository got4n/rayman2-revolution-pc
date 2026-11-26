#ifndef REVO_PO_PO_H
#define REVO_PO_PO_H

#include "ps2/types.h"
#include "COL/COL.h"

typedef struct PO_tdstPhysicalObject_ {
    struct GeometricObjectView* _hVisualSet;
    void* _hCollideSet;
    COL_tdstBoundingSphere_* _hVisualBoundingVolume;
} PO_tdstPhysicalObject_;

#endif
