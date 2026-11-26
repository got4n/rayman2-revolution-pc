#include "common.h"

#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"

#define DYNAMICS_ADVANCED 0x2
#define DYNAMICS_COMPLETE 0x4
#define DYNAMICS_HAS_REPORT 0x4000

void DNM_fn_vDynamicsInitDefault(DNM_stDynamics* p_stDynamic)
{
    u32 flags;
    u32 preservedFlags;
    u32 updatedFlags;
    DNM_stReport_* report;

    memset(&g_stReport, 0, sizeof(g_stReport));
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags = 0;
    flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
    preservedFlags = flags & 0x4400F;
    report = NULL;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = preservedFlags;
    if (flags & DYNAMICS_HAS_REPORT) {
        report = p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport;
        DNM_p_stReportDefaultInit(report);
    }

    flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
    if (flags & DYNAMICS_COMPLETE) {
        memset(p_stDynamic, 0, sizeof(DNM_stDynamics));
    } else if (flags & DYNAMICS_ADVANCED) {
        memset(p_stDynamic, 0, sizeof(DNM_stDynamicsBase_) + sizeof(DNM_stDynamicsAdvancedBlock));
    } else {
        memset(p_stDynamic, 0, sizeof(DNM_stDynamicsBase_));
    }
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = preservedFlags;
    if (report != NULL) {
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport = report;
    }

    updatedFlags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
    p_stDynamic->m_tdstDynamicsBaseBlock.temporalCurveType = 1;
    updatedFlags |= 0x80;
    p_stDynamic->m_tdstDynamicsBaseBlock.collisionPolicy = 0;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = updatedFlags;
    if (updatedFlags & DYNAMICS_ADVANCED) {
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamFactor = 1.0f;
    }
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_COMPLETE) {
        p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform = 0;
    }
    memset(&g_stReport, 0, sizeof(g_stReport));
}

void DNM_fn_vInitBaseMechanic(DNM_stDynamics* p_stDynamic)
{
    DNM_fn_vDynamicsInitDefault(p_stDynamic);
}

void DNM_fn_vInitDynamic(DNM_stDynamics* p_stDynamic, const DNM_stMecBaseIdCard* hIdCard)
{
    u32 flags;
    u32 value;
    f32 speed;

    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x1) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x1)
                              : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x1);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x2) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x2)
                              : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x2);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x4) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x4)
                              : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x4);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x8) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x8)
                              : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x8);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x10) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x10)
                               : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x10);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x20) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x20)
                               : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x20);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x40) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x40)
                               : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x40);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
        (hIdCard->ulFlags & 0x80) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x100)
                               : (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x100);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
        (hIdCard->ulFlags & 0x100) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x200)
                                : (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x200);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
        (hIdCard->ulFlags & 0x20000) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x100000)
                                  : (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x100000);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x200) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x80)
                                : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x80);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x400) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x100)
                                : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x100);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x800) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x200)
                                : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x200);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x1000) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x400)
                                 : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x400);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x2000) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x800)
                                 : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x800);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x4000) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x1000)
                                 : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x1000);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
        (hIdCard->ulFlags & 0x8000) ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x2000)
                                 : (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x2000);

    p_stDynamic->m_tdstDynamicsBaseBlock.m_xGravity = hIdCard->m_xGravity;
    flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags | 0x10;
    speed = hIdCard->m_xSlopeLimit;
    value = flags & ~0x200000;
    value &= ~0x2000000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = value;
    p_stDynamic->m_tdstDynamicsBaseBlock.m_xSlopeLimit = speed;
    p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope =
        MTH_fn_xSqrtOpt2(1.0f / (MTH_fn_xSqrOpt1(speed) + 1.0f));

    if (flags & DYNAMICS_ADVANCED) {
        p_stDynamic->m_tdstDynamicsBaseBlock.m_xSlide = hIdCard->m_xSlide;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_xRebound = hIdCard->m_xRebound;
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaX = hIdCard->m_xInertiaX;
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaY = hIdCard->m_xInertiaY;
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaZ = hIdCard->m_xInertiaZ;
        *(MTH3D_tdstVector_*) &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stMaxSpeed =
            hIdCard->m_stMaxSpeed;
        if (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80)) {
            *(MTH3D_tdstVector_*) &p_stDynamic->m_tdstDynamicsAdvancedBlock
                 .m_stCollisionTranslation =
                *(MTH3D_tdstVector_*) &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix
                     .stMatrix.stCol_3.xX;
        }
        return;
    }
    if (flags & DYNAMICS_COMPLETE) {
        p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltIntensity = hIdCard->m_xTiltIntensity;
        p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltInertia = hIdCard->m_xTiltInertia;
        p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltOrigin = hIdCard->m_xTiltOrigin;
    }
}

void DNM_fn_vChangeIdCard(DNM_stDynamics* _p_stDynamics, DNM_stMecBaseIdCard* _hNewIdCard)
{
    if (_hNewIdCard->m_eIdentity == 1) {
        DNM_fn_vInitDynamic(_p_stDynamics, _hNewIdCard);
    }
    _p_stDynamics->m_tdstDynamicsBaseBlock.m_pCurrentIdCard = _hNewIdCard;
}

void DNM_fn_vDynamicsCopyClone(DNM_stDynamics* _p_stDst, DNM_stDynamics* _p_stSrc)
{
    DNM_stReport_* report = NULL;

    if (_p_stDst->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_HAS_REPORT) {
        report = _p_stDst->m_tdstDynamicsBaseBlock.m_pstReport;
    }

    if (_p_stSrc->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_COMPLETE) {
        *_p_stDst = *_p_stSrc;
    } else if (_p_stSrc->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_ADVANCED) {
        memcpy(_p_stDst, _p_stSrc,
               sizeof(DNM_stDynamicsBase_) + sizeof(DNM_stDynamicsAdvancedBlock));
    } else {
        _p_stDst->m_tdstDynamicsBaseBlock = _p_stSrc->m_tdstDynamicsBaseBlock;
    }

    if (report != NULL) {
        _p_stDst->m_tdstDynamicsBaseBlock.m_pstReport = report;
        if (_p_stSrc->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_HAS_REPORT) {
            *report = *_p_stSrc->m_tdstDynamicsBaseBlock.m_pstReport;
        }
    }
}
