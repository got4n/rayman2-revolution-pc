#include "common.h"

#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "WP/WP.h"

void WP_fnv_WayPoint_GetVertex(WP_tdstWayPoint_* _hWayPoint, MTH3D_tdstVector_* _pstVector)
{
    *_pstVector = _hWayPoint->m_stVertex;
}

f32 WP_fnx_WayPoint_GetRadius(WP_tdstWayPoint_* _hWayPoint)
{
    return _hWayPoint->m_xRadius;
}

void func_002024D8(void)
{
}

void WP_fnv_WayPoint_ComputeLocation(WP_tdstWayPoint_* _hWayPoint, MTH3D_tdstVector_* _p_stVertex)
{
    MTH3D_tdstVector_ position;

    if (_hWayPoint->m_hSuperObject != NULL) {
        position = _hWayPoint->m_stVertex;
        POS_fn_vMulMatrixVertex(_p_stVertex, _hWayPoint->m_hSuperObject->gu_hGlobalMatrix, &position);
    } else {
        *_p_stVertex = _hWayPoint->m_stVertex;
    }
}

void func_00202548(void)
{
}
