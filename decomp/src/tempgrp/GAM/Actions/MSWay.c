#include "common.h"

#include "GAM/GAM.h"
#include "AI/AI.h"

tdstMSWay_* fn_h_MSWayRealAlloc(tdstEngineObject_* object);
void MSWay_fn_vInitPath(tdstMSWay_* way);

void fn_vMSWayAlloc(tdstEngineObject_* p_stObject)
{
    p_stObject->h_MSWay = fn_h_MSWayRealAlloc(p_stObject);
}

tdstMSWay_* fn_h_MSWayRealAlloc(tdstEngineObject_* object)
{
    tdstMSWay_* way = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstMSWay_));

    MSWay_fn_vInitPath(way);
    return way;
}

#ifdef PC_PORT
void fn_vMSWayInit(tdstEngineObject_* object, void* unused)
#else
void fn_vMSWayInit(tdstEngineObject_* object)
#endif
{
    MSWay_fn_vInitPath(object->h_MSWay);
}

void fn_vMSWayDesinit(tdstEngineObject_* object)
{
    MSWay_fn_vDesinitPath(object->h_MSWay);
}

void fn_vMSWayCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    tdstMSWay_* copy;
    tdstMSWay_* original;

    MSWay_fn_vDesinitPath(_p_stObjectDest->h_MSWay);
    copy = _p_stObjectDest->h_MSWay;
    original = _p_stObjectSrc->h_MSWay;
    *copy = *original;
    MSWay_fn_vInitPath(_p_stObjectDest->h_MSWay);
}

void MSWay_fn_vInitPath(tdstMSWay_* way)
{
    MSWay_fn_vDesinitPath(way);
}

void MSWay_fn_vDesinitPath(tdstMSWay_* hWay)
{
    if (hWay == NULL) {
        return;
    }
    if (hWay->m_hPath != NULL) {
        WPG_fn_vDestroy(hWay->m_hPath);
    }
    hWay->m_hPath = NULL;
    hWay->m_lIndex = -1;
    hWay->m_bIsCircular = 0;
}

void MSWay_fn_vSetPath(tdstMSWay_* hWay, WP_tdstGraph_* hPath, s8 _bIsCircular)
{
    if (hWay != NULL && hPath != NULL) {
        if (hWay->m_hPath != NULL) {
            WPG_fn_vDestroy(hWay->m_hPath);
        }
        hWay->m_bIsCircular = _bIsCircular;
        hWay->m_hPath = hPath;
        hWay->m_lIndex = 0;
    }
}

s32 MSWay_fn_lGetNumberWayPointInPath(tdstMSWay_* hWay)
{
    s32 count = -1;
    if (hWay != 0) {
        WP_tdstGraph_* path = hWay->m_hPath;
        if (path != 0) {
            count = path->m_hListOfNode.lNumberOfElementsDyn;
        }
    }
    return count;
}

s32 MSWay_fn_lGetCurrentIndexInPath(tdstMSWay_* hWay)
{
    if (hWay != 0) {
        if (hWay->m_hPath != 0) {
            return hWay->m_lIndex;
        }
    }
    return -1;
}

s32 MSWay_fn_lGetLastIndexInPath(tdstMSWay_* hWay)
{
    if (hWay != 0) {
        WP_tdstGraph_* p = hWay->m_hPath;
        if (p != 0) {
            return p->m_hListOfNode.lNumberOfElementsDyn - 1;
        }
    }
    return -1;
}

s32 MSWay_fn_lGetFirstIndexInPath(tdstMSWay_* hWay)
{
    if (hWay != NULL && hWay->m_hPath != NULL) {
        return 0;
    }
    return -1;
}

s32 MSWay_fn_lSetCurrentIndexInPath(tdstMSWay_* hWay, s32 _lIndex)
{
    WP_tdstGraph_* data;
    s32 count;

    if (hWay != NULL) {
        data = hWay->m_hPath;
        if (data != NULL) {
            count = data->m_hListOfNode.lNumberOfElementsDyn;
            if (hWay->m_bIsCircular != 0) {
                _lIndex = _lIndex % count;
                if (_lIndex < 0) {
                    _lIndex = _lIndex + count;
                }
            }
            if (_lIndex >= count) {
                _lIndex = count - 1;
            }
            if (_lIndex < 0) {
                _lIndex = 0;
            }
            hWay->m_lIndex = _lIndex;
            return _lIndex;
        }
    }
    return -1;
}

s32 MSWay_fn_lLookForWPInMSWay(tdstMSWay_* obj, WP_tdstWayPoint_* _hWayPoint)
{
    WP_tdstGraph_* data;
    WP_tdstGraphNode_* node;
    s32 index;
    s32 count;

    if (obj != NULL) {
        data = obj->m_hPath;
        if (data != NULL) {
            node = data->m_hListOfNode.hFirstElementDyn;
            count = data->m_hListOfNode.lNumberOfElementsDyn;
            if (node != NULL) {
                index = count - 1;
                do {
                    if (_hWayPoint == node->m_hWayPoint) {
                        return index;
                    }
                    node = node->hNextBrotherDyn;
                    index = index - 1;
                } while (node != NULL);
            }
        }
    }
    return -1;
}

s32 MSWay_fn_lIncrementCurrentIndexInPath(tdstMSWay_* hWay, s32 iStep)
{
    s32 count;
    s32 index;

    if (hWay == NULL) {
        return -1;
    }
    if (hWay->m_hPath != NULL) {
        count = hWay->m_hPath->m_hListOfNode.lNumberOfElementsDyn;
        if (hWay->m_bIsCircular != 0) {
            hWay->m_lIndex = (hWay->m_lIndex + iStep) % count;
            if (hWay->m_lIndex < 0) {
                hWay->m_lIndex = hWay->m_lIndex + count;
            }
            return hWay->m_lIndex;
        }
        index = hWay->m_lIndex + iStep;
        if (index >= 0 && index < count) {
            hWay->m_lIndex = index;
            return index;
        }
    }
    return -1;
}

WP_tdstWayPoint_* MSWay_fn_hGetWayPointOfPath(tdstMSWay_* hWay, s32 iIndex)
{
    WP_tdstGraph_* data;
    WP_tdstGraphNode_* node;
    s32 lNbOfElts;
    s32 i;

    if (hWay != NULL) {
        data = hWay->m_hPath;
        if (data != NULL) {
            lNbOfElts = data->m_hListOfNode.lNumberOfElementsDyn;
            if (hWay->m_bIsCircular != 0) {
                iIndex = iIndex % lNbOfElts;
                if (iIndex < 0) {
                    iIndex = iIndex + lNbOfElts;
                }
            } else if (iIndex < 0 || iIndex >= lNbOfElts) {
                return 0;
            }
            iIndex = lNbOfElts - iIndex;
            i = iIndex - 1;
            node = data->m_hListOfNode.hFirstElementDyn;
            if (i > 0) {
                do {
                    i = i - 1;
                    node = node->hNextBrotherDyn;
                } while (i != 0);
            }
            return node->m_hWayPoint;
        }
    }
    return 0;
}

s32 MSWay_fn_ulGetCapsForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex)
{
    s32 count;
    WP_tdstWayPoint_* currentWaypoint;
    WP_tdstWayPoint_* nextWaypoint;
    u32 value;
    s32 result;

    if (hWay != NULL) {
        if (hWay->m_hPath != NULL) {
            count = hWay->m_hPath->m_hListOfNode.lNumberOfElementsDyn;
            if (hWay->m_bIsCircular != 0) {
                iIndex = iIndex % count;
                if (iIndex < 0) {
                    iIndex = iIndex + count;
                }
            }
            if (iIndex >= 0) {
                if (iIndex < count) {
                    currentWaypoint = MSWay_fn_hGetWayPointOfPath(hWay, iIndex);
                    nextWaypoint = MSWay_fn_hGetWayPointOfPath(hWay, iIndex + 1);
                    if (nextWaypoint != 0) {
                        result = WPG_fn_lGetLinkCapabilities(hWay->m_hPath, currentWaypoint,
                                                              nextWaypoint, &value);
                        if (result != -1) {
                            return value;
                        }
                    }
                }
            }
        } else {
            return 0;
        }
    }
    return 0;
}

s32 MSWay_fn_lChangeCapsForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex, u32 ulNewCaps,
                                              s32 lValue)
{
    s32 count;
    WP_tdstWayPoint_* id0;
    WP_tdstWayPoint_* id1;

    if (hWay != NULL) {
        if (hWay->m_hPath != NULL) {
            count = hWay->m_hPath->m_hListOfNode.lNumberOfElementsDyn;
            if (hWay->m_bIsCircular != 0) {
                iIndex = iIndex % count;
                if (iIndex < 0) {
                    iIndex = iIndex + count;
                }
            }
            if (iIndex >= 0) {
                if (iIndex < count) {
                    id0 = MSWay_fn_hGetWayPointOfPath(hWay, iIndex);
                    id1 = MSWay_fn_hGetWayPointOfPath(hWay, iIndex + 1);
                    if (id1 != 0) {
                        return WPG_fn_lChangeLinkCapabilities(hWay->m_hPath, id0, id1, ulNewCaps,
                                                              lValue);
                    }
                }
            }
            return -1;
        }
        return -1;
    }
    return -1;
}

s32 MSWay_fn_lGetWeightForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex)
{
    s32 lNbOfElts;
    WP_tdstWayPoint_* lValue;
    WP_tdstWayPoint_* nextWaypoint;
    s32 value;
    s32 result;

    if (hWay != NULL) {
        if (hWay->m_hPath != NULL) {
            lNbOfElts = hWay->m_hPath->m_hListOfNode.lNumberOfElementsDyn;
            if (hWay->m_bIsCircular != 0) {
                iIndex = iIndex % lNbOfElts;
                if (iIndex < 0) {
                    iIndex = iIndex + lNbOfElts;
                }
            }
            if (iIndex >= 0) {
                if (iIndex < lNbOfElts) {
                    lValue = MSWay_fn_hGetWayPointOfPath(hWay, iIndex);
                    nextWaypoint = MSWay_fn_hGetWayPointOfPath(hWay, iIndex + 1);
                    if (nextWaypoint != 0) {
                        result = WPG_fn_lGetLinkWeight(hWay->m_hPath, lValue, nextWaypoint,
                                                        &value);
                        if (result != -1) {
                            return value;
                        }
                    }
                }
            }
        } else {
            return 0;
        }
    }
    return 0;
}

s32 MSWay_fn_lChangeWeightForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex, s32 lWeight)
{
    s32 count;
    WP_tdstWayPoint_* id0;
    WP_tdstWayPoint_* id1;

    if (hWay != NULL) {
        if (hWay->m_hPath != NULL) {
            count = hWay->m_hPath->m_hListOfNode.lNumberOfElementsDyn;
            if (hWay->m_bIsCircular != 0) {
                iIndex = iIndex % count;
                if (iIndex < 0) {
                    iIndex = iIndex + count;
                }
            }
            if (iIndex >= 0) {
                if (iIndex < count) {
                    id0 = MSWay_fn_hGetWayPointOfPath(hWay, iIndex);
                    id1 = MSWay_fn_hGetWayPointOfPath(hWay, iIndex + 1);
                    if (id1 != 0) {
                        return WPG_fn_lChangeLinkWeight(hWay->m_hPath, id0, id1, lWeight);
                    }
                }
            }
            return -1;
        }
        return -1;
    }
    return -1;
}
