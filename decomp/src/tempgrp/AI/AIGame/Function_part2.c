#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "COL/COL.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "PO/PO.h"
#include "SCT/SCT.h"
#include "WP/WP.h"

#include "ps2/copy_types.h"
#ifdef PC_PORT
f32 fabsf(f32 value);
#endif
#include "ps2/libc.h"
#include "ps2/r5900.h"
#include "ps2/types.h"

#include "include_asm.h"
#ifdef PC_PORT
f32 powf(f32 x, f32 y);
static f32 pc_curve_powf(f32 x, f32 ticks, s32 curve) {
    f32 r = powf(__builtin_fabsf(x), ticks);
    return (x < 0.0f && (curve == 1 || curve == 3 || curve == 5)) ? -r : r;
}
#endif
#include "SAI/SAI.h"
#include "GMT/GMT.h"
#include "GLI/GLI.h"
#include "GEO/GEO.h"
#include "AGO/AGO.h"
#include "snd/snd.h"
extern f32 flt_393D90[];
extern f32 flt_393D94[];
extern f32 flt_393D98[];
extern f32 flt_393D9C[];
extern s32 g_ulCollideTimeCounter;
extern u8 inv_TXT_resolutionName[];
extern u8 unk_383BF0[];
extern struct TXT_tdstLabel_* inv_AI_p_stTextLabels;

f32 acosf(f32);
f32 cosf(f32);
f32 fn_xExtendedDistanceBetweenPerso(HIE_tdstSuperObject_*, HIE_tdstSuperObject_*, u8);
f32 fn_xInternalDistance(f32*, s32);
f32 sinf(f32);
s32 atoi(void*);
s32 inv_SAV_fn_bLoadGameFromSlot(s32);
s32 inv_SAV_fn_bSelectOrReplaceSlot(s32);
void* inv_fn_p_stSetColorA(void*, void*, u8*);
void* inv_fn_p_stSetColorB(void*, void*, u8*);
void* inv_fn_p_stSetColorG(void*, void*, u8*);
void* inv_fn_p_stSetColorR(void*, void*, u8*);
u32 fn_lAddColor(u32, u32);
void MSWay_fn_vSetPath(void*, void*, s32);
void fn_vGetPersoZDxCenter(HIE_tdstSuperObject_*, u16, u8, u8, MTH3D_tdstVector_*);
void* inv_fn_p_stEvalRGB(void*, void*, u8*);
HIE_tdstSuperObject_* fn_hFindClosestPersoCandidate(
    HIE_tdstSuperObject_* best, f32* bestDistance, f32* bestScore, HIE_tdstSuperObject_* self,
    MTH3D_tdstVector_* position, MTH3D_tdstVector_* axis, POS_tdstMatrix_* matrix,
    HIE_tdstSuperObject_* sector, f32 weightX, f32 weightXY, f32 weightDistance, f32 slopeLimit,
    f32 sineLimit, s32 mask, u16 zone, s32 family, u16 opcode);
void* fn_hGetNearestActorInSectorWithAnyCustombit(void*, u32, s32);
void* inv_fn_p_stEvalRGBA(void*, void*, void*);

extern COL_tdstCollideObject_* fn_hGetGeometricZdxOfTypeAtIndex(u8 type, u16 index,
                                                                HIE_tdstSuperObject_* perso);

extern f32 flt_393D88[];

extern f32 flt_393D8C[];

f32 atanf(f32 value);
f32 atanf(f32 value);

extern void POS_fn_vGetYAxisVector(void* m, void* out);

extern f32 tanf(f32 x);

extern u8 byte_2542A9[];

static inline f32 inv_fn_xAnalogDivide(f32 numerator, f32 denominator)
{
    return numerator / denominator;
}

void* fn_p_stCode4VitessePadAnalogique(HIE_tdstSuperObject_* superObject,
                                       tdstNodeInterpret_* stream, tdstGetSetParam_* param)
{
    f32 xInvLength;
    f32 xValue;
    f32 xAxisZ, xAxisX;
    s32 lSpeed;
    f32 xSpeed, xZero;
    MTH3D_tdstVector_ stSpeedVector;

    if (AI_NODE_ID(stream) == eFunc_VitessePadAnalogique) {
        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float)
            xValue = param->u.xValue;
        else
            xValue = (f32) param->u.lValue;
        xAxisZ = -xValue;

        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float)
            xAxisX = -param->u.xValue;
        else
            xAxisX = -(f32) param->u.lValue;

        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float) {
            lSpeed = inv_MTH_fn_lToIntegerOrdered(param->u.xValue);
        } else {
            lSpeed = param->u.lValue;
        }

        xSpeed = (f32) lSpeed;
        xZero = 0.0f;
        MTH3D_M_vSetXYZ_inv(&stSpeedVector, xSpeed * inv_fn_xAnalogDivide(xAxisX, 100.0f), xZero,
                            xSpeed * inv_fn_xAnalogDivide(xAxisZ, 100.0f));
        xValue = MTH_fn_xSqrAddSqrAddSqrOpt2(stSpeedVector.xX, stSpeedVector.xY, stSpeedVector.xZ);
        xValue = MTH_fn_xSqrtOpt2(xValue);
        if (xValue != 0.0f) {
            f32 xNormX, xNormY, xNormZ;
            xInvLength = inv_fn_xAnalogDivide(1.0f, xValue);
            xNormX = xInvLength * stSpeedVector.xX;
            xNormY = xInvLength * stSpeedVector.xY;
            xNormZ = xInvLength * stSpeedVector.xZ;
            MTH3D_M_vSetXYZ_inv(&stSpeedVector, xNormX, xNormY, xNormZ);
        }
        inv_MTH3D_fn_vKTimesXYZ(&stSpeedVector, xSpeed, &stSpeedVector);

        param->u.stVector = stSpeedVector;
        param->ucType = E_vt_Vector;
        return stream;
    }
    return stream;
}

void* fn_p_stCode4LitCentreZDx(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                               tdstGetSetParam_* param)
{
    HIE_tdstSuperObject_* context;
    COL_tdstCollideObject_* volume;
    MTH3D_tdstVector_ value;
    MTH3D_tdstVector_ other;
    s16 index_x;
    s16 index_y;
    s32 op;
    s32 value_arg;
    s32 out_type;
    f32 zero;

    op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &context);
    stream = fn_p_stEvalTree(superObject, stream, param);

    value_arg = param->u.lValue;
    switch (op) {
    case eFunc_LitCentreZDM:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(1, (value_arg - 1) & 0xFFFF, context);
        break;
    case eFunc_LitCentreZDE:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(2, (value_arg - 1) & 0xFFFF, context);
        break;
    case eFunc_LitCentreZDD:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(0, (value_arg - 1) & 0xFFFF, context);
        break;
    default:
        return stream;
    }

    if (volume != (void*) 0) {
        switch (*volume->d_xListOfElementsTypes) {
        case 7:
            param->u.stVector = *volume->d_stListOfPoints;
            param->ucType = E_vt_Vector;
            goto done;
        case 8:
            GEO_vGetMinMaxPointOfIndexedAlignedBox(volume, 0, 0, &index_x, &index_y);
            GEO_vGetPointOfObject(volume, &value, index_x);
            GEO_vGetPointOfObject(volume, &other, index_y);
            out_type = 0x15;
            MTH3D_M_vSetXYZ_inv(&value, (value.xX + other.xX) * 0.5f, (value.xY + other.xY) * 0.5f,
                                (value.xZ + other.xZ) * 0.5f);
            param->u.stVector = value;
            goto set_type;
        default:
            break;
        }

        out_type = 0x15;
        zero = 0.0f;
        MTH3D_M_vSetXYZ_inv(&value, zero, zero, zero);
        param->u.stVector = value;
    } else {
        zero = 0.0f;
        out_type = 0x15;
        MTH3D_M_vSetXYZ_inv(&value, zero, zero, zero);
        param->u.stVector = value;
    }
set_type:
    param->ucType = (u8) out_type;
done:
    return stream;
}

tdstNodeInterpret_* fn_p_stCode4LitDimensionZDx(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                tdstNodeInterpret_* p_stTree,
                                                tdstGetSetParam_* param)
{
    HIE_tdstSuperObject_* context;
    COL_tdstCollideObject_* volume;
    MTH3D_tdstVector_ value;
    MTH3D_tdstVector_ other;
    s16 index_x;
    s16 index_y;
    f32 scale;
    f32 scalar;
    s32 op;
    s32 value_arg;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &context);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);

    value_arg = param->u.lValue;
    switch (op) {
    case eFunc_LitDimensionZDM:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(1, (value_arg - 1) & 0xFFFF, context);
        break;
    case eFunc_LitDimensionZDE:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(2, (value_arg - 1) & 0xFFFF, context);
        break;
    case eFunc_LitDimensionZDD:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(0, (value_arg - 1) & 0xFFFF, context);
        break;
    default:
        return p_stTree;
    }
    if (volume == (void*) 0)
        goto zero;

    switch (*volume->d_xListOfElementsTypes) {
    case 7:
        gu__GEO_vGetPointOfObject(&value, &scale, volume);
        scalar = scale;
        goto fill;

    case 8:
        break;

    default:
        goto zero;
    }

    GEO_vGetMinMaxPointOfIndexedAlignedBox(volume, 0, 0, &index_x, &index_y);
    GEO_vGetPointOfObject(volume, &value, index_x);
    GEO_vGetPointOfObject(volume, &other, index_y);
    MTH3D_M_vSetXYZ_inv(&value, inv_MTH_fn_xAbsolute(value.xX - other.xX),
                        inv_MTH_fn_xAbsolute(value.xY - other.xY),
                        inv_MTH_fn_xAbsolute(value.xZ - other.xZ));
    goto store;

zero:
    scalar = 0.0f;
fill:
    MTH3D_M_vSetXYZ_inv(&value, scalar, scalar, scalar);
    goto store;

store:
    param->u.stVector = value;
    param->ucType = E_vt_Vector;

    return p_stTree;
}

tdstNodeInterpret_* fn_p_stCode4LitAxeZDx(HIE_tdstSuperObject_* superObject,
                                          tdstNodeInterpret_* stream, tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ stMinPoint;
    MTH3D_tdstVector_ stMaxPoint;
    MTH3D_tdstVector_ axis;
    u16 minimum;
    u16 maximum;
    HIE_tdstSuperObject_* perso;
    s32 toWorld;
    COL_tdstCollideObject_* volume;
    f32 zero;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 xMinY;
    u16 op;
    s32 index;

    op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &perso);
    stream = fn_p_stEvalTree(superObject, stream, param);
    index = param->u.lValue;
    stream = fn_p_stEvalTree(superObject, stream, param);
    toWorld = param->u.lValue;
    switch (op) {
    case eFunc_LitAxeZDM:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(1, index - 1, perso);
        break;
    case eFunc_LitAxeZDE:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(2, index - 1, perso);
        break;
    case eFunc_LitAxeZDD:
        volume = fn_hGetGeometricZdxOfTypeAtIndex(0, index - 1, perso);
        break;
    default:
        return stream;
    }
    zero = 0.0f;
    axis.xY = zero;
    axis.xZ = zero;
    axis.xX = zero;
    if (volume != 0 && volume->d_xListOfElementsTypes[0] == 8) {
        GEO_vGetMinMaxPointOfIndexedAlignedBox(volume, 0, 0, &minimum, &maximum);
        GEO_vGetPointOfObject(volume, &stMinPoint, minimum);
        GEO_vGetPointOfObject(volume, &stMaxPoint, maximum);
        dx = stMinPoint.xX;
        dx = stMaxPoint.xX - dx;
        xMinY = stMinPoint.xY;
        dy = stMaxPoint.xY - xMinY;
        dz = stMinPoint.xZ;
        dz = stMaxPoint.xZ - dz;
        if (dy < dx && dz < dx)
            axis.xX = 1.0f;
        if (dx < dy && dz < dy)
            axis.xY = 1.0f;
        if (dx < dz && dy < dz)
            axis.xZ = 1.0f;
        if ((u32) minimum & 0x10000) {
            xMinY = xMinY + xMinY;
            axis.xY = xMinY;
        }
    } else {
        stMinPoint.xZ = stMinPoint.xY = stMinPoint.xX = 0.0f;
    }

    if (toWorld == 1)
        POS_fn_vRotateVector(&axis, superObject->gu_hGlobalMatrix, &axis);
    param->u.stVector = axis;
    param->ucType = E_vt_Vector;
    return stream;
}

tdstNodeInterpret_* fn_p_stCode4VecteurPointAxe(HIE_tdstSuperObject_* ctx,
                                                tdstNodeInterpret_* stream, tdstGetSetParam_* out)
{
    MTH3D_tdstVector_ stProj;
    MTH3D_tdstVector_ stResult;
    MTH3D_tdstVector_ stDelta;
    MTH3D_tdstVector_ stEnd;
    tdstGetSetParam_* scratch;
    MTH3D_tdstVector_* pPoint;
    MTH3D_tdstVector_* pOrig;
    MTH3D_tdstVector_* pAxis;
    u16 type;
    f32 dy, dz;

    scratch = inv_AI_parameterPoolCursor;
    type = AI_NODE_ID(stream);
    pOrig = &scratch[1].u.stVector;
    inv_AI_parameterPoolCursor = scratch + 3;
    pAxis = &scratch[2].u.stVector;
    pPoint = &scratch[0].u.stVector;
    stream = fn_p_stEvalTree(ctx, stream, scratch);
    stream = fn_p_stEvalTree(ctx, stream, scratch + 1);
    stream = fn_p_stEvalTree(ctx, stream, scratch + 2);

    switch (type) {
    case eFunc_VecteurPointSegment:
        stEnd = *pAxis;
        inv_MTH3D_fn_vSubXYZ(&stDelta, pPoint, pOrig);
        pAxis->xX = pAxis->xX - pOrig->xX;
        dy = pAxis->xY - pOrig->xY;
        pAxis->xY = dy;
        dz = pAxis->xZ - pOrig->xZ;
        pAxis->xZ = dz;
        if (__builtin_fabsf(pAxis->xX) < 0.0001f && __builtin_fabsf(dy) < 0.0001f &&
            __builtin_fabsf(dz) < 0.0001f) {
            f32 nx;
            nx = -stDelta.xX;
            dy = stDelta.xY;
            __asm__ volatile("");
            dy = -dy;
            dz = stDelta.xZ;
            dz = -dz;
            __asm__ volatile("");
            MTH3D_M_vSetXYZ_inv(&stResult, nx, dy, dz);
        } else {
            f32 x, y, z;
            f32 sq, len, inv, dot;

            x = pAxis->xX;
            y = pAxis->xY;
            z = pAxis->xZ;
            sq = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
            len = MTH_fn_xSqrtOpt2(sq);
            if (len != 0.0f) {
                inv = 1.0f / len;
                MTH3D_M_vSetXYZ_inv(pAxis, inv * x, inv * y, inv * z);
            }
            dot = inv_MTH_fn_xMulAdd3Grouped(stDelta.xX, stDelta.xY, stDelta.xZ,
                                   pAxis->xX, pAxis->xY, pAxis->xZ);
            if (dot < 0.0f) {
                MTH3D_M_vSetXYZ_inv(&stResult, inv_MTH_fn_xNegate(stDelta.xX),
                                    inv_MTH_fn_xNegate(stDelta.xY), inv_MTH_fn_xNegate(stDelta.xZ));
            } else {
                inv_MTH3D_fn_vKTimesXYZ(&stProj, dot, pAxis);
                if (len < dot) {
                    inv_MTH3D_fn_vSubXYZ(&stResult, &stEnd, pPoint);
                } else {
                    inv_MTH3D_fn_vSubXYZ(&stResult, &stProj, &stDelta);
                }
            }
        }
        break;
    case eFunc_VecteurPointAxe:
        inv_MTH3D_fn_vSubXYZ(&stDelta, pPoint, pOrig);
        if (__builtin_fabsf(scratch[2].u.stVector.xX) < 0.0001f &&
            __builtin_fabsf(({
                f32 t_ = pAxis->xY;
                __asm__ volatile("" ::: "memory");
                t_;
            })) < 0.0001f &&
            __builtin_fabsf(({
                f32 t_ = pAxis->xZ;
                __asm__ volatile("" ::: "memory");
                t_;
            })) < 0.0001f) {
            stResult.xZ = stResult.xY = stResult.xX = 0.0f;
        } else {
            f32 x, y, z;
            f32 sq, len, inv, dot;

            x = pAxis->xX;
            y = pAxis->xY;
            z = pAxis->xZ;
            sq = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
            len = MTH_fn_xSqrtOpt2(sq);
            if (len != 0.0f) {
                inv = 1.0f / len;
                MTH3D_M_vSetXYZ_inv(pAxis, inv * x, inv * y, inv * z);
            }
            dot = inv_MTH_fn_xMulAdd3Grouped(stDelta.xX, stDelta.xY, stDelta.xZ,
                                   pAxis->xX, pAxis->xY, pAxis->xZ);
            inv_MTH3D_fn_vKTimesXYZ(&stProj, dot, pAxis);
            inv_MTH3D_fn_vSubXYZ(&stResult, &stProj, &stDelta);
        }
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    out->u.stVector = stResult;
    out->ucType = E_vt_Vector;
    return stream;
}

tdstNodeInterpret_* fn_p_stCode4LitPositionZDx(HIE_tdstSuperObject_* p_SuperObjPerso,
                                               tdstNodeInterpret_* p_stTree,
                                               tdstGetSetParam_* p_stValue)
{
    typedef struct __attribute__((aligned(8))) {
        MTH3D_tdstVector_ v;
    } Code4LitPositionVectorStorage;

    Code4LitPositionVectorStorage stPoint;
    u16 uwMinIndex;
    u16 uwMaxIndex;
    HIE_tdstSuperObject_* hPerso;
    u16 sel;
    void* saved;
    COL_tdstCollideObject_* p_stZdx;
    s32 index;

    saved = p_stTree;
    p_stTree = (u8*) p_stTree - 8;
    sel = *(u16*) p_stTree;
    p_stTree = saved;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);

    index = *(s32*) p_stValue;
    switch (sel) {
    case 0xC3:
        p_stZdx = fn_hGetGeometricZdxOfTypeAtIndex(1, (u16) (index - 1), hPerso);
        break;
    case 0xC4:
        p_stZdx = fn_hGetGeometricZdxOfTypeAtIndex(2, (u16) (index - 1), hPerso);
        break;
    case 0xC5:
        p_stZdx = fn_hGetGeometricZdxOfTypeAtIndex(0, (u16) (index - 1), hPerso);
        break;
    default:
        return p_stTree;
    }

    if (p_stZdx) {
        s16 swElementType = *p_stZdx->d_xListOfElementsTypes;
        switch (swElementType) {
        case 7:
            p_stValue->u.stVector = *p_stZdx->d_stListOfPoints;
            p_stValue->ucType = E_vt_Vector;
            return p_stTree;
        case 8:
            GEO_vGetMinMaxPointOfIndexedAlignedBox(p_stZdx, 0, 0, &uwMinIndex, &uwMaxIndex);
            GEO_vGetPointOfObject(p_stZdx, &stPoint.v, (s16) uwMinIndex);
            *(MTH3D_tdstVector_*) p_stValue = stPoint.v;
            p_stValue->ucType = E_vt_Vector;
            break;
        default:
            break;
        }
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stCode4VitesseHorizontaleDuPerso(HIE_tdstSuperObject_* context,
                                                          tdstNodeInterpret_* tree,
                                                          tdstGetSetParam_* output)
{
    u16 opcode;
    HIE_tdstSuperObject_* object;
    opcode = AI_NODE_ID(tree);
    fn_vGetUltraOperatorPerso(context, &object);
    if (fn_pstGetDNMReport(object) == 0)
        return tree;
    switch (opcode) {
    case eFunc_VitesseHorizontaleDuPerso:
        output->u.lValue = 0;
        output->ucType = E_vt_Float;
        return tree;
    case eFunc_VitesseVerticaleDuPerso:
        output->u.lValue = 0;
        output->ucType = E_vt_Float;
        return tree;
    default:
        break;
    }
    return tree;
}

typedef struct TXT_tdstLabel_ {
    char szText[600];
    s32 lId;
    void* pText;
    struct TXT_tdstLabel_* pNext;
    f32 xScale;
    u8 ucUseScale;
    u8 ucPad[3];
} TXT_tdstLabel_;

s32 fn_p_stGetRadiusWP(void* p_SuperObjPerso, void* p_stTree, void* p_stValue)
{
    tdstGetSetParam_* p_stOutput;
    f32 value;
    s32 nextStream;
    WP_tdstWayPoint_* record;

    p_stOutput = p_stValue;
    value = 0.0f;
    nextStream = (s32) fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stOutput);
    record = p_stOutput->u.pvValue;
    if (record != 0) {
        value = record->m_xRadius;
    }
    p_stOutput->u.xValue = value;
    p_stOutput->ucType = E_vt_Float;
    record = (void*) nextStream;
    return (s32) record;
}

void inv_AI_fn_vClearTextLabels(u32 a0)
{
    TXT_tdstLabel_* node;

    if (a0 & 0xFF) {
        node = (*&inv_AI_p_stTextLabels);
        while (node != 0) {
            TXT_tdstLabel_* cur;
            void* arg;

            cur = node;
            arg = gp_6F74;
            node = node->pNext;
            (*&inv_AI_p_stTextLabels) = node;
            inv_MEM_fn_vFreeInHeap(arg, cur);
            node = (*&inv_AI_p_stTextLabels);
        }
    } else {
        (*&inv_AI_p_stTextLabels) = 0;
    }
}

void inv_AI_fn_vClearTextLabelsOfObject(HIE_tdstSuperObject_* key)
{
    TXT_tdstLabel_* node;
    void* nodeKey;

    node = inv_AI_p_stTextLabels;
    while (node != 0) {
        nodeKey = node->pText;
        if (nodeKey == key) {
            node->szText[0] = 0;
            node->pText = 0;
        }
        node = node->pNext;
    }
}

void inv_AI_fn_vSetTextLabelUseScale(HIE_tdstSuperObject_* key, s32 enabled)
{
    TXT_tdstLabel_* node;
    u8 enabledByte;

    node = inv_AI_p_stTextLabels;
    enabledByte = (u8) (enabled & 0xFF);
    while (node != (void*) 0) {
        if (node->pText == key) {
            node->ucUseScale = enabledByte;
            if (enabledByte != 0) {
                JFFTXT_vExtendText(node->lId, 0, node->xScale);
            } else {
                JFFTXT_vExtendText(node->lId, 0, 255.0f);
            }
        }
        node = node->pNext;
    }
}

void inv_AI_fn_vSetTextLabelScale(HIE_tdstSuperObject_* key, f32 value)
{
    TXT_tdstLabel_* node;

    node = inv_AI_p_stTextLabels;
    while (node != (void*) 0) {
        if (node->pText == key) {
            if (value > 250.0f) {
                node->xScale = 255.0f;
            } else {
                node->xScale = value;
            }
            if (node->ucUseScale != 0) {
                JFFTXT_vExtendText(node->lId, 0, node->xScale);
            } else {
                JFFTXT_vExtendText(node->lId, 0, 255.0f);
            }
        }
        node = node->pNext;
    }
}

void* fn_p_stCode4TextAffiche(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                              tdstGetSetParam_* param)
{
    void* perso = superObject;
    void* current = stream;
    u32 lReturn = 0;
    tdstEngineObject_* pEval = 0;
    HIE_tdstSuperObject_* pText = 0;
    void* pCurrent = 0;
    MTH3D_tdstVector_ stCol3;
    MTH3D_tdstVector_ stBase;
    u8 aucText[600];

    struct {
        u8 ucSet;
        u8 ucSet2;
        u8 ucRest2;
        u8 ucRest3;
        u16 uwRest4;
        u8 ucRest6;
        u8 ucRest7;
    } stBind;

    u8 aucFmt[20];

    struct {
        s8 ucSet;
        u8 ucSet2;
        u8 ucRest2;
        u8 ucRest3;
        u16 uwRest4;
        u8 ucRest6;
        u8 ucRest7;
    } stWait;

    u32 ulNow;
    u32 ulElapsed;
    u32 ulWait;
    u8* str;
    s32 iNumber = 0;
    s16 sIndex = 0;
    u8 ucMark = 1;
    HIE_tdstSuperObject_* pBind = 0;
    HIE_tdstSuperObject_* pWait;
    s32 iEntry;

    stBind.ucSet = 0xFF;
    stWait.ucSet = -1;
    stBind.uwRest4 = 13;
    stWait.uwRest4 = 13;
    stBind.ucSet2 = 0;
    stBind.ucRest2 = 0;
    pWait = 0;
    stWait.ucSet2 = 0;
    stWait.ucRest2 = 0;

    current = inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
        perso, current, param, &pBind, &stBind.ucSet, &stBind.ucSet2, &stBind.ucRest2,
        &stBind.uwRest4, &stBind.ucRest6);

    if (param->u.lValue != 0) {
        tdstEngineObject_* pLink = param->u.pvValue;

        pCurrent = pLink->h_StandardGame->h_SuperObject;
    }

    current = fn_p_stEvalTree(perso, current, param);
    pEval = (void*) param->u.lValue;

    current = fn_p_stEvalTree(perso, current, param);
    stBase = *(MTH3D_tdstVector_*) param;

    current = fn_p_stEvalTree(perso, current, param);
    str = fn_szGetStringFromTextOrStringParam(param);
    if (str == 0)
        str = "No Text";

    if (*str == '/' && *(str + 1) == '-') {
        u8* pCmd;

        pCmd = str + 2;
        switch (*pCmd) {
        case 'W':
        case 'w':
            TFMT_vUpdateSpacingTable(pCmd + 1);
            break;
        case 'Z':
        case 'z':
            TFMT_vUpdateScaleTable(pCmd + 1);
            break;
        }
        param->u.lValue = (s32) lReturn;
        param->ucType = E_vt_Integer;
        return fn_p_stSkipThisArgument(current);
    }

    inv_strcpy(aucText, str);

    {
        u8* pIn;
        u8* pOut;

        pOut = aucText;
        pIn = str;
        while (*pIn) {
            if (pIn[0] == '/') {
                if (pIn[1] == '%') {
                    switch (pIn[2]) {
                    case 'i':
                    case 'I':
                        iEntry = inv_atoi(pIn + 3);
                        sprintf(aucFmt, "%d", TEXT_gai_IntParameters[iEntry]);
                        inv_strcpy2(pOut, aucFmt);
                        pOut += strlen(aucFmt);
                        while (*pIn != ':')
                            pIn++;
                        pIn++;
                        continue;
                    case 'f':
                    case 'F':
                        iEntry = inv_atoi(pIn + 3);
                        sprintf(aucFmt, "%.3f", TEXT_gaf_FloatParameters[iEntry]);
                        inv_strcpy2(pOut, aucFmt);
                        pOut += strlen(aucFmt);
                        while (*pIn != ':')
                            pIn++;
                        pIn++;
                        continue;
                    }
                }
            }
            *pOut++ = *pIn++;
        }
        *pOut = 0;
        str = aucText;
    }
#ifdef PC_PORT
    {
        extern void inv_JFFTXT_fn_vWrapForWideBox(u8* text, s32 size, MTH3D_tdstVector_* pos);
        MTH3D_tdstVector_ stAt;

        stAt.xX = stBase.xX + 5.0f;
        stAt.xY = stBase.xY - 25.0f;
        stAt.xZ = stBase.xZ;
        inv_JFFTXT_fn_vWrapForWideBox(aucText, sizeof aucText, &stAt);
    }
#endif

    current = inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
        perso, current, param, &pWait, &stWait.ucSet, &stWait.ucSet2, &stWait.ucRest2,
        &stWait.uwRest4, &stWait.ucRest6);
    ulWait = (u32) param->u.lValue;

    if (pCurrent != 0) {
        iNumber = TFMT_ucNumberOfCurrentTextAffiche(pCurrent);
        sIndex = g_stInfoTextAffiche[iNumber].wCurrentLetterInString;
        ucMark = (u8) g_stInfoTextAffiche[iNumber].bFirstInit;
        lReturn = (u32) g_stInfoTextAffiche[iNumber].ulEvent;
        if (sIndex >= (s16) inv_strlen(str)) {
            param->ucType = E_vt_Integer;
            param->u.lValue = (s32) lReturn;
            return current;
        }
    }

    if (pCurrent == 0) {
        MTH3D_M_vSetXYZ_inv(&stCol3, 1.0f, 1.0f, 1.0f);

        if (pEval != 0) {
            if (fn_ucIsAnAlwaysObject(pEval)) {
                POS_tdstMatrix_ stMatrix;
                POS_tdstMatrix_* pMatrix = &stMatrix;
                u16 uwModel;

                uwModel = pEval->h_StandardGame->otObjectModelType;
                gu_MTH4D_M_vSetIdentityMatrix(pMatrix);
                if (stMatrix.ucType == MAT_eTransform_Identity)
                    pMatrix->ucType = MAT_eTransform_Translation;
                MTH4D_fn_vCopy3DTo4DVector(&stMatrix.stMatrix.stCol_3, &stCol3);

                pText = fn_p_stAllocateAlways(uwModel, gp_stDynamicWorld, perso, 0, pMatrix);

                if (pText != 0) {
                    tdstEngineObject_* pObject = ((HIE_tdstSuperObject_*) pText)->hLinkedObject;
                    u32 ulSaved = ulWait;
                    gu_tdst3dData_* p3dData = pObject->h_3dData;

                    p3dData->ucFlagEndState = 1;

                    iNumber = TFMT_ucSetNumber4CurrentTextAffiche();
                    ulNow = inv_fn_ulGetEngineClock();
                    g_stInfoTextAffiche[iNumber].ulTimer = ulNow;
                    {
                        u32 ulCopy = ulWait;
                        s32 lKind = 2;

                        g_stInfoTextAffiche[iNumber].ulCurrentDelay = ulCopy;
                        gp_SuperObjPersoGenerated[iNumber] = pText;

                        {
                            void* pChild = 0;

                            if (pText->lTypeOfLinkedObject == lKind)
                                pChild = pText->hLinkedObject;
                            param->u.lValue = (s32) pChild;
                        }
                    }
                    param->ucType = E_vt_Perso;
                    fn_ucSetDsgVar(
                        stBind.ucSet, stBind.ucSet2,
                        ((tdstEngineObject_*) pBind->hLinkedObject)->h_Brain->p_stMind,
                        param);

                    if (!ulWait) {
                        sIndex = -1;
                        lReturn = (u32) TFMT_wPositionModulesAccordingToText2(str, 0, -1, &ucMark,
                                                                              &sIndex, &ulWait);
                        sIndex = (s16) inv_strlen(str);
                    } else {
                        lReturn = (u32) TFMT_wPositionModulesAccordingToText2(str, 0, -1, &ucMark,
                                                                              &sIndex, &ulWait);
                    }

                    if (ulWait != ulSaved) {
                        if ((s32) ulWait < 0)
                            ulWait = (u32) (-(s32) ulWait);
                        else
                            g_stInfoTextAffiche[iNumber].ulCurrentDelay = ulWait;
                    }
                }
            }
        }
    } else {
        ulNow = g_stInfoTextAffiche[iNumber].ulTimer;
        ulElapsed = inv_fn_ulGetEngineClock() - ulNow;
        pText = pCurrent;
        ucMark = (u8) g_stInfoTextAffiche[iNumber].bFirstInit;

        if (ulElapsed >= ulWait) {
            u32 ulSaved = g_stInfoTextAffiche[iNumber].ulCurrentDelay;

            ulWait = ulSaved;
            if (!ulWait) {
                while ((sIndex < (s16) inv_strlen(str)) && (!ulWait)) {
                    lReturn = (u32) TFMT_wPositionModulesAccordingToText2(str, 0, -1, &ucMark,
                                                                          &sIndex, &ulWait);
                }
            } else {
                lReturn = (u32) TFMT_wPositionModulesAccordingToText2(str, 0, -1, &ucMark, &sIndex,
                                                                      &ulWait);
            }
            g_stInfoTextAffiche[iNumber].ulTimer = inv_fn_ulGetEngineClock();
            if (ulWait != ulSaved) {
                if ((s32) ulWait < 0)
                    ulWait = (u32) (-(s32) ulWait);
                else
                    g_stInfoTextAffiche[iNumber].ulCurrentDelay = ulWait;
            }
            g_stInfoTextAffiche[iNumber].ulTimer = inv_fn_ulGetEngineClock();
        } else {
            if (lReturn) {
                u32 ulSaved = g_stInfoTextAffiche[iNumber].ulCurrentDelay;
                u32 ulCopy = ulSaved;

                lReturn = (u32) TFMT_wPositionModulesAccordingToText2(str, 0, -1, &ucMark, &sIndex,
                                                                      &ulCopy);
                if (ulCopy != ulSaved) {
                    if ((s32) ulCopy < 0)
                        ulCopy = (u32) (-(s32) ulCopy);
                    else
                        g_stInfoTextAffiche[iNumber].ulCurrentDelay = ulCopy;
                    ulWait = ulCopy;
                }
            }
        }
    }

    {
        TXT_tdstLabel_* node;
        TXT_tdstLabel_* pPick;
        s32 bPick;
        MTH3D_tdstVector_ pos;

        g_stInfoTextAffiche[iNumber].bFirstInit = ucMark;
        g_stInfoTextAffiche[iNumber].ulEvent = (s32) lReturn;
        g_stInfoTextAffiche[iNumber].wCurrentLetterInString = sIndex;

        bPick = 0;
        node = inv_AI_p_stTextLabels;
        pPick = 0;
        while (node != 0 && bPick == 0) {
            if (node->pText == 0 || node->pText == pText) {
                pPick = node;
                bPick = 1;
            } else {
                node = node->pNext;
            }
        }

        if (!bPick) {
            pPick = AI_fn_p_vTrueAlloc(gp_6F74, 0x26C);
            node = pPick;
            {
                s32 id = inv_TXT_fn_lGetFreeScriptStringNumber();
                TXT_tdstLabel_* previous = inv_AI_p_stTextLabels;
                pPick->lId = id;
                pPick->pNext = previous;
            }
            inv_AI_p_stTextLabels = pPick;
        }

        inv_strcpy(pPick->szText, str);
        node->ucUseScale = 0;
        node->pText = pText;
        node->xScale = 255.0f;

        if (sIndex >= 0 && sIndex < (s16) inv_strlen(str))
            pPick->szText[sIndex] = 0;

        MTH3D_M_vSetXYZ_inv(&pos, stBase.xX + 5.0f, stBase.xY - 25.0f, stBase.xZ);
        if (node->lId != -1)
            JFFTXT_vAddText(node->lId, &pos, pPick->szText, 255.0f);
    }

    param->ucType = E_vt_Integer;
    param->u.lValue = (s32) ulWait;
    fn_ucSetDsgVar((u8) stWait.ucSet, stWait.ucSet2,
                      ((tdstEngineObject_*) pWait->hLinkedObject)->h_Brain->p_stMind,
                      param);
    ((volatile tdstGetSetParam_*) param)->ucType = E_vt_Integer;
    ((volatile tdstGetSetParam_*) param)->u.lValue = (s32) lReturn;
    return current;
}

static inline AI_tdstMind_* inv_AI_fn_p_stSuperObjectMind(HIE_tdstSuperObject_* superObject)
{
    return ((tdstEngineObject_*) superObject->hLinkedObject)->h_Brain->p_stMind;
}

void* func_001E9068(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                    tdstGetSetParam_* param)
{
    stTfmtStatus_ status;
    MTH3D_tdstVector_ scale;
    MTH3D_tdstVector_ position;
    u8 aucText[600];

    struct {
        u8 ucSet;
        u8 ucSet2;
        u8 ucRest2;
        u8 ucRest3;
        u16 uwRest4;
        u8 ucRest6;
        u8 ucRest7;
    } stBind;

    u8 aucFmt[20];

    struct {
        s8 ucSet;
        u8 ucSet2;
        u8 ucRest2;
        u8 ucRest3;
        u16 uwRest4;
        u8 ucRest6;
        u8 ucRest7;
    } stWait;

    POS_tdstMatrix_ stMatrix;
    u8 ucMark;
    s16 sIndex;
    HIE_tdstSuperObject_* pBind;
    HIE_tdstSuperObject_* pWait;
    s32 lWait;
    s32 lCopy;
    s32 lReturn;
    tdstEngineObject_* pEval;
    HIE_tdstSuperObject_* pCurrent;
    s32 iNumber;
    u8* str;
    u8* pIn;
    u8* pOut;
    u8* pCmd;
    HIE_tdstSuperObject_* pText;
    s32 lSaved;
    s32 lPrevious;
    s32 lCopySaved;
    u32 ulNow;
    s32 iEntry;
    u16 uwModel;
    POS_tdstMatrix_* pMatrix;
    s32* pSlotWait;

    ucMark = 1;
    stBind.ucSet = 0xFF;
    stWait.ucSet = -1;
    sIndex = 0;
    pBind = 0;
    stBind.uwRest4 = 13;
    stBind.ucSet2 = 0;
    stWait.uwRest4 = 13;
    stBind.ucRest2 = 0;
    lReturn = 0;
    pWait = 0;
    pCurrent = 0;
    stWait.ucSet2 = 0;
    iNumber = 0;
    stWait.ucRest2 = 0;

    stream = inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
        superObject, stream, param, &pBind, &stBind.ucSet, &stBind.ucSet2, &stBind.ucRest2,
        &stBind.uwRest4, &stBind.ucRest6);
    if (param->u.lValue != 0)
        pCurrent = inv_fn_hGetOwnerSuperObject((tdstEngineObject_*) param->u.pvValue);
    stream = fn_p_stEvalTree(superObject, stream, param);
    pEval = param->u.pvValue;
    stream = fn_p_stEvalTree(superObject, stream, param);
    position = param->u.stVector;
    stream = fn_p_stEvalTree(superObject, stream, param);

    str = fn_szGetStringFromTextOrStringParam(param);
    if (str == 0)
        str = (u8*) "No Text";

    if (str[0] == '/' && str[1] == '-') {
        pCmd = str + 2;
        switch (*pCmd) {
        case 'W':
        case 'w':
            TFMT_vUpdateSpacingTable(pCmd + 1);
            break;
        case 'Z':
        case 'z':
            TFMT_vUpdateScaleTable(pCmd + 1);
            break;
        }
        param->u.lValue = 0;
        param->ucType = E_vt_Integer;
        return fn_p_stSkipThisArgument(stream);
    }

    inv_strcpy(aucText, str);
    pIn = str;
    pOut = aucText;
    while (*pIn) {
        if (pIn[0] == '/' && pIn[1] == '%') {
            switch (pIn[2]) {
            case 'I':
            case 'i':
                iEntry = inv_atoi(pIn + 3);
                sprintf(aucFmt, "%d", TEXT_gai_IntParameters[iEntry]);
                inv_strcpy2(pOut, aucFmt);
                pOut += strlen(aucFmt);
                while (*pIn != ':')
                    pIn++;
                pIn++;
                continue;
            case 'F':
            case 'f':
                iEntry = inv_atoi(pIn + 3);
                sprintf(aucFmt, "%.3f", TEXT_gaf_FloatParameters[iEntry]);
                inv_strcpy2(pOut, aucFmt);
                pOut += strlen(aucFmt);
                while (*pIn != ':')
                    pIn++;
                pIn++;
                continue;
            }
        }
        *pOut++ = *pIn++;
    }
    *pOut = 0;
    str = aucText;

    stream = inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
        superObject, stream, param, &pWait, &stWait.ucSet, &stWait.ucSet2, &stWait.ucRest2,
        &stWait.uwRest4, &stWait.ucRest6);
    lWait = param->u.lValue;

    if (pCurrent != 0) {
        iNumber = TFMT_ucNumberOfCurrentTextAffiche(pCurrent);
        sIndex = g_stInfoTextAffiche[iNumber].wCurrentLetterInString;
        ucMark = g_stInfoTextAffiche[iNumber].bFirstInit;
        lReturn = g_stInfoTextAffiche[iNumber].ulEvent;
        if (sIndex >= (s16) inv_strlen(str)) {
            param->ucType = E_vt_Integer;
            param->u.lValue = lReturn;
            return stream;
        }
    }
    if (pCurrent == 0) {
        MTH3D_M_vSetXYZ_inv(&scale, 1.0f, 1.0f, 1.0f);
        if (pEval != 0 && fn_ucIsAnAlwaysObject(pEval)) {
            uwModel = pEval->h_StandardGame->otObjectModelType;
            pMatrix = &stMatrix;
            gu_MTH4D_M_vSetIdentityMatrix(pMatrix);
            if (stMatrix.ucType == MAT_eTransform_Identity)
                pMatrix->ucType = MAT_eTransform_Translation;
            MTH4D_fn_vCopy3DTo4DVector(&stMatrix.stMatrix.stCol_3, &scale);
            pText = fn_p_stAllocateAlways(uwModel, gp_stDynamicWorld, superObject, 0, pMatrix);
            if (pText != 0) {
                lSaved = lWait;
                inv_HIE_fn_p_stGetEngineObject(pText)->h_3dData->ucFlagEndState = 1;
                iNumber = TFMT_ucSetNumber4CurrentTextAffiche();
                g_stInfoTextAffiche[iNumber].ulTimer = inv_fn_ulGetEngineClock();
                g_stInfoTextAffiche[iNumber].ulCurrentDelay = lWait;
                gp_SuperObjPersoGenerated[iNumber] = pText;
                param->u.lValue = pText->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv ? (s32) pText->hLinkedObject : 0;
                param->ucType = E_vt_Perso;
                fn_ucSetDsgVar(stBind.ucSet, stBind.ucSet2, inv_AI_fn_p_stSuperObjectMind(pBind), param);
                if (lWait == 0) {
                    sIndex = -1;
                    lReturn = TFMT_wPositionModulesAccordingToText(&status, pText, str, 0, -1,
                                                                   &ucMark, &sIndex, &lWait);
                    sIndex = inv_strlen(str);
                } else {
                    lReturn = TFMT_wPositionModulesAccordingToText(&status, pText, str, 0, -1,
                                                                   &ucMark, &sIndex, &lWait);
                }
                if (lWait != lSaved) {
                    if (lWait < 0)
                        lWait = -lWait;
                    else
                        g_stInfoTextAffiche[iNumber].ulCurrentDelay = lWait;
                }
            }
        }
    } else {
        lPrevious = g_stInfoTextAffiche[iNumber].ulTimer;
        ulNow = inv_fn_ulGetEngineClock();
        ucMark = g_stInfoTextAffiche[iNumber].bFirstInit;
        if (ulNow - lPrevious >= (u32) lWait) {
            lPrevious = lWait = g_stInfoTextAffiche[iNumber].ulCurrentDelay;
            if (lPrevious == 0) {
                while (sIndex < (s16) inv_strlen(str) && lWait == 0)
                    lReturn = TFMT_wPositionModulesAccordingToText(&status, pCurrent, str, 0, -1,
                                                                   &ucMark, &sIndex, &lWait);
            } else {
                lReturn = TFMT_wPositionModulesAccordingToText(&status, pCurrent, str, 0, -1,
                                                               &ucMark, &sIndex, &lWait);
            }
            g_stInfoTextAffiche[iNumber].ulTimer = inv_fn_ulGetEngineClock();
            if (lWait != lPrevious) {
                if (lWait < 0)
                    lWait = -lWait;
                else
                    g_stInfoTextAffiche[iNumber].ulCurrentDelay = lWait;
            }
            g_stInfoTextAffiche[iNumber].ulTimer = inv_fn_ulGetEngineClock();
        } else if (lReturn != 0) {
            pSlotWait = &g_stInfoTextAffiche[iNumber].ulCurrentDelay;
            lCopySaved = lCopy = *pSlotWait;
            lReturn = TFMT_wPositionModulesAccordingToText(&status, pCurrent, str, 0, -1, &ucMark,
                                                           &sIndex, &lCopy);
            if (lCopy != lCopySaved) {
                if (lCopy < 0)
                    lCopy = -lCopy;
                else
                    *pSlotWait = lCopy;
                lWait = lCopy;
            }
        }
    }

    g_stInfoTextAffiche[iNumber].bFirstInit = ucMark;
    g_stInfoTextAffiche[iNumber].ulEvent = lReturn;
    g_stInfoTextAffiche[iNumber].wCurrentLetterInString = sIndex;
    param->u.lValue = lWait;
    param->ucType = E_vt_Integer;
    fn_ucSetDsgVar((u8) stWait.ucSet, stWait.ucSet2, inv_AI_fn_p_stSuperObjectMind(pWait), param);
    param->u.lValue = lReturn;
    param->ucType = E_vt_Integer;
    return stream;
}

tdstNodeInterpret_* fn_p_stCode4PointsDair(HIE_tdstSuperObject_* context,
                                           tdstNodeInterpret_* stream, tdstGetSetParam_* out)
{
    tdstGetSetParam_* value;
    f32 sign;
    f32 result;
    f32 change;

    value = out;
    sign = 1.0f;
    switch (AI_NODE_ID(stream)) {
    case eFunc_LitPointsDair:
        result = stExtendDatas4Ray.xAirPoints4Ray;
        break;
    case eFunc_LitPointsDairMax:
        result = stExtendDatas4Ray.xAirPointsMax4Ray;
        break;
    case eFunc_EnleveEtLitPointsDair:
        sign = -1.0f;
    case eFunc_AjouteEtLitPointsDair:
        stream = fn_p_stEvalTree(context, stream, value);
        change = sign * (value->ucType == E_vt_Float ? value->u.xValue : (f32) value->u.lValue);
        result = stExtendDatas4Ray.xAirPoints4Ray + change;
        if (result > stExtendDatas4Ray.xAirPointsMax4Ray) {
            result = stExtendDatas4Ray.xAirPointsMax4Ray;
        } else if (result < 0.0f) {
            result = 0.0f;
        }
        stExtendDatas4Ray.xAirPoints4Ray = result;
        break;
    case eFunc_EnleveEtLitPointsDairMax:
        sign = -1.0f;
    case eFunc_AjouteEtLitPointsDairMax:
        stream = fn_p_stEvalTree(context, stream, value);
        change = sign * (value->ucType == E_vt_Float ? value->u.xValue : (f32) value->u.lValue);
        result = stExtendDatas4Ray.xAirPointsMax4Ray + change;
        if (result < 0.0f) {
            result = 0.0f;
        }
        if (result < stExtendDatas4Ray.xAirPoints4Ray) {
            stExtendDatas4Ray.xAirPoints4Ray = result;
        }
        stExtendDatas4Ray.xAirPointsMax4Ray = result;
        break;
    default:
        return stream;
    }
    out->u.xValue = result;
    out->ucType = E_vt_Float;
    return stream;
}

void* fn_p_stCode4PointsDeMagie(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                tdstGetSetParam_* out)
{
    s32 value;
    s32 amount;

    switch (AI_NODE_ID(stream)) {
    case eFunc_LitPointsDeMagie:
        value = stExtendDatas4Ray.iMagicPoints4Ray;
        break;
    case eFunc_LitPointsDeMagieMax:
        value = dword_38419C[0];
        break;
    case eFunc_AjouteEtLitPointsDeMagie:
        stream = fn_p_stEvalTree(superObject, stream, out);
        if (out->ucType == E_vt_Float)
            amount = inv_MTH_fn_lToIntegerOrdered(out->u.xValue);
        else
            amount = out->u.lValue;
        stExtendDatas4Ray.iMagicPoints4Ray += amount;
        if (stExtendDatas4Ray.iMagicPointsMax4Ray < stExtendDatas4Ray.iMagicPoints4Ray)
            stExtendDatas4Ray.iMagicPoints4Ray = stExtendDatas4Ray.iMagicPointsMax4Ray;
        value = stExtendDatas4Ray.iMagicPoints4Ray;
        break;
    case eFunc_AjouteEtLitPointsDeMagieMax:
        stream = fn_p_stEvalTree(superObject, stream, out);
        if (out->ucType == E_vt_Float)
            amount = inv_MTH_fn_lToIntegerOrdered(out->u.xValue);
        else
            amount = out->u.lValue;
        stExtendDatas4Ray.iMagicPointsMax4Ray += amount;
        value = stExtendDatas4Ray.iMagicPointsMax4Ray;
        break;
    case eFunc_EnleveEtLitPointsDeMagie:
        stream = fn_p_stEvalTree(superObject, stream, out);
        if (out->ucType == E_vt_Float)
            amount = inv_MTH_fn_lToIntegerOrdered(out->u.xValue);
        else
            amount = out->u.lValue;
        stExtendDatas4Ray.iMagicPoints4Ray -= amount;
        if (stExtendDatas4Ray.iMagicPoints4Ray < 0)
            stExtendDatas4Ray.iMagicPoints4Ray = 0;
        value = stExtendDatas4Ray.iMagicPoints4Ray;
        break;
    case eFunc_EnleveEtLitPointsDeMagieMax:
        stream = fn_p_stEvalTree(superObject, stream, out);
        if (out->ucType == E_vt_Float)
            amount = inv_MTH_fn_lToIntegerOrdered(out->u.xValue);
        else
            amount = out->u.lValue;
        stExtendDatas4Ray.iMagicPointsMax4Ray -= amount;
        if (stExtendDatas4Ray.iMagicPointsMax4Ray < 0)
            stExtendDatas4Ray.iMagicPointsMax4Ray = 0;
        value = stExtendDatas4Ray.iMagicPointsMax4Ray;
        break;
    default:
        return stream;
    }

    out->u.lValue = value;
    out->ucType = E_vt_Integer;
    return stream;
}

s32 fn_vGetZoneCenter(COL_tdstCollideObject_* _hGeoObj, MTH3D_tdstVector_* vector)
{
    tdstGetSetParam_ stA;
    tdstGetSetParam_ stB;
    u16 indexA;
    u16 indexB;
    f32 scale;
    s16 type;

    type = *_hGeoObj->d_xListOfElementsTypes;
    switch (type) {
    case 7:
        gu__GEO_vGetPointOfObject(vector, &scale, _hGeoObj);
        return 1;
    case 8:
        GEO_vGetMinMaxPointOfIndexedAlignedBox(_hGeoObj, 0, 0, &indexA, &indexB);
        GEO_vGetPointOfObject(_hGeoObj, &stA.u.stVector, (s16) indexA);
        GEO_vGetPointOfObject(_hGeoObj, &stB.u.stVector, (s16) indexB);
        MTH3D_M_vSetXYZ_inv(vector, (stA.u.stVector.xX + stB.u.stVector.xX) * 0.5f,
                            (stA.u.stVector.xY + stB.u.stVector.xY) * 0.5f,
                            (stA.u.stVector.xZ + stB.u.stVector.xZ) * 0.5f);
        return 1;
    default:
        return 0;
    }
}

void fn_vGetPersoZDxCenter(HIE_tdstSuperObject_* obj, u16 mask, u8 kind, u8 flag,
                           MTH3D_tdstVector_* vec)
{
    f32 scale[3];
    u8 inverseMatrix[0x60];
    s32 index;
    s32 count;
    void* node;
    s32 hit;
    void* geo;
    tdstAChannel_* link;
    HIE_tdstSuperObject_* znode;
    POS_tdstMatrix_* matrix;
    s32 type;
    s32 ok;
    u64 tested;

    count = fn_lGetZdxListNumberOfElements(kind, obj);
    hit = 0;
    for (index = 0; index < count; index++) {
        node = fn_hGetGeometricZdxOfTypeAtIndex(kind, (u16) index, obj);
        if (node != 0 && fn_hIsTypeExistsInGeoObj(node, mask) != 0 &&
            fn_hIsThisZoneActive(obj, kind, (u8) index) != 0) {
            hit = fn_vGetZoneCenter(node, vec);
            if (hit != 0) {
                if (flag != 0) {
                    POS_fn_vMulMatrixVertex(vec, obj->gu_hGlobalMatrix, vec);
                } else {
                    matrix = obj->gu_hGlobalMatrix;
                    type = matrix->ucType;
                    ok = 0;
                    if (type == 3 || type == 4 || (u8) type == 7 || (u8) type == 6 ||
                        (u8) type == 8) {
                        ok = 1;
                    }
                    tested = ok;
                    if (tested != 0) {
                        POS_fn_vGetScaleMatrix(matrix, (MTH3D_tdstVector_*) scale);
                        MTH3D_M_vSetXYZ_inv(vec, scale[0] * vec->xX, scale[1] * vec->xY,
                                            scale[2] * vec->xZ);
                    }
                }
                break;
            }
        }
    }

    if (hit != 0) {
        return;
    }
    if (kind != 3) {
        return;
    }

    link = ((tdstEngineObject_*) obj->hLinkedObject)->h_3dData->hFirstActiveChannel;
    if (link != 0) {
        do {
            znode = link->hSupObject;
            if (znode != 0 && (znode->lTypeOfLinkedObject & HIE_TYPE_PHYSICAL_OBJECT_inv) != 0 && (znode->flags & 2) == 0) {
                geo = ((PO_tdstPhysicalObject_*) znode->hLinkedObject)->_hCollideSet;
                if (geo != 0 && fn_hIsTypeExistsInGeoObj(geo, mask) != 0 &&
                    fn_vGetZoneCenter(geo, vec) != 0) {
                    POS_fn_vMulMatrixVertex(vec, znode->gu_hGlobalMatrix, vec);
                    if (flag == 0) {
                        POS_fn_vInvertMatrix((POS_tdstMatrix_*) inverseMatrix, obj->gu_hGlobalMatrix);
                        POS_fn_vMulMatrixVertex(vec, (POS_tdstMatrix_*) inverseMatrix, vec);
                    }
                    return;
                }
            }
            link = link->p_stNextActiveChannel;
        } while (link != 0);
    }
}

static inline f32 inv_AI_fn_xCandidateDistance(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b)
{
    f32 dx;
    f32 dy;
    f32 dz;
    f32 squareLength;

    dx = a->xX - b->xX;
    dy = a->xY - b->xY;
    dz = a->xZ - b->xZ;
    squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    return MTH_fn_xSqrtOpt2(squareLength);
}

static inline f32 inv_AI_fn_xCandidateHypot(f32 x, f32 y, f32 z)
{
    f32 squareLength;

    squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    return MTH_fn_xSqrtOpt2(squareLength);
}

static inline void inv_AI_fn_vCandidateGetElements(f32* x, f32* y, f32* z, MTH3D_tdstVector_* v)
{
    *x = v->xX;
    *y = v->xY;
    *z = v->xZ;
}

HIE_tdstSuperObject_* fn_hFindClosestPersoCandidate(
    HIE_tdstSuperObject_* best, f32* bestDistance, f32* bestScore, HIE_tdstSuperObject_* self,
    MTH3D_tdstVector_* position, MTH3D_tdstVector_* axis, POS_tdstMatrix_* matrix,
    HIE_tdstSuperObject_* sector, f32 weightX, f32 weightXY, f32 weightDistance, f32 slopeLimit,
    f32 sineLimit, s32 mask, u16 zone, s32 family, u16 opcode)
{
    MTH3D_tdstVector_ candidatePosition;
    MTH3D_tdstVector_ local;
    SECT_tdstListOfSOCharacters_* node;
    HIE_tdstSuperObject_* character;
    gu_tdstStandardGame* standardGame;
    f32 squareLength;
    f32 distance;
    f32 planar;
    f32 x;
    f32 y;
    f32 z;
    f32 horizontal;
    f32 angle;
    f32 sine;
    f32 score;
    f32 absX;

    for (node = ((SECT_tdstSector_*) sector->hLinkedObject)->stListOfCharacters.hFirstElementDyn; node != NULL;
         node = node->hNextBrotherDyn) {
        character = node->hPointerOfCharacter;
        if (character == self)
            continue;
        if (family != 0 &&
            family != (s32) ((tdstEngineObject_*) character->hLinkedObject)->h_3dData->h_Family)
            continue;
        standardGame = ((tdstEngineObject_*) character->hLinkedObject)->h_StandardGame;
        if (standardGame == NULL)
            continue;
        if (standardGame->ucMiscFlags & 1)
            continue;
        if (!((standardGame->ucMiscFlags >> 1) & 1))
            continue;
        if ((standardGame->ulCustomBits & mask) != mask)
            continue;

        candidatePosition = *inv_POS_fn_p_stPosMatrixTranslation(character->gu_hGlobalMatrix);
        if (zone != 0)
            fn_vGetPersoZDxCenter(character, zone, 2, 1, &candidatePosition);

        if (opcode == 0x74) {
            distance = inv_AI_fn_xCandidateDistance(&candidatePosition, position);
            if (distance < *bestDistance) {
                *bestDistance = distance;
                best = character;
            }
        } else if (opcode >= 0x76 && opcode <= 0x79) {
            inv_MTH3D_fn_vSubXYZ(&candidatePosition, position, &candidatePosition);
            inv_MTH3D_fn_vNormalizeXYZ(&candidatePosition);
            distance = inv_MTH_fn_xMulAdd3(axis->xX, candidatePosition.xX, axis->xY,
                                candidatePosition.xY, axis->xZ, candidatePosition.xZ);
            if (!(0.5f < distance))
                continue;
            distance = inv_AI_fn_xCandidateDistance(&candidatePosition, position);
            if (distance < *bestDistance) {
                *bestDistance = distance;
                best = character;
            }
        } else {
            score = 0.0f;
            POS_fn_vMulMatrixVertex(&local, matrix, &candidatePosition);
            inv_AI_fn_vCandidateGetElements(&absX, &y, &z, &local);
            local.xZ = 0.0f;
            distance = inv_MTH3D_fn_xNormXYZ(&local);
            local.xZ = z;
            x = absX;
            horizontal = inv_AI_fn_xCandidateHypot(x, y, score);
            if (horizontal != score)
                angle = atanf(z / horizontal);
            else
                angle = MTH_HALF_PI_inv;
            sine = inv_MTH_fn_xAbsolute(sinf(angle));
            if (!(slopeLimit * horizontal < inv_MTH_fn_xNegate(y)))
                continue;
            if (!(sine < sineLimit))
                continue;
            if (0.0f <= y) {
                if (x == 0.0f)
                    score = flt_393D88[0];
                else
                    score = inv_MTH_fn_xNegate(fabsf(y / x));
            } else {
                if (x == 0.0f) {
                    score = flt_393D8C[0];
                } else {
                    absX = inv_MTH_fn_xAbsolute(x);
                    squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(local.xX, local.xY, local.xZ);
                    score = weightX / absX;
                    planar = absX * absX + y * y;
                    score += weightXY / MTH_fn_xSqrtOpt2(planar);
                    score += weightDistance / MTH_fn_xSqrtOpt2(squareLength);
                }
            }
            if (*bestScore < score) {
                *bestScore = score;
                best = character;
            }
        }
    }
    return best;
}

void* fn_p_stCode4PersoLePlusProche(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                    tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ position;
    POS_tdstMatrix_ workMatrix;
    MTH3D_tdstVector_ axis;
    MTH3D_tdstVector_ wanted;
    MTH3D_tdstVector_ basisX;
    MTH3D_tdstVector_ basisY;
    MTH3D_tdstVector_ basisZ;
    POS_tdstMatrix_ sourceMatrix;
    MTH3D_tdstVector_ offset;
    HIE_tdstSuperObject_* saved;
    f32 bestDist;
    f32 worstDist;
    s32 selector;
    u16 useVector;
    s32 extra;
    u16 opcode;
    f32 xWeightX;
    f32 xWeightXY;
    f32 xWeightDistance;
    f32 xSlopeLimit;
    f32 xSineLimit;
    HIE_tdstSuperObject_* best;
    HIE_tdstSuperObject_* group;
    SECT_tdstListOfSOSectorsInGraphicInteraction_* entries;
    s32 count;
    void* selected;

    opcode = AI_NODE_ID(stream);
    bestDist = flt_393D90[0];
    worstDist = flt_393D94[0];
    xWeightX = 0.0f;
    xWeightXY = 0.0f;
    xWeightDistance = 0.0f;
    fn_vGetUltraOperatorPerso(superObject, &saved);

    stream = fn_p_stEvalTree(superObject, stream, param);
    useVector = 0;
    xSlopeLimit = 0.8944f;
    xSineLimit = 0.4473f;
    extra = 0;

    if (param->ucType == E_vt_Float) {
        selector = inv_MTH_fn_lToInteger(param->u.xValue);
    } else {
        selector = param->u.lValue;
    }

    switch (opcode) {
    case eFunc_CibleLaPlusProche:
    case eFunc_CibleLaPlusProcheAvecAngles:
        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float)
            xWeightX = param->u.xValue;
        else
            xWeightX = (f32) param->u.lValue;

        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float)
            xWeightXY = param->u.xValue;
        else
            xWeightXY = (f32) param->u.lValue;

        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float)
            xWeightDistance = param->u.xValue;
        else
            xWeightDistance = (f32) param->u.lValue;

        useVector = 1;
        stream = fn_p_stEvalTree(superObject, stream, param);
        wanted = param->u.stVector;

        if (opcode == eFunc_CibleLaPlusProcheAvecAngles) {
            stream = fn_p_stEvalTree(superObject, stream, param);
            if (param->ucType == E_vt_Float)
                xSlopeLimit = param->u.xValue;
            else
                xSlopeLimit = (f32) param->u.lValue;
            xSlopeLimit = cosf(xSlopeLimit * MTH_DEG_TO_RAD_inv);

            stream = fn_p_stEvalTree(superObject, stream, param);
            if (param->ucType == E_vt_Float)
                xSineLimit = param->u.xValue;
            else
                xSineLimit = (f32) param->u.lValue;
            xSineLimit = sinf(xSineLimit * MTH_DEG_TO_RAD_inv);
        }

        sourceMatrix = *(POS_tdstMatrix_*) ((HIE_tdstSuperObject_*) saved)->gu_hGlobalMatrix;
        if (!(MTH3D_M_bIsZeroXYZ_inv(&wanted))) {
            POS_fn_vGetRotationVectors(&sourceMatrix, &basisX, &basisY, &basisZ);

            {
                f32 nx;
                f32 ny;
                f32 nz;
                f32 norm2;
                f32 norm;
                f32 zero;
                f32 scale;

                nx = inv_MTH_fn_xNegate(wanted.xX);
                ny = inv_MTH_fn_xNegate(wanted.xY);
                nz = inv_MTH_fn_xNegate(wanted.xZ);
                zero = 0.0f;
                norm2 = MTH_fn_xSqrAddSqrAddSqrOpt2(nx, ny, nz);
                wanted.xX = nx;
                norm = MTH_fn_xSqrtOpt2(norm2);
                wanted.xY = ny;
                wanted.xZ = nz;
                if (norm != zero) {
                    scale = 1.0f / norm;
                    MTH3D_M_vSetXYZ_inv(&basisY, scale * nx, scale * ny, scale * nz);
                } else {
                    *&basisY = *&wanted;
                }
            }

            {
                f32 dot;
                dot = inv_MTH_fn_xMulAdd3(basisY.xX, basisZ.xX, basisY.xY, basisZ.xY, basisY.xZ,
                                          basisZ.xZ);
                if (__builtin_fabsf(dot - 1.0f) < 0.002f) {
                    DNM_p_stVectorCrossProduct(&basisZ, &basisX, &basisY);
                    {
                        f32 len2;
                        f32 len;
                        f32 inv;
                        inv_MTH3D_fn_vNormalizeXYZ(&basisZ);
                    }

                    DNM_p_stVectorCrossProduct(&basisX, &basisY, &basisZ);
                } else {
                    DNM_p_stVectorCrossProduct(&basisX, &basisY, &basisZ);
                    {
                        f32 len2;
                        f32 len;
                        f32 inv;
                        inv_MTH3D_fn_vNormalizeXYZ(&basisX);
                    }

                    DNM_p_stVectorCrossProduct(&basisZ, &basisX, &basisY);
                }
            }
            _POS_fn_vSetRotationMatrix(&sourceMatrix, &basisX, &basisY, &basisZ);
        }
        POS_fn_vInvertIsoMatrix(&workMatrix, &sourceMatrix);
        break;

    case eFunc_PersoLePlusProche:
        *(MTH3D_tdstVector_*) &position =
            *(MTH3D_tdstVector_*) &(
                 (POS_tdstMatrix_*) ((HIE_tdstSuperObject_*) saved)->gu_hGlobalMatrix)
                 ->stMatrix.stCol_3;
        break;

    case eFunc_NearerActorOfFamilyInFieldOfVision:
    case eFunc_NearerActorOfFamilyInFOVWithOffset:
        stream = fn_p_stEvalTree(superObject, stream, param);
        extra = param->u.lValue;
    case eFunc_NearerActorInFieldOfVision:
    case eFunc_NearerActorInFieldOfVisionWithOffset: {
        POS_tdstMatrix_* actorMatrix;
        actorMatrix = ((HIE_tdstSuperObject_*) saved)->gu_hGlobalMatrix;
        *(MTH3D_tdstVector_*) &position = *(MTH3D_tdstVector_*) &actorMatrix->stMatrix.stCol_3;
        POS_fn_vGetYAxisVector(actorMatrix, &axis);
        if ((u32) (opcode - eFunc_NearerActorInFieldOfVisionWithOffset) < 2u) {
            stream = fn_p_stEvalTree(superObject, stream, param);
            offset = param->u.stVector;
            POS_fn_vApplyMatrixToVector(&offset, actorMatrix, &offset);
            MTH3D_M_vAddXYZ_inv(&position, &position, &offset);
        }
    } break;

    case eFunc_PersoLePlusProcheDansSecteurCourant:
    case eFunc_GetNbActivePerso:
    default:
        return stream;
    }

    group = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)
                ->h_SectInfo->hCurrentSector;
    best = fn_hFindClosestPersoCandidate((void*) 0, &bestDist, &worstDist, saved, &position, &axis,
                                         &workMatrix, group, xWeightX, xWeightXY, xWeightDistance, xSlopeLimit, xSineLimit, selector,
                                         useVector, extra, opcode);

    {
        SECT_tdstSector_* sectorList;
        sectorList = group->hLinkedObject;
        count = sectorList->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt;
        entries = sectorList->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    }
    if (count > 0) {
        do {
            void* graphicSector;
            graphicSector = entries->hPointerOfSectorSO;
            best = fn_hFindClosestPersoCandidate(best, &bestDist, &worstDist, saved, &position,
                                                 &axis, &workMatrix, graphicSector, xWeightX, xWeightXY, xWeightDistance, xSlopeLimit,
                                                 xSineLimit, selector, useVector, extra, opcode);
            count--;
            entries++;
        } while (count > 0);
    }

    if (best != (void*) 0) {
        if (best->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv)
            selected = best->hLinkedObject;
        else
            selected = (void*) 0;
    } else {
        selected = (void*) 0;
    }
    param->u.lValue = (s32) selected;
    param->ucType = E_vt_Perso;
    return stream;
}

tdstNodeInterpret_* fn_p_stGraphFunction(HIE_tdstSuperObject_* p_SuperObjPerso,
                                         tdstNodeInterpret_* p_stTree, tdstGetSetParam_* param)
{
    void* node;
    s32 type;
    WP_tdstGraph_* hGraph;
    WP_tdstWayPoint_* hWayPoint1;
    WP_tdstWayPoint_* hWayPoint2;
    u32 ulCaps;
    s32 lWeight;
    s32 lStatus;
    s32 lResult;
    s8 before_type, after_type;

    node = p_stTree;
    type = *(u16*) ((u8*) node - 8);

    node = fn_p_stEvalTree(p_SuperObjPerso, node, param);
    hGraph = param->u.hGraph;
    node = fn_p_stEvalTree(p_SuperObjPerso, node, param);
    hWayPoint1 = param->u.m_hWayPoint;
    node = fn_p_stEvalTree(p_SuperObjPerso, node, param);
    hWayPoint2 = param->u.m_hWayPoint;

    if (hGraph == NULL) {
        lResult = -1;
        if (type == 0x97) {
            param->ucType = E_vt_Caps;
            param->u.lValue = 0;
            return node;
        }
    } else {
        if (type != 0x98) {
            if (type < 0x99) {
                if (type == 0x97)
                    goto op97;
                return node;
            }
            if (type == 0x99)
                goto op99;
            if (type == 0x9A)
                goto op9A;
            return node;

        op97:
            lStatus = WPG_fn_lGetLinkCapabilities(hGraph, hWayPoint1, hWayPoint2, &ulCaps);
            if (lStatus == -1) {
                after_type = 0x2C;
                param->u.lValue = 0;
                goto write_type;
            }
            lResult = ulCaps;
            before_type = 0x2C;
            goto type_then_value;
        }
        node = fn_p_stEvalTree(p_SuperObjPerso, node, param);
        ulCaps = param->u.lValue;
        node = fn_p_stEvalTree(p_SuperObjPerso, node, param);
        param->u.lValue = WPG_fn_lChangeLinkCapabilities(hGraph, hWayPoint1, hWayPoint2, ulCaps, param->u.lValue);
        after_type = 6;
        goto write_type;
    op99:
        lResult = WPG_fn_lGetLinkWeight(hGraph, hWayPoint1, hWayPoint2, &lWeight);
        if (lResult == -1) {
            param->u.lValue = lResult;
            param->ucType = E_vt_Integer;
            return node;
        }
        lResult = lWeight;
    }
    before_type = 6;
type_then_value:
store_value:
    param->u.lValue = lResult;
    param->ucType = before_type;
    goto return_node;
op9A:
    node = fn_p_stEvalTree(p_SuperObjPerso, node, param);
    lWeight = param->u.lValue;
    param->u.lValue = WPG_fn_lChangeLinkWeight(hGraph, hWayPoint1, hWayPoint2, lWeight);
    after_type = 6;
write_type:
    param->ucType = after_type;
return_node:
    return node;
}

void* fn_p_stGraphToWayFunction(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                tdstGetSetParam_* p_stValue)
{
    void* context;
    tdstGetSetParam_* value;
    HIE_tdstSuperObject_* node;
    void* object;
    WP_tdstGraph_* graph;
    WP_tdstGraph_* path;
    WP_tdstWayPoint_* start;
    s32 command;
    s32 testLastChar;
    WP_tdstWayPoint_* end;
    s32 capability;
    s32 lNbNodes;

    value = p_stValue;
    testLastChar = 0;
    command = AI_NODE_ID(stream);
    context = superObject;
    fn_vGetUltraOperatorPerso(superObject, &node);
    object = ((tdstEngineObject_*) node->hLinkedObject)->h_MSWay;
    stream = fn_p_stEvalTree(context, stream, value);
    graph = (WP_tdstGraph_*) value->u.lValue;

    switch (command) {
    case eFunc_ReseauCheminLePlusCourt:
        stream = fn_p_stEvalTree(context, stream, value);
        start = (WP_tdstWayPoint_*) value->u.lValue;
        stream = fn_p_stEvalTree(context, stream, value);
        end = (WP_tdstWayPoint_*) value->u.lValue;
        if (object == 0 || graph == 0) {
            value->u.lValue = 0;
            value->ucType = E_vt_Integer;
            return stream;
        }
        capability = fn_ulGetCapabilities(
            node->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv ? node->hLinkedObject : 0);
        path = WPG_fn_lShortestPath(graph, start, end, capability);
        if (path == 0) {
            MSWay_fn_vDesinitPath(object);
            lNbNodes = 0;
            goto store;
        }
        MSWay_fn_vSetPath(object, path, testLastChar);
        lNbNodes = path->m_hListOfNode.lNumberOfElementsDyn;
        goto store;

    case eFunc_NetworkBuildOrderedPathCircular:
        testLastChar = 1;
    case eFunc_NetworkBuildOrderedPath:
        stream = fn_p_stEvalTree(context, stream, value);
        start = (WP_tdstWayPoint_*) value->u.lValue;
        if (object == 0 || graph == 0) {
            value->u.lValue = 0;
            value->ucType = E_vt_Integer;
            return stream;
        }
        path = WPG_fn_lBuildOrderedPath(graph, start);
        if (path != 0)
            goto test_shared;
        MSWay_fn_vDesinitPath(object);
        lNbNodes = 0;
        goto store;

    case eFunc_NetworkAllocateGraphToMSWayCircular:
        testLastChar = 1;
    case eFunc_NetworkAllocateGraphToMSWay:
        if (object == 0) {
            value->u.lValue = 0;
            value->ucType = E_vt_Integer;
            return stream;
        }
        if (graph == 0) {
            value->u.lValue = 0;
            value->ucType = E_vt_Integer;
            return stream;
        }
        path = WPG_fn_lBuildOrderedPath(graph, graph->m_hListOfNode.hFirstElementDyn->m_hWayPoint);
        do {
            if (path == 0)
                goto no_path;
        } while (0);
    test_shared:
        MSWay_fn_vSetPath(object, path, testLastChar);
        lNbNodes = path->m_hListOfNode.lNumberOfElementsDyn;
        goto store;
    default:
        return stream;
    }
no_path:
    MSWay_fn_vDesinitPath(object);
    lNbNodes = 0;
store:
    value->u.lValue = lNbNodes;
    value->ucType = E_vt_Integer;
    return stream;
}

void* fn_p_stWayFunction(HIE_tdstSuperObject_* inputContext, tdstNodeInterpret_* stream,
                         tdstGetSetParam_* value)
{
    void* context;
    HIE_tdstSuperObject_* node;
    s32 command;
    void* object;
    s32 arg_a;
    s32 arg_b;
    s32 type;

    context = inputContext;
    command = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(context, &node);
    object = ((tdstEngineObject_*) node->hLinkedObject)->h_MSWay;

    if (object == NULL) {
        switch (command) {
        case eFunc_ReseauLitWPAIndex:
            value->u.lValue = 0;
            value->ucType = E_vt_WayPoint;
            break;
        case eFunc_ReseauLitCapaciteLiaisonAIndex:
            value->u.lValue = 0;
            value->ucType = E_vt_Caps;
            break;
        default:
            break;
        }
        value->u.lValue = -1;
        value->ucType = E_vt_Integer;
        return stream;
    }
    switch (command) {
    case eFunc_ReseauLitIndexCourant:
        value->u.lValue = MSWay_fn_lGetCurrentIndexInPath(object);
    set_type_6:
        type = 6;
        goto store_type;

    case eFunc_ReseauForceIndexCourant:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue = MSWay_fn_lSetCurrentIndexInPath(object, value->u.lValue);
        goto set_type_6;

    case eFunc_ReseauLitPremierIndex:
        value->u.lValue = MSWay_fn_lGetFirstIndexInPath(object);
        goto set_type_6;

    case eFunc_ReseauLitDernierIndex:
        value->u.lValue = MSWay_fn_lGetLastIndexInPath(object);
        goto set_type_6;

    case eFunc_ReseauIncrementIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue = MSWay_fn_lIncrementCurrentIndexInPath(object, value->u.lValue);
        goto set_type_6;

    case eFunc_ReseauDecrementIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue = MSWay_fn_lIncrementCurrentIndexInPath(object, -value->u.lValue);
        goto set_type_6;

    case eFunc_ReseauLitWPAIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.m_hWayPoint = MSWay_fn_hGetWayPointOfPath(object, value->u.lValue);
        type = 10;
        goto store_type;

    case eFunc_ReseauLitCapaciteLiaisonAIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue = MSWay_fn_ulGetCapsForNextWayPointOfPath(object, value->u.lValue);
        type = 44;
        goto store_type;

    case eFunc_ReseauChangeCapaciteLiaisonAIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        arg_a = value->u.lValue;
        stream = fn_p_stEvalTree(context, stream, value);
        arg_b = value->u.lValue;
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue =
            MSWay_fn_lChangeCapsForNextWayPointOfPath(object, arg_a, arg_b, value->u.lValue);
        goto set_type_6;

    case eFunc_ReseauLitPoidsLiaisonAIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue = MSWay_fn_lGetWeightForNextWayPointOfPath(object, value->u.lValue);
        goto set_type_6;

    case eFunc_ReseauChangePoidsLiaisonAIndex:
        stream = fn_p_stEvalTree(context, stream, value);
        arg_a = value->u.lValue;
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue =
            MSWay_fn_lChangeWeightForNextWayPointOfPath(object, arg_a, value->u.lValue);
        goto set_type_6;

    case eFunc_NetworkGetIndexOfWPInMSWay:
        stream = fn_p_stEvalTree(context, stream, value);
        value->u.lValue = MSWay_fn_lLookForWPInMSWay(object, value->u.m_hWayPoint);
        goto set_type_6;

    case eFunc_NetworkForceWPToCurrent: {
        s32 index;
        s32 lResult;

        stream = fn_p_stEvalTree(context, stream, value);
        index = MSWay_fn_lLookForWPInMSWay(object, value->u.m_hWayPoint);
        if (index == -1) {
            lResult = -1;
        } else {
            lResult = MSWay_fn_lSetCurrentIndexInPath(object, index);
        }
        value->u.lValue = lResult;
        value->ucType = E_vt_Integer;
        return stream;
    }

    default:
        return stream;
    }

store_type:
    value->ucType = type;
    return stream;
}

tdstNodeInterpret_* fn_p_stCloserWPInGraph(HIE_tdstSuperObject_* superObject,
                                           tdstNodeInterpret_* stream, tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ location;
    MTH3D_tdstVector_ origin;
    MTH3D_tdstVector_ toWayPoint;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ toAimed;
    HIE_tdstSuperObject_* perso;
    s32 flags;
    u16 operatorId;
    s32 testVisibility;
    s32 mask;
    WP_tdstWayPoint_* excluded1;
    WP_tdstWayPoint_* excluded2;
    HIE_tdstSuperObject_* sector;
    WP_tdstWayPoint_* closest;
    WP_tdstWayPoint_* aimed;
    f32 best;
    WP_tdstGraph_* graph;
    s32 i;
    s32 count;
    WP_tdstWayPoint_* wayPoint;
    f32 distance;

    mask = 0;
    closest = NULL;
    excluded1 = NULL;
    excluded2 = NULL;
    best = -1.0f;
    operatorId = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &perso);
    stream = fn_p_stEvalTree(superObject, stream, param);
    graph = param->u.pvValue;
    stream = fn_p_stEvalTree(superObject, stream, param);
    testVisibility = param->u.lValue;

    switch (operatorId) {
    case 0x7E:
        stream = fn_p_stEvalTree(superObject, stream, param);
        mask = param->u.lValue;
        stream = fn_p_stEvalTree(superObject, stream, param);
        excluded1 = param->u.pvValue;
        stream = fn_p_stEvalTree(superObject, stream, param);
        excluded2 = param->u.pvValue;
    case 0x7D:
        if (graph == NULL) {
            param->u.lValue = 0;
            param->ucType = E_vt_WayPoint;
            return stream;
        }
        origin = *inv_POS_fn_p_stPosMatrixTranslation(perso->gu_hGlobalMatrix);
        count = graph->m_hListOfNode.lNumberOfElementsDyn;
        sector = ((tdstEngineObject_*) perso->hLinkedObject)->h_SectInfo->hCurrentSector;
        for (i = 0; i < count; i++) {
            wayPoint = WPG_fn_hGetWaypointOfGraph(graph, i, &flags);
            if (operatorId == 0x7E) {
                if (wayPoint == excluded1 || wayPoint == excluded2 || (mask & flags) != mask)
                    continue;
            }
            WP_fnv_WayPoint_ComputeLocation(wayPoint, &location);
            inv_MTH3D_fn_vSubXYZ(&toWayPoint, &location, &origin);
            distance =
                MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(toWayPoint.xX, toWayPoint.xY, toWayPoint.xZ));
            if (best == -1.0f || distance < best) {
                if (testVisibility) {
                    if (!HIE_bDetectIntersectSegmentWithSuperObject(&origin, &toWayPoint, sector,
                                                                     0)) {
                        closest = wayPoint;
                        best = MTH_fn_xSqrtOpt2(
                            MTH_fn_xSqrAddSqrAddSqrOpt2(toWayPoint.xX, toWayPoint.xY, toWayPoint.xZ));
                    }
                } else {
                    best = distance;
                    closest = wayPoint;
                }
            }
        }
        param->u.pvValue = closest;
        param->ucType = E_vt_WayPoint;
        return stream;

    case 0x80:
        stream = fn_p_stEvalTree(superObject, stream, param);
        mask = param->u.lValue;
        stream = fn_p_stEvalTree(superObject, stream, param);
        excluded1 = param->u.pvValue;
        stream = fn_p_stEvalTree(superObject, stream, param);
        excluded2 = param->u.pvValue;
    case 0x7F:
        stream = fn_p_stEvalTree(superObject, stream, param);
        direction = param->u.stVector;
        if (graph == NULL) {
            param->u.lValue = 0;
            param->ucType = E_vt_WayPoint;
            return stream;
        }
        aimed = NULL;
        origin = *inv_POS_fn_p_stPosMatrixTranslation(perso->gu_hGlobalMatrix);
        count = graph->m_hListOfNode.lNumberOfElementsDyn;
        sector = ((tdstEngineObject_*) perso->hLinkedObject)->h_SectInfo->hCurrentSector;
        for (i = 0; i < count; i++) {
            wayPoint = WPG_fn_hGetWaypointOfGraph(graph, i, &flags);
            if (operatorId == 0x80) {
                if (wayPoint == excluded1 || wayPoint == excluded2 || (mask & flags) != mask)
                    continue;
            }
            WP_fnv_WayPoint_ComputeLocation(wayPoint, &location);
            inv_MTH3D_fn_vSubXYZ(&toAimed, &location, &origin);
            distance = inv_MTH3D_fn_xDotXYZGrouped(&direction, &toAimed) /
                       MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(toAimed.xX, toAimed.xY, toAimed.xZ));
            if (best == -1.0f || best < distance) {
                if (testVisibility &&
                    HIE_bDetectIntersectSegmentWithSuperObject(&origin, &toAimed, sector, 0))
                    continue;
                best = distance;
                aimed = wayPoint;
            }
        }
        param->u.pvValue = aimed;
        param->ucType = E_vt_WayPoint;
        return stream;
    }
    return stream;
}

void* fn_p_stTravelOnAGraph(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                            tdstGetSetParam_* pRec)
{
    WP_tdstGraph_* hGraph;
    WP_tdstGraphNode_* hCurrentNode;
    WP_tdstWayPoint_* hWayPoint;
    WPArcNode* hElement;
    WPArcNode* hLink;
    u32 wMask;
    u32 wAnd;
    WP_tdstListOfArc_* hListOfArc;
    u32 wOp;
    s32 wBest;
    u32 wResult;
    u16 usType;

    hListOfArc = NULL;
    wMask = 0;
    wOp = 0;

    usType = AI_NODE_ID(stream);
    stream = fn_p_stEvalTree(superObject, stream, pRec);
    hGraph = pRec->u.pvValue;

    if (usType == eFunc_NetworkAffectTypeOfConnectedWP || usType == eFunc_NetworkAffectTypeOfConnectedWPWithCapa) {
        stream = fn_p_stEvalTree(superObject, stream, pRec);
        wMask = pRec->u.ulValue;
    }

    stream = fn_p_stEvalTree(superObject, stream, pRec);
    wAnd = 0;
    hWayPoint = pRec->u.pvValue;

    if (usType == 0x81 || usType == eFunc_NetworkAffectTypeOfConnectedWPWithCapa) {
        stream = fn_p_stEvalTree(superObject, stream, pRec);
        wAnd = pRec->u.ulValue;
    }

    if (usType == eFunc_NetworkAffectTypeOfConnectedWP || usType == eFunc_NetworkAffectTypeOfConnectedWPWithCapa) {
        stream = fn_p_stEvalTree(superObject, stream, pRec);
        wOp = pRec->u.ucValue;
    }

    wResult = 0;
    for (hCurrentNode = hGraph->m_hListOfNode.hFirstElementDyn; hCurrentNode != NULL;
         hCurrentNode = hCurrentNode->hNextBrotherDyn) {
        if (hCurrentNode->m_hWayPoint == hWayPoint) {
            wResult = (u32) hWayPoint;
            break;
        }
    }
    if (wResult != 0) {
        hListOfArc = IPT_fn_bGetActivationFlagOfEntryElement_0(hCurrentNode);
    }

    switch (usType) {
    case 0x81:
        wResult = 0;
        if (hListOfArc != NULL) {
            wBest = 0x7FFFFFFF;
            hElement = hListOfArc->head;
            while (hElement != NULL) {
                hWayPoint = hElement->destination->m_hWayPoint;
                if ((hElement->currentCapabilities & wAnd) != 0 && hElement->currentWeight < wBest) {
                    wBest = hElement->currentWeight;
                    wResult = (u32) hWayPoint;
                }
                hElement = hElement->next;
            }
        }
        pRec->u.ulValue = wResult;
        pRec->ucType = E_vt_WayPoint;
        return stream;
    case eFunc_NetworkAffectTypeOfConnectedWP:
    case eFunc_NetworkAffectTypeOfConnectedWPWithCapa:
        wResult = 0;
        if (wOp == 2) {
            for (hCurrentNode = hGraph->m_hListOfNode.hFirstElementDyn; hCurrentNode != NULL;
                 hCurrentNode = hCurrentNode->hNextBrotherDyn) {
                hCurrentNode->m_lTypeOfWP &= ~wMask;
            }
        }
        if (wOp == 3) {
            for (hCurrentNode = hGraph->m_hListOfNode.hFirstElementDyn; hCurrentNode != NULL;
                 hCurrentNode = hCurrentNode->hNextBrotherDyn) {
                hCurrentNode->m_lTypeOfWP = 0;
            }
        }
        if (hListOfArc != NULL) {
            hLink = hListOfArc->head;
            while (hLink != NULL) {
                if (usType != eFunc_NetworkAffectTypeOfConnectedWPWithCapa || (hLink->currentCapabilities & wAnd) != 0) {
                    wResult++;
                    if (wOp < 6) {
                        switch (wOp) {
                        case 0:
                        case 2:
                            hLink->destination->m_lTypeOfWP |= wMask;
                            break;
                        case 1:
                        case 3:
                            hLink->destination->m_lTypeOfWP = wMask;
                            break;
                        case 4:
                            hLink->destination->m_lTypeOfWP &= ~wMask;
                            break;
                        case 5:
                            hLink->destination->m_lTypeOfWP = 0;
                            break;
                        }
                    }
                }
                hLink = hLink->next;
            }
        }
        pRec->u.ulValue = wResult;
        pRec->ucType = E_vt_Integer;
        return stream;
    }

    return stream;
}

tdstNodeInterpret_* fn_p_stMiscFuncOnTypeOfWP(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                              tdstGetSetParam_* outParam)
{
    WP_tdstGraph_* hGraph;
    WP_tdstGraphNode_* walk;
    WP_tdstGraphNode_* found;
    WP_tdstWayPoint_* waypoint;
    u32 value;
    tdstNodeInterpret_* nextStream;
    tdstNodeInterpret_* result;

    nextStream = fn_p_stEvalTree(superObject, stream, outParam);
    hGraph = outParam->u.pvValue;
    result = fn_p_stEvalTree(superObject, nextStream, outParam);
    walk = hGraph->m_hListOfNode.hFirstElementDyn;
    waypoint = outParam->u.pvValue;
    found = (void*) 0;
    while (walk != (void*) 0) {
        if (walk->m_hWayPoint == waypoint) {
            found = walk;
            break;
        }
        walk = walk->hNextBrotherDyn;
    }
    value = 0;
    if (found != (void*) 0) {
        value = found->m_lTypeOfWP;
    }
    outParam->u.ulValue = value;
    outParam->ucType = E_vt_Caps;
    return result;
}

void* fn_p_stGetSituationOnRail(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* cursor,
                                tdstGetSetParam_* out)
{
    MTH3D_tdstVector_ waypoint_position;
    MTH3D_tdstVector_ object_position;
    HIE_tdstSuperObject_* node;
    tdstEngineObject_* owner;
    tdstMSWay_* path;
    POS_tdstMatrix_* position_owner;
    WP_tdstWayPoint_* waypoint;
    void* result;
    s32 current;
    s32 result_code;
    f32 radius;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 root;

    result = cursor;
    fn_vGetUltraOperatorPerso(superObject, &node);
    result = fn_p_stEvalTree(superObject, result, out);
    owner = node->hLinkedObject;
    path = owner->h_MSWay;
    radius = out->u.xValue;

    if (path == 0) {
        out->u.lValue = -1;
        out->ucType = E_vt_Integer;
        return result;
    }

    result_code = MSWay_fn_lGetNumberWayPointInPath(path);
    current = MSWay_fn_lGetCurrentIndexInPath(path);
    if (current == 1) {
        waypoint = (WP_tdstWayPoint_*) MSWay_fn_hGetWayPointOfPath(path, 0);
        result_code = 1;
    } else {
        if (current == result_code - 1) {
            waypoint = (WP_tdstWayPoint_*) MSWay_fn_hGetWayPointOfPath(path, current);
            result_code = 2;
        } else {
            out->u.lValue = 0;
            out->ucType = E_vt_Integer;
            return result;
        }
    }
    if (radius == 0.0f) {
        radius = WP_fnx_WayPoint_GetRadius(waypoint);
    }

    position_owner = node->gu_hLocalMatrix;
    object_position = *inv_POS_fn_p_stPosMatrixTranslation(position_owner);
    WP_fnv_WayPoint_GetVertex(waypoint, &waypoint_position);

    if (waypoint_position.xX == object_position.xX) {
        if (waypoint_position.xY == object_position.xY) {
            if (waypoint_position.xZ == object_position.xZ) {
                goto vectors_equal;
            }
        } else {
            goto vectors_not_equal;
        }
    } else {
        goto vectors_not_equal;
    }
vectors_not_equal:
    dx = waypoint_position.xX - object_position.xX;
    dy = waypoint_position.xY - object_position.xY;
    dz = waypoint_position.xZ - object_position.xZ;
    distance = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    root = MTH_fn_xSqrtOpt2(distance);
    distance = root;

    if (radius > distance) {
        out->u.lValue = result_code;
        out->ucType = E_vt_Integer;
        return result;
    }
vectors_equal:
    out->u.lValue = 0;
    out->ucType = E_vt_Integer;
type_done:
    return result;
}

void* fn_p_stPersoCapabilityFunction(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                     tdstGetSetParam_* outParam)
{
    void* savedStream = stream;
    tdstGetSetParam_* p_stOutput = outParam;
    HIE_tdstSuperObject_* object;

    fn_vGetUltraOperatorPerso(superObject, &object);
    p_stOutput->u.lValue = fn_ulGetCapabilities(
        object->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv ? object->hLinkedObject : NULL);
    p_stOutput->ucType = E_vt_Caps;
    return savedStream;
}

void* fn_p_stCapabilityFunction(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                tdstGetSetParam_* outParam)
{
    tdstGetSetParam_* p_stOutput = outParam;
    void* result = fn_p_stEvalTree(superObject, stream, p_stOutput);
    u8 shift = p_stOutput->u.ucValue;
    s32 mask = 1 << shift;
    p_stOutput->ucType = E_vt_Caps;
    p_stOutput->u.lValue = mask;
    return result;
}

s32 fn_p_stGetScrollSpeed(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                          tdstGetSetParam_* outParam)
{
    void* savedContext;
    tdstGetSetParam_* p_stOutput;
    s32 result;
    void* vector;
    s32 type;

    savedContext = superObject;
    p_stOutput = outParam;
    result = (s32) fn_p_stEvalTree(superObject, stream, p_stOutput);
    outParam = (void*) p_stOutput->u.lValue;
    func_001018B8(savedContext, &vector, outParam);
    *(MTH3D_tdstVector_*) p_stOutput = *(MTH3D_tdstVector_*) &vector;
    type = 0x15;
    p_stOutput->ucType = (u8) type;
    return result;
}

void* fn_p_stGetNbFrame(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                        tdstGetSetParam_* outParam)
{
    void* savedStream;
    tdstGetSetParam_* p_stOutput;
    HIE_tdstSuperObject_* object;
    s32 value;

    savedStream = stream;
    p_stOutput = outParam;
    fn_vGetUltraOperatorPerso(superObject, &object);
    value = PLA_fn_uwGetNbFrameInCurrAnim(object);
    p_stOutput->u.lValue = value;
    value = 6;
    p_stOutput->ucType = (u8) value;
    return savedStream;
}

tdstNodeInterpret_* fn_p_stVectorOperations(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree,
                                            tdstGetSetParam_* p_stValue)
{
    u16 uwOperator;
    void* p_stPerso;
    void* p_stNext;
    MTH3D_tdstVector_ stFirst;
    MTH3D_tdstVector_ stSecond;
    f32 dot;
    f32 xInvLength;
    f32 xLength;
    f32 xSquareLength;
    f32 xVectorY;
    p_stPerso = p_SuperObjPerso;
    uwOperator = AI_NODE_ID(p_stTree);
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    switch (uwOperator) {
    case eFunc_CrossProduct:
        stFirst = p_stValue->u.stVector;
        p_stNext = fn_p_stEvalTree(p_stPerso, p_stNext, p_stValue);
        stSecond = p_stValue->u.stVector;
        DNM_p_stVectorCrossProduct(&p_stValue->u.stVector, &stFirst, &stSecond);
        break;
    case eFunc_Normalize:
        xVectorY = p_stValue->u.stVector.xY;
        if (!(__builtin_fabsf(p_stValue->u.stVector.xX - 0.0f) < 1e-9f &&
              __builtin_fabsf(xVectorY - 0.0f) < 1e-9f &&
              __builtin_fabsf(p_stValue->u.stVector.xZ - 0.0f) < 1e-9f)) {
            xSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stValue->u.stVector.xX, xVectorY, p_stValue->u.stVector.xZ);
            xLength = MTH_fn_xSqrtOpt2(xSquareLength);
            if (xLength != 0.0f) {
                xInvLength = 1.0f / xLength;
                MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, xInvLength * p_stValue->u.stVector.xX,
                                    xInvLength * xVectorY, xInvLength * p_stValue->u.stVector.xZ);
            }
        }
        break;
    }
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stGetSPOCoordinates(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                             tdstGetSetParam_* outParam)
{
    MTH3D_tdstVector_ position;
    HIE_tdstSuperObject_* holder;
    tdstNodeInterpret_* result;

    result = fn_p_stEvalTree(superObject, stream, outParam);
    holder = (HIE_tdstSuperObject_*) outParam->u.lValue;
    if (holder != 0) {
        position = *(MTH3D_tdstVector_*) &holder->gu_hLocalMatrix->stMatrix.stCol_3;
    } else {
        position.xZ = position.xY = position.xX = 0.0f;
    }
    outParam->u.stVector = position;
    outParam->ucType = E_vt_Vector;
    return result;
}

tdstNodeInterpret_* fn_p_stGetNbActivePerso(HIE_tdstSuperObject_* superObject,
                                            tdstNodeInterpret_* stream, u8* out)
{
    *(s32*) out = GAM_fn_lGetNbPersoActive();
    out[0xC] = 6;
    return stream;
}

void* fn_p_stTractionFactorFunction(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                    tdstGetSetParam_* outParam)
{
    void* savedStream = stream;
    tdstGetSetParam_* p_stOutput = outParam;
    HIE_tdstSuperObject_* object;
    void* record;
    void* result;
    u32 type;
    u8 value;

    fn_vGetUltraOperatorPerso(superObject, &object);
    superObject = object;
    stream = 0;
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        stream = superObject->hLinkedObject;
    }
    record = ((tdstEngineObject_*) stream)->h_StandardGame;
    result = savedStream;
    type = 3;
    value = ((gu_tdstStandardGame*) record)->ucTractionFactor;
    p_stOutput->ucType = type;
    p_stOutput->u.ucValue = value;
    return result;
}

void* fn_p_stGetZDxCenter(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                          tdstGetSetParam_* p_stValue)
{
    s32 type;
    tdstEngineObject_* rec;
    void* node = (void*) (u32) stream;
    s32 mode;
    s32 flags;
    s32 tag;
    tdstNodeInterpret_* p_stNext;
    HIE_tdstSuperObject_* lOwnerPerso;
    f32 xZero;

    type = AI_NODE_ID(node);
    p_stNext = fn_p_stEvalTree(superObject, stream, p_stValue);
    rec = *(tdstEngineObject_**) p_stValue;
    lOwnerPerso = rec->h_StandardGame->h_SuperObject;
    node = fn_p_stEvalTree(superObject, p_stNext, p_stValue);
    flags = *(u16*) p_stValue;

    xZero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, xZero, xZero, xZero);
    switch (type) {
    case eFunc_GetCenterZDEType:
        mode = 2;
        break;
    case eFunc_GetCenterZDMType:
        mode = 1;
        break;
    case eFunc_GetCenterZDRType:
        mode = 3;
        break;
    case eFunc_GetCenterZDDType:
        mode = 0;
        break;
    default:
        mode = 0xFF;
        break;
    }
    tag = 0x15;
    p_stValue->ucType = tag;
    fn_vGetPersoZDxCenter(lOwnerPerso, flags, mode, 0, &p_stValue->u.stVector);
    return node;
}

void* fn_hGetNearestActorInSectorWithAnyCustombit(void* _hMe, u32 _hSector, s32 _ulFilterCustomBits)
{
    typedef struct PersoSearchListNode {
        void* object;
        struct PersoSearchListNode* next;
    } PersoSearchListNode;

    MTH3D_tdstVector_ source;
    MTH3D_tdstVector_ candidatePosition;
    void* listData = (void*) _hSector;
    PersoSearchListNode* node;
    void* best;
    f32 bestDistance;

    source = *(MTH3D_tdstVector_*) &(
                  (POS_tdstMatrix_*) ((HIE_tdstSuperObject_*) _hMe)->gu_hGlobalMatrix)
                  ->stMatrix.stCol_3;
    bestDistance = flt_393D98[0];
    best = (void*) 0;
    node = *(PersoSearchListNode**) ((HIE_tdstSuperObject_*) listData)->hLinkedObject;
    while (node != (PersoSearchListNode*) 0) {
        HIE_tdstSuperObject_* candidate = node->object;

        if (candidate != _hMe) {
            tdstEngineObject_* owner = candidate->hLinkedObject;
            gu_tdstStandardGame* flagsOwner = owner->h_StandardGame;

            if ((flagsOwner->ulCustomBits & _ulFilterCustomBits) != 0) {
                f32 dx;
                f32 dy;
                f32 dz;
                f32 distance;
                POS_tdstMatrix_* position = candidate->gu_hGlobalMatrix;

                candidatePosition = *inv_POS_fn_p_stPosMatrixTranslation(position);
                dx = candidatePosition.xX - source.xX;
                dy = candidatePosition.xY - source.xY;
                dz = candidatePosition.xZ - source.xZ;
                distance = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    best = candidate;
                }
            }
        }
        node = node->next;
    }

    return best;
}

void* fn_p_stGetNearestActorInCurrentSector(HIE_tdstSuperObject_* superObject,
                                            tdstNodeInterpret_* stream, tdstGetSetParam_* param)
{
    HIE_tdstSuperObject_* obj;
    HIE_tdstSuperObject_* node;
    s32 value;
    s32 result;

    fn_vGetUltraOperatorPerso(superObject, &obj);
    stream = fn_p_stEvalTree(superObject, stream, param);
    if (param->ucType == E_vt_Float) {
        value = inv_MTH_fn_lToIntegerOrdered(param->u.xValue);
    } else {
        value = param->u.lValue;
    }
    node = fn_hGetNearestActorInSectorWithAnyCustombit(obj,
                         (u32) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) obj)->hLinkedObject)
                             ->h_SectInfo->hCurrentSector,
                         value);
    if (node != (void*) 0) {
        if (node->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            result = (s32) node->hLinkedObject;
        } else {
            result = 0;
        }
    } else {
        result = 0;
    }
    param->u.lValue = result;
    param->ucType = E_vt_Perso;
    return stream;
}

tdstNodeInterpret_* fn_p_stDotProduct(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                      tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ save;
    f32 dot;
    tdstNodeInterpret_* id;

    id = fn_p_stEvalTree(superObject, stream, param);
    save = *(MTH3D_tdstVector_*) param;
    id = fn_p_stEvalTree(superObject, id, param);
    dot = inv_MTH_fn_xMulAdd3(save.xX, param->u.stVector.xX, save.xY,
                   param->u.stVector.xY, save.xZ, param->u.stVector.xZ);
    param->ucType = E_vt_Float;
    param->u.xValue = dot;
    return id;
}

tdstNodeInterpret_* fn_p_stGetCPUCounter(HIE_tdstSuperObject_* superObject,
                                         tdstNodeInterpret_* stream, tdstGetSetParam_* outParam)
{
    s32 type;
    type = 6;
    outParam->u.lValue = 0;
    outParam->ucType = (s8) type;
    return stream;
}

tdstNodeInterpret_* fn_p_stFunctTurnToPosition(HIE_tdstSuperObject_* superObject,
                                               tdstNodeInterpret_* stream, tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ dir;
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ ref;
    MTH3D_tdstVector_ xaxis;
    MTH3D_tdstVector_ yaxis;
    MTH3D_tdstVector_ zaxis;
    POS_tdstMatrix_ inverse;
    HIE_tdstSuperObject_* obj;
    POS_tdstMatrix_* xf;
    tdstDynam_* sub;
    f32 limit;
    f32 len;
    f32 len2;
    f32 invLen;
    f32 dot;
    f32 angle;
    f32 reduced;
    f32 sign;
    f32 base;
    f32 sine;
    f32 xCosSquareComplement;
    f32 absReduced;
    f32 result;
    s32 lTurns;
    u32 bits;
    s32 flags;

    fn_vGetUltraOperatorPerso(superObject, &obj);
    xf = obj->gu_hGlobalMatrix;
    POS_fn_vInvertIsoMatrix(&inverse, xf);
    sub = ((tdstEngineObject_*) obj->hLinkedObject)->h_Dynam;

    stream = fn_p_stEvalTree(superObject, stream, param);
    target = param->u.stVector;
    stream = fn_p_stEvalTree(superObject, stream, param);

    MTH3D_M_vSetXYZ_inv(&ref, 0.0f, -1.0f, 0.0f);
    angle = ref.xX;
    MTH3D_M_vSetXYZ_inv(&zaxis, angle, angle, 1.0f);

    pos = *(MTH3D_tdstVector_*) &xf->stMatrix.stCol_3;
    inv_MTH3D_fn_vSubXYZ(&dir, &target, &pos);
    limit = param->u.xValue * MTH_DEG_TO_RAD_inv;

    POS_fn_vApplyMatrixToVector(&dir, &inverse, &dir);
    dir.xZ = angle;
    len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(dir.xX, dir.xY, dir.xZ);
    len = MTH_fn_xSqrtOpt2(len2);
    if (len != angle) {
        invLen = 1.0f / len;
        inv_MTH3D_fn_vKTimesXYZ(&dir, invLen, &dir);
    }

    dot = inv_MTH_fn_xMulAdd3Grouped(ref.xX, ref.xY, ref.xZ, dir.xX, dir.xY, dir.xZ);
    if (dot > 1.0f) {
        dot = 1.0f;
    } else if (dot < -1.0f) {
        dot = -1.0f;
    }

    angle = acosf(dot);
    if ((ref.xX * dir.xY - ref.xY * dir.xX) < 0.0f) {
        angle = inv_MTH_fn_xNegate(angle);
    }
    if (limit < angle) {
        angle = limit;
    } else {
        f32 negLimit;
        negLimit = inv_MTH_fn_xNegate(limit);
        if (angle < negLimit) {
            angle = negLimit;
        }
    }

    sign = MTH_TWO_PI_inv;
    lTurns = inv_MTH_fn_lToIntegerOrdered(angle * MTH_INV_TWO_PI_inv);
    reduced = angle - (f32) lTurns * sign;
    dot = cosf(angle);
    sign = MTH_PI_inv;

    absReduced = inv_MTH_fn_xAbsolute(reduced);
    if (absReduced < sign) {
        sign = inv_MTH_fn_xSign(reduced);
    } else {
        sign = -inv_MTH_fn_xSign(reduced);
    }

    xCosSquareComplement = 1.0f - MTH_fn_xSqrOpt1(dot);
    sine = MTH_fn_xSqrtOpt2(xCosSquareComplement);
    sine *= sign;

    MTH3D_M_vSetXYZ_inv(&xaxis, dot, sine, 0.0f);
    MTH3D_M_vSetXYZ_inv(&yaxis, -sine, dot, 0.0f);

    POS_fn_vRotateVector(&xaxis, xf, &xaxis);
    POS_fn_vRotateVector(&yaxis, xf, &yaxis);
    POS_fn_vRotateVector(&zaxis, xf, &zaxis);
    _POS_fn_vSetRotationMatrix(xf, &xaxis, &yaxis, &zaxis);

    obj->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
    HIE_fn_vComputeNewRelativeMatrix(obj);
    if (obj->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv ||
        ((flags = fn_ulStandardGameGetCustomBitsSO(obj)),
         ((flags & 2) != 0 && (sub == 0 || (flags & 0x8000) != 0)))) {
        HIE_fn_vRefreshHierarchy(obj);
    } else {
        HIE_fn_vInvalidateHierarchyFrame(obj);
    }

    if (sub != 0) {
        MEC_vImposeRotationMatrix(sub->p_stDynamics, xf);
    }

    result = angle * MTH_RAD_TO_DEG_inv;
    param->ucType = E_vt_Float;
    param->u.xValue = result;
    return stream;
}

tdstNodeInterpret_* func_001EC8F0(HIE_tdstSuperObject_* context, tdstNodeInterpret_* stream,
                                  tdstGetSetParam_* outParam)
{
    HIE_tdstSuperObject_* saved;
    HIE_tdstSuperObject_* generator;
    tdstGetSetParam_* p_stValue;
    s32 value;

    fn_vGetUltraOperatorPerso(context, &saved);
    generator = inv_ALWAYS_fn_hGetGeneratorOfAlways(saved);
    if (generator == 0) {
        value = 0;
    } else {
        generator = inv_ALWAYS_fn_hGetGeneratorOfAlways(saved);
        if (generator->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
            value = 0;
        } else {
            generator = inv_ALWAYS_fn_hGetGeneratorOfAlways(saved);
            value = (s32) generator->hLinkedObject;
        }
    }
    p_stValue = outParam;
    p_stValue->u.lValue = value;
    p_stValue->ucType = E_vt_Perso;
    return stream;
}

void* func_001EC968(void* superObject, void* stream, tdstGetSetParam_* out)
{
    void* p_stNext;
    s32 lIndex;
    s16 index;
    u16 op;

    p_stNext = stream;
    op = AI_NODE_ID(p_stNext);
    p_stNext = fn_p_stEvalTree(superObject, p_stNext, out);

    if (out->ucType == E_vt_Float) {
        lIndex = inv_MTH_fn_lToIntegerOrdered(out->u.xValue);
        index = (s16) lIndex;
    } else {
        index = *&out->u.lValue;
    }

    switch (op) {
    case 0xAF:
        out->u.stVector = qword_383BD0[index].left.stGlobalVector;
        out->ucType = E_vt_Vector;
        break;
    case 0xB0:
        out->u.lValue = qword_383BD0[index].left.wHorizontalAxis;
        out->ucType = E_vt_Integer;
        break;
    case 0xB1:
        out->u.lValue = qword_383BD0[index].left.wVerticalAxis;
        out->ucType = E_vt_Integer;
        break;
    case 0xB2:
        out->u.xValue = qword_383BD0[index].left.xAnalogForce;
        out->ucType = E_vt_Float;
        break;
    case 0xB3:
        out->u.xValue = qword_383BD0[index].left.xTrueAnalogForce;
        out->ucType = E_vt_Float;
        break;
    case 0xB4:
        out->u.xValue = qword_383BD0[index].left.xRotationAngle;
        out->ucType = E_vt_Float;
        break;
    case 0xB5:
        out->u.lValue = qword_383BD0[index].left.lSector;
        out->ucType = E_vt_Integer;
        break;
    case 0xB6:
        *(MTH3D_tdstVector_*) out = *(MTH3D_tdstVector_*) (unk_383BF0 + index * 0x40);
        out->ucType = E_vt_Vector;
        break;
    case 0xB7:
        out->u.lValue = qword_383BD0[index].right.wHorizontalAxis;
        out->ucType = E_vt_Integer;
        break;
    case 0xB8:
        out->u.lValue = qword_383BD0[index].right.wVerticalAxis;
        out->ucType = E_vt_Integer;
        break;
    case 0xB9:
        out->u.xValue = qword_383BD0[index].right.xAnalogForce;
        out->ucType = E_vt_Float;
        break;
    case 0xBA:
        out->u.xValue = qword_383BD0[index].right.xTrueAnalogForce;
        out->ucType = E_vt_Float;
        break;
    case 0xBB:
        out->u.xValue = qword_383BD0[index].right.xRotationAngle;
        out->ucType = E_vt_Float;
        break;
    case 0xBC:
        out->u.lValue = qword_383BD0[index].right.lSector;
        out->ucType = E_vt_Integer;
        break;
    }
    return p_stNext;
}

void* fn_p_stGetDTFunction(void* context, void* result, tdstGetSetParam_* out)
{
    f32 delta;

    delta = inv_fn_xGetEngineClockDelta();
    out->ucType = E_vt_Float;
    out->u.xValue = delta * 1000.0f;
    return result;
}

void* fn_p_stGetNormalCollideVector(HIE_tdstSuperObject_* p_SuperObjPerso,
                                    tdstNodeInterpret_* p_stTree, tdstGetSetParam_* out)
{
    MTH3D_tdstVector_ stNormalSum;
    HIE_tdstSuperObject_* saved;
    DNM_stReport_* set;
    DNM_stObstacle* entry;
    u32 flags;
    u16 op;
    f32 zero;
    f32 dot, norm, inv, x, y, z;

    op = *(u16*) ((u8*) p_stTree - 8);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&stNormalSum, zero, zero, zero);
    set = fn_pstGetDNMReport(saved);

    if (op != 0xDA) {
        if (op != 0xDB) {
            return p_stTree;
        }
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, out);
        switch (out->u.lValue) {
        default:
            out->u.stVector = stNormalSum;
            out->ucType = E_vt_Vector;
            return p_stTree;
        case 0:
            entry = &set->m_stObstacle;
            break;
        case 1:
            entry = &set->m_stGround;
            break;
        case 2:
            entry = &set->m_stWall;
            break;
        case 3:
            entry = &set->m_stCharacter;
            break;
        case 4:
            entry = &set->m_stWater;
            break;
        case 5:
            entry = &set->m_stCeil;
            break;
        }
        out->u.stVector = entry->m_stNorm;
        out->ucType = E_vt_Vector;
        return p_stTree;
    }
    flags = set->m_ulCurrSurfaceState;
    if (flags & 0x10000) {
        entry = &set->m_stCharacter;
        MTH3D_M_vAddXYZ_inv(&stNormalSum, &stNormalSum, &entry->m_stNorm);
    }
    if (flags & 1) {
        entry = &set->m_stGround;
        MTH3D_M_vAddXYZ_inv(&stNormalSum, &stNormalSum, &entry->m_stNorm);
    }
    if (flags & 4) {
        entry = &set->m_stWall;
        MTH3D_M_vAddXYZ_inv(&stNormalSum, &stNormalSum, &entry->m_stNorm);
    }
    if (flags & 0x40) {
        entry = &set->m_stWater;
        MTH3D_M_vAddXYZ_inv(&stNormalSum, &stNormalSum, &entry->m_stNorm);
    }
    if (flags & 0x10) {
        entry = &set->m_stCeil;
        MTH3D_M_vAddXYZ_inv(&stNormalSum, &stNormalSum, &entry->m_stNorm);
    }
    if (flags == 0) {
        entry = &set->m_stObstacle;
        MTH3D_M_vAddXYZ_inv(&stNormalSum, &stNormalSum, &entry->m_stNorm);
    }
    if (stNormalSum.xX != zero || stNormalSum.xY != zero || stNormalSum.xZ != zero) {
        x = stNormalSum.xX;
        y = stNormalSum.xY;
        z = stNormalSum.xZ;
        dot = x;
        dot = MTH_fn_xSqrAddSqrAddSqrOpt2(dot, y, z);
        norm = MTH_fn_xSqrtOpt2(dot);
        if (norm != 0.0f) {
            inv = 1.0f / norm;
            MTH3D_M_vSetXYZ_inv(&stNormalSum, inv * x, inv * y, inv * z);
        }
    }
    out->u.stVector = stNormalSum;
    out->ucType = E_vt_Vector;
    return p_stTree;
}

void* fn_p_stCollideCommunicationFunc(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                      tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ stSelected;
    MTH3D_tdstVector_ stZero;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    CollisionSet* values;
    u16 op;
    s32 scalar;

    s32* node;
    s32* first_node;
    s32* value_node;
    s32 result;
    f32 number;

    op = AI_NODE_ID(stream);
    scalar = 0;
    fn_vGetUltraOperatorPerso(superObject, &hNewSuperObjPerso);
    values = inv_HIE_fn_p_stGetEngineObject(hNewSuperObjPerso)->h_CollSet;
    switch (op) {
    case eFunc_GetColliderType:
        if (values != 0)
            scalar = values->stColliderInfo.ucColliderType;
        param->u.lValue = scalar;
        param->ucType = E_vt_Integer;
        break;
    case eFunc_GetColliderActor:
        stream = fn_p_stEvalTree(superObject, stream, param);
        scalar = (s16) param->u.lValue;
        if (values != 0) {
            first_node = ((u16) scalar < 2) ? values->stColliderInfo.a_stColliderActors[scalar] : 0;
            if (first_node != 0) {
                node = ((u16) scalar < 2) ? values->stColliderInfo.a_stColliderActors[scalar] : 0;
                if (*node == 2) {
                    value_node =
                        ((u16) scalar < 2) ? values->stColliderInfo.a_stColliderActors[scalar] : 0;
                    result = value_node[1];
                } else
                    result = 0;
            } else
                result = 0;
            param->u.lValue = result;
            param->ucType = E_vt_Perso;
            break;
        } else {
            param->u.lValue = 0;
            param->ucType = E_vt_Perso;
            break;
        }
    case eFunc_GetColliderVector: {
        tdstGetSetParam_* destination = param;
        stream = fn_p_stEvalTree(superObject, stream, param);
        scalar = (s16) param->u.lValue;
        if (values != 0) {
            memset(&stZero, 0, 12);
            if ((u16) scalar < 2)
                stSelected = values->stColliderInfo.a_stColliderVectors[scalar];
            else
                stSelected = stZero;
            destination->u.stVector = stSelected;
        } else {
            f32 zero;
            zero = 0.0f;
            MTH3D_M_vSetXYZ_inv(&param->u.stVector, zero, zero, zero);
        }
        param->ucType = E_vt_Vector;
        break;
    }
    case eFunc_GetColliderReal:
        stream = fn_p_stEvalTree(superObject, stream, param);
        scalar = (s16) param->u.lValue;
        if (values != 0) {
            number = ((u16) scalar < 2) ? values->stColliderInfo.a_xColliderReals[scalar] : 0.0f;
            param->u.xValue = number;
        } else
            param->u.lValue = 0;
        param->ucType = E_vt_Float;
        break;
    default:
        return stream;
    }
    return stream;
}

static inline void inv_AI_fn_vNegateVector(MTH3D_tdstVector_* v)
{
    f32 x = v->xX;
    f32 y = v->xY;
    f32 z = v->xZ;
    x = inv_MTH_fn_xNegateTied(x);
    y = inv_MTH_fn_xNegateTied(y);
    z = inv_MTH_fn_xNegateTied(z);
    MTH3D_M_vSetXYZ_inv(v, x, y, z);
}

void* fn_p_stGetPersoSighting(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                              tdstGetSetParam_* param)
{
    HIE_tdstSuperObject_* resolved;
    fn_vGetUltraOperatorPerso(superObject, &resolved);
    POS_fn_vGetYAxisVector(resolved->gu_hGlobalMatrix, param);
    inv_AI_fn_vNegateVector(&param->u.stVector);
    param->ucType = E_vt_Vector;
    return stream;
}

static inline HIE_tdstSuperObject_* inv_AI_fn_p_stCsoSuperObject(HIE_tdstSuperObject_* resolved, u16 index)
{
    tdstEngineObject_* owner = resolved->hLinkedObject;
    tdstAChannel_* entries = (tdstAChannel_*) owner->h_3dData->hArrayOfChannels;

    if (entries != 0)
        return entries[index].hSupObject;
    return 0;
}

void* fn_p_st3DFunc(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                    tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ result;
    MTH3D_tdstVector_ basisX;
    MTH3D_tdstVector_ basisY;
    HIE_tdstSuperObject_* resolved;
    void* matrix;
    tdstEngineObject_* owner;
    HIE_tdstSuperObject_* candidate;
    u16 op;
    s32 use24;
    u8 objectIndex;
    f32 zero;

    use24 = 1;
    op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &resolved);

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&result, zero, zero, zero);

    switch (op) {
    case eFunc_GetPersoZoomFactor:
        use24 = 0;
        break;

    case eFunc_GetPersoHorizon:
    case eFunc_GetPersoBanking:
        break;

    case eFunc_GetModuleRelativePosition:
    case eFunc_GetModuleZoomFactor:
        use24 = 0;

    case eFunc_GetModuleAbsolutePosition:
    case 0xF3:
        stream = fn_p_stEvalTree(superObject, stream, param);
        objectIndex = *(u8*) &param->u;

        if (objectIndex == 0xFF) {
            param->u.stVector = result;
            param->ucType = E_vt_Vector;
            return fn_p_stSwapNextParameter(stream);
        }

        candidate = inv_AI_fn_p_stCsoSuperObject(resolved, objectIndex);
        if (candidate != 0)
            resolved = candidate;

        if (op == 0xF3) {
            stream = fn_p_stEvalTree(superObject, stream, param);
            use24 = param->u.lValue;
        }
        break;

    default:
        return stream;
    }

    if (use24 != 0)
        matrix = resolved->gu_hGlobalMatrix;
    else
        matrix = resolved->gu_hLocalMatrix;

    switch (op) {
    case eFunc_GetPersoBanking: {
        tdstDynam_* obj;

        owner = resolved->hLinkedObject;
        obj = owner->h_Dynam;

        if (obj != 0 && (obj->p_stDynamics->m_tdstDynamicsBaseBlock.ulFlags & 8) != 0) {
            MTH3D_M_vSetXYZ_inv(&result, 0.0f, 0.0f, 1.0f);
        } else {
            POS_fn_vGetRotationVectors(matrix, &basisX, &basisY, &result);
        }
        break;
    }

    case eFunc_GetPersoHorizon:
        inv_POS_fn_vGetXAxisVector(matrix, &result);
        break;

    case 0xF3:
        POS_fn_vGetYAxisVector(matrix, &result);
        MTH3D_M_vSetXYZ_inv(&result, inv_MTH_fn_xNegateTied(result.xX),
                            inv_MTH_fn_xNegateTied(result.xY), inv_MTH_fn_xNegateTied(result.xZ));
        break;

    case eFunc_GetPersoZoomFactor:
    case eFunc_GetModuleZoomFactor:
        POS_fn_vGetScaleMatrix(matrix, &result);
        break;

    case eFunc_GetModuleAbsolutePosition:
    case eFunc_GetModuleRelativePosition:
        result = *(MTH3D_tdstVector_*) &((POS_tdstMatrix_*) matrix)->stMatrix.stCol_3;
        break;

    default:
        break;
    }

    param->u.stVector = result;
    param->ucType = E_vt_Vector;
    return stream;
}

tdstNodeInterpret_* func_001ED4B0(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                  tdstGetSetParam_* param)
{
    char* text;
    s16 position;
    s16 index;
    s32 bias;
    s32 character;

    if (*(u16*) ((u8*) stream - 8) == 0xF5) {
        stream = fn_p_stEvalTree(superObject, stream, param);
        text = fn_szGetStringFromTextOrStringParam(param);
        stream = fn_p_stEvalTree(superObject, stream, param);
        index = *&param->u.lValue;
        position = 0;
        while (text[position++] == '/') {
            while (text[position++] != ':') {
            }
        }
        bias = index - 1;
        index = (s16) (position + bias);
        if (index >= 0 && index < (s16) strlen(text)) {
            character = text[index];
            param->ucType = E_vt_Integer;
            param->u.lValue = character;
        } else {
            param->u.lValue = 0;
            param->ucType = E_vt_Integer;
        }
    }
    return stream;
}

s32 inv_fn_uwGetObstacleCollideIdentifier(DNM_stObstacle* record)
{
    GMT_tdstGameMaterial_* handle;
    GMT_tdstCollideMaterial_* id;

    if (record == 0) {
        return 0;
    }
    handle = record->hCollidedMaterial;
    if (!GMT_fn_b_ulIsValid(handle)) {
        return 0;
    }
    id = GMT_fn_hGetCollideMaterial(handle);
    if (id != 0 && id != (GMT_tdstCollideMaterial_*) -1) {
        return GMT_fn_hGetCollideMaterialIdentifier(id);
    }
    return 0;
}

static inline void inv_AI_fn_vSetFloatResult(tdstGetSetParam_* o, f32 v)
{
    o->u.xValue = v;
    o->ucType = E_vt_Float;
}

tdstNodeInterpret_* fn_p_stGetCollidePoint(HIE_tdstSuperObject_* owner, tdstNodeInterpret_* stream,
                                           tdstGetSetParam_* out)
{
    u32 flags;
    u16 mask;
    DNM_stReport_* resolved;
    DNM_stObstacle* p_stSlot;
    HIE_tdstSuperObject_* holder;
    MTH3D_tdstVector_ scratch;
    u16 op;

    mask = 0;
    op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(owner, &holder);
    resolved = fn_pstGetDNMReport(holder);

    switch (op) {
    case 0xE1:
        flags = resolved->m_ulCurrSurfaceState;
        if ((flags & 0x10000) != 0) {
            mask = (u16) inv_fn_uwGetObstacleCollideIdentifier(&resolved->m_stCharacter);
        }
        if ((flags & 0x1) != 0) {
            mask = (u16) (mask | inv_fn_uwGetObstacleCollideIdentifier(&resolved->m_stGround));
        }
        if ((flags & 0x4) != 0) {
            mask = (u16) (mask | inv_fn_uwGetObstacleCollideIdentifier(&resolved->m_stWall));
        }
        if ((flags & 0x40) != 0) {
            mask = (u16) (mask | inv_fn_uwGetObstacleCollideIdentifier(&resolved->m_stWater));
        }
        if ((flags & 0x10) != 0) {
            mask = (u16) (mask | inv_fn_uwGetObstacleCollideIdentifier(&resolved->m_stCeil));
        }
        if (flags == 0) {
            mask = (u16) (mask | inv_fn_uwGetObstacleCollideIdentifier(&resolved->m_stObstacle));
        }
        out->u.ulValue = mask;
        out->ucType = E_vt_Mask;
        return stream;

    case eFunc_GetCollideMaterialType2:
        stream = fn_p_stEvalTree(owner, stream, out);
        switch (out->u.ulValue) {
        default:
            out->u.xValue = 0.0f;
            out->ucType = E_vt_Mask;
            return stream;
        case 0:
            p_stSlot = &resolved->m_stObstacle;
            break;
        case 1:
            p_stSlot = &resolved->m_stGround;
            break;
        case 2:
            p_stSlot = &resolved->m_stWall;
            break;
        case 3:
            p_stSlot = &resolved->m_stCharacter;
            break;
        case 4:
            p_stSlot = &resolved->m_stWater;
            break;
        case 5:
            p_stSlot = &resolved->m_stCeil;
            break;
        }
        out->u.ulValue = inv_fn_uwGetObstacleCollideIdentifier(p_stSlot);
        out->ucType = E_vt_Mask;
        return stream;

    case eFunc_GetCollidePoint2:
        memset(&scratch, 0, 12);
        stream = fn_p_stEvalTree(owner, stream, out);
        switch (out->u.ulValue) {
        default:
            out->u.stVector = scratch;
            out->ucType = E_vt_Vector;
            return stream;
        case 0:
            p_stSlot = &resolved->m_stObstacle;
            break;
        case 1:
            p_stSlot = &resolved->m_stGround;
            break;
        case 2:
            p_stSlot = &resolved->m_stWall;
            break;
        case 3:
            p_stSlot = &resolved->m_stCharacter;
            break;
        case 4:
            p_stSlot = &resolved->m_stWater;
            break;
        case 5:
            p_stSlot = &resolved->m_stCeil;
            break;
        }
        out->u.stVector = p_stSlot->m_stContact;
        out->ucType = E_vt_Vector;
        return stream;

    case eFunc_GetHandsCollidePoint:
        out->ucType = E_vt_Vector;
        if (GMT_fn_xTestCollideIdentifierTypeOfGameMaterial(resolved->m_stGround.hCollidedMaterial, 2) != 0) {
            out->u.stVector = resolved->m_stGround.m_stContact;
            break;
        }
        out->u.stVector.xZ = out->u.stVector.xY = out->u.stVector.xX = 0.0f;
        break;

    case eFunc_GetCollidePoint:
        out->u.stVector = resolved->m_stObstacle.m_stContact;
        out->ucType = E_vt_Vector;
        return stream;

    case eFunc_GetCollideRate2:
        stream = fn_p_stEvalTree(owner, stream, out);
        switch (out->u.ulValue) {
        default:
            out->u.ulValue = 0;
            out->ucType = E_vt_Float;
            return stream;
        case 0:
            p_stSlot = &resolved->m_stObstacle;
            break;
        case 1:
            p_stSlot = &resolved->m_stGround;
            break;
        case 2:
            p_stSlot = &resolved->m_stWall;
            break;
        case 3:
            p_stSlot = &resolved->m_stCharacter;
            break;
        case 4:
            p_stSlot = &resolved->m_stWater;
            break;
        case 5:
            p_stSlot = &resolved->m_stCeil;
            break;
        }
        out->u.xValue = p_stSlot->m_xRate;
        out->ucType = E_vt_Float;
        return stream;

    case eFunc_GetCollideRate:
        inv_AI_fn_vSetFloatResult(out, 1.0f);
        inv_AI_fn_vSetFloatResult(out, resolved->m_stObstacle.m_xRate);
        return stream;
    default:
        return stream;
    }
    return stream;
}

tdstNodeInterpret_* fn_p_stGetLastCollisionActor(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                                 tdstGetSetParam_* outParam)
{
    tdstNodeInterpret_* status;
    s32 value;
    tdstEngineObject_* node;
    gu_tdstStandardGame* selector;
    HIE_tdstSuperObject_* desc;

    status = fn_p_stEvalTree(superObject, stream, outParam);
    node = (tdstEngineObject_*) outParam->u.lValue;
    selector = node->h_StandardGame;
    desc = fn_hSuperObjectGetLastCollidedCharacter(selector->h_SuperObject);
    if (desc != 0) {
        if (desc->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
            value = 0;
        } else {
            value = (s32) desc->hLinkedObject;
        }
    } else {
        value = 0;
    }
    outParam->u.lValue = value;
    outParam->ucType = E_vt_Perso;
    return status;
}

void* fn_p_stGetCollision(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                          tdstGetSetParam_* out)
{
    MTH3D_tdstVector_ saved;
    s32 changed;
    s32 mode;
    u16 op;
    s32 value;

    changed = 0;
    op = AI_NODE_ID(stream);

    stream = fn_p_stEvalTree(superObject, stream, out);
    {
        MTH3D_tdstVector_* p_stParamVector = &out->u.stVector;
        if (g_stCollideParameterPoint[0].xX != p_stParamVector->xX || g_stCollideParameterPoint[0].xY != p_stParamVector->xY ||
            g_stCollideParameterPoint[0].xZ != p_stParamVector->xZ) {
            g_stCollideParameterPoint[0] = *p_stParamVector;
            changed = 1;
        }
    }

    stream = fn_p_stEvalTree(superObject, stream, out);
    {
        MTH3D_tdstVector_* p_stParamVector = &out->u.stVector;
        if (g_stCollideParameterVector[0].xX != p_stParamVector->xX || g_stCollideParameterVector[0].xY != p_stParamVector->xY ||
            g_stCollideParameterVector[0].xZ != p_stParamVector->xZ) {
            g_stCollideParameterVector[0] = *p_stParamVector;
            changed = 1;
        }
    }

    stream = fn_p_stEvalTree(superObject, stream, out);
    mode = out->u.lValue;

    if (changed || g_ulCollideTimeCounter != inv_fn_ulGetEngineClock() ||
        g_ulMode != mode) {
        saved = g_stCollideParameterVector[0];
        g_ulCollideTimeCounter = inv_fn_ulGetEngineClock();
        g_ulMode = mode;
        g_hSuperObjectHit = 0;
        g_xMask = 0;
        func_001E7348(superObject);
        g_stCollideParameterVector[0] = saved;
    }

    switch (op) {
    case eFunc_GetCollisionPoint:
        out->u.stVector = g_stCollideResultPoint[0];
        out->ucType = E_vt_Vector;
        break;
    case eFunc_GetCollisionVector:
        out->u.stVector = g_stCollideResultVector[0];
        out->ucType = E_vt_Vector;
        break;
    case eFunc_GetCollisionPerso:
        if (g_hSuperObjectHit != 0) {
            if (g_hSuperObjectHit->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
                value = 0;
            } else {
                value = (s32) g_hSuperObjectHit->hLinkedObject;
            }
        } else {
            value = 0;
        }
        out->u.lValue = value;
        out->ucType = E_vt_Perso;
        break;
    }

    g_ulOverRideRayTraceSpoMask = 0x40;
    return stream;
}

void* fn_p_stGetCollisionWithCollideMaterial(HIE_tdstSuperObject_* superObject,
                                             tdstNodeInterpret_* stream, tdstGetSetParam_* out)
{
    MTH3D_tdstVector_ saved;
    s32 changed;
    s32 mode;
    u16 op;

    changed = 0;

    stream = fn_p_stEvalTree(superObject, stream, out);
    {
        MTH3D_tdstVector_* p_stParamVector = &out->u.stVector;
        if (g_stCollideParameterPoint[0].xX != p_stParamVector->xX || g_stCollideParameterPoint[0].xY != p_stParamVector->xY ||
            g_stCollideParameterPoint[0].xZ != p_stParamVector->xZ) {
            g_stCollideParameterPoint[0] = *p_stParamVector;
            changed = 1;
        }
    }

    stream = fn_p_stEvalTree(superObject, stream, out);
    {
        MTH3D_tdstVector_* p_stParamVector = &out->u.stVector;
        if (g_stCollideParameterVector[0].xX != p_stParamVector->xX || g_stCollideParameterVector[0].xY != p_stParamVector->xY ||
            g_stCollideParameterVector[0].xZ != p_stParamVector->xZ) {
            g_stCollideParameterVector[0] = *p_stParamVector;
            changed = 1;
        }
    }

    stream = fn_p_stEvalTree(superObject, stream, out);
    mode = out->u.lValue;

    stream = fn_p_stEvalTree(superObject, stream, out);
    op = *(u16*) out;

    if (changed || g_ulCollideTimeCounter != inv_fn_ulGetEngineClock() ||
        g_ulMode != mode) {
        g_xMask = op;
        saved = g_stCollideParameterVector[0];
        g_ulCollideTimeCounter = inv_fn_ulGetEngineClock();
        g_ulMode = mode;
        g_hSuperObjectHit = 0;
        func_001E7348(superObject);
        g_xMask = 0;
        g_stCollideParameterVector[0] = saved;
    }

    g_ulOverRideRayTraceSpoMask = 0x40;
    out->u.stVector = g_stCollideResultPoint[0];
    out->ucType = E_vt_Vector;
    return stream;
}

void* fn_p_stComputeRebondVector(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                 tdstGetSetParam_* param)
{
    MTH3D_tdstVector_ normal;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ result;
    f32 factor;
    f32 projection;

    stream = fn_p_stEvalTree(superObject, stream, param);
    direction = param->u.stVector;
    stream = fn_p_stEvalTree(superObject, stream, param);
    normal = param->u.stVector;
    stream = fn_p_stEvalTree(superObject, stream, param);
    if (param->ucType == E_vt_Float) {
        factor = param->u.xValue;
    } else {
        factor = (f32) param->u.lValue;
    }
    projection = inv_MTH_fn_xMulAdd3(direction.xX, normal.xX, direction.xY,
                          normal.xY, direction.xZ, normal.xZ);
    projection = projection * -2.0f;
    MTH3D_M_vSetXYZ_inv(&result, factor * (projection * normal.xX + direction.xX),
                        factor * (projection * normal.xY + direction.xY),
                        factor * (projection * normal.xZ + direction.xZ));
    param->u.stVector = result;
    param->ucType = E_vt_Vector;
    return stream;
}

void* fn_p_stVectorAndAngle(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                            tdstGetSetParam_* out)
{
#ifndef NULL
#define NULL ((void*) 0)
#endif

    MTH3D_tdstVector_ stVect1;
    MTH3D_tdstVector_ stVect2;
    MTH3D_tdstVector_ cross;
    u16 op;
    s32 zeroVec;
    f32 x;
    f32 len2;
    f32 dot;
    f32 angle;
    f32 mag;
    f32 zero;
    f32 len2a, len2b, lena, lenb, inva, invb, zeroa, zerob;

    zeroVec = 0;
    op = AI_NODE_ID(stream);
    stream = fn_p_stEvalTree(superObject, stream, out);
    stVect1 = out->u.stVector;
    stream = fn_p_stEvalTree(superObject, stream, out);
    stVect2 = out->u.stVector;

    if (__builtin_fabsf(stVect1.xX) < 1e-05f && __builtin_fabsf(stVect1.xY) < 1e-05f &&
        __builtin_fabsf(stVect1.xZ) < 1e-05f)
        goto a_zero;
a_norm:
    len2a = MTH_fn_xSqrAddSqrAddSqrOpt2(stVect1.xX, stVect1.xY, stVect1.xZ);
    lena = MTH_fn_xSqrtOpt2(len2a);
    zeroa = 0.0f;
    if (lena != zeroa) {
        inva = 1.0f / lena;
        inv_MTH3D_fn_vKTimesXYZ(&stVect1, inva, &stVect1);
    }
    goto a_done;
a_zero:
    zeroVec = 1;
a_done:

    if (__builtin_fabsf(stVect2.xX) < 1e-05f && __builtin_fabsf(stVect2.xY) < 1e-05f &&
        __builtin_fabsf(stVect2.xZ) < 1e-05f)
        goto b_zero;
b_norm:
    len2b = MTH_fn_xSqrAddSqrAddSqrOpt2(stVect2.xX, stVect2.xY, stVect2.xZ);
    lenb = MTH_fn_xSqrtOpt2(len2b);
    zerob = 0.0f;
    if (lenb != zerob) {
        invb = 1.0f / lenb;
        inv_MTH3D_fn_vKTimesXYZ(&stVect2, invb, &stVect2);
    }
    goto b_done;
b_zero:
    zeroVec = 1;
b_done:

    switch (op) {
    case eFunc_VectorAngle:
        if (zeroVec != 0) {
            stream = fn_p_stSkipThisArgument(stream);
            angle = 0.0f;
        } else {
            stream = fn_p_stEvalTree(superObject, stream, out);
            op = *(u8*) &out->u;
            dot = inv_MTH3D_fn_xDotXYZ(&stVect1, &stVect2);
            if (dot > 1.0f)
                dot = 1.0f;
            else if (dot < -1.0f)
                dot = -1.0f;
            angle = acosf(dot);
            switch (op) {
            case 0:
                x = stVect1.xX * stVect2.xY - stVect1.xY * stVect2.xX;
                if (x < 0.0f)
                    angle = inv_MTH_fn_xNegateTied(angle);
                break;
            case 1:
                x = stVect1.xY * stVect2.xZ - stVect1.xZ * stVect2.xY;
                if (x < 0.0f)
                    angle = inv_MTH_fn_xNegateTied(angle);
                break;
            case 2:
                x = stVect1.xZ * stVect2.xX - stVect1.xX * stVect2.xZ;
                if (x < 0.0f)
                    angle = inv_MTH_fn_xNegateTied(angle);
                break;
            }
            angle = angle * MTH_RAD_TO_DEG_inv;
        }
        out->u.xValue = angle;
        out->ucType = E_vt_Float;
        break;

    case eFunc_VectorCos:
        if (zeroVec != 0) {
            dot = 1.0f;
        } else {
            dot = inv_MTH3D_fn_xDotXYZ(&stVect1, &stVect2);
            if (dot > 1.0f)
                dot = 1.0f;
            else if (dot < -1.0f)
                dot = -1.0f;
        }
        out->u.xValue = dot;
        out->ucType = E_vt_Float;
        break;

    case eFunc_VectorSin:
        if (zeroVec != 0) {
            stream = fn_p_stSkipThisArgument(stream);
            mag = 0.0f;
        } else {
            stream = fn_p_stEvalTree(superObject, stream, out);
            DNM_p_stVectorCrossProduct(&cross, &stVect1, &stVect2);
            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(cross.xX, cross.xY, cross.xZ);
            mag = MTH_fn_xSqrtOpt2(len2);
            op = *(u8*) &out->u;
            switch (op) {
            case 0:
                if (cross.xZ < 0.0f)
                    mag = inv_MTH_fn_xNegateTied(mag);
                break;
            case 1:
                if (cross.xX < 0.0f)
                    mag = inv_MTH_fn_xNegateTied(mag);
                break;
            case 2:
                if (cross.xY < 0.0f)
                    mag = inv_MTH_fn_xNegateTied(mag);
                break;
            }
            if (mag > 1.0f)
                mag = 1.0f;
            else if (mag < -1.0f)
                mag = -1.0f;
        }
        out->u.xValue = mag;
        out->ucType = E_vt_Float;
        break;

    default:
        return stream;
    }

    return stream;
}

tdstNodeInterpret_* fn_p_GetMechanicParameter(HIE_tdstSuperObject_* ctx, tdstNodeInterpret_* stream,
                                              tdstGetSetParam_* out)
{
    HIE_tdstSuperObject_* spo;
    DNM_stDynamics* pstDynamic;
    MTH3D_tdstVector_ stVector;
    u16 op;

    op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(ctx, &spo);
    pstDynamic = ((tdstEngineObject_*) spo->hLinkedObject)->h_Dynam->p_stDynamics;

    switch (op) {
    case eFunc_GetMechanicGravityFactor:
        out->u.xValue = pstDynamic->m_tdstDynamicsBaseBlock.m_xGravity;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicSlide:
        out->u.xValue = pstDynamic->m_tdstDynamicsBaseBlock.m_xSlide;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicRebound:
        out->u.xValue = pstDynamic->m_tdstDynamicsBaseBlock.m_xRebound;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicSlopeLimit:
        out->u.xValue =
            tanf(pstDynamic->m_tdstDynamicsBaseBlock.m_xSlopeLimit * MTH_DEG_TO_RAD_inv);
        out->ucType = E_vt_Float;
        break;
    case eFunc_GetMechanicInertiaX:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaX;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicInertiaY:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaY;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicInertiaZ:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaZ;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicTiltIntensity:
        out->u.xValue = pstDynamic->m_tdstDynamicsComplexBlock.m_xTiltIntensity;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicTiltInertia:
        out->u.xValue = pstDynamic->m_tdstDynamicsComplexBlock.m_xTiltInertia;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicTiltOrigin:
        out->u.xValue = pstDynamic->m_tdstDynamicsComplexBlock.m_xTiltOrigin;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicMaxSpeed:
        stVector = pstDynamic->m_tdstDynamicsAdvancedBlock.m_stMaxSpeed;
        out->u.stVector = stVector;
        out->ucType = E_vt_Vector;
        return stream;
    case eFunc_GetMechanicStreamPriority:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamPrority;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetMechanicStreamSpeed:
        stVector = pstDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed;
        out->u.stVector = stVector;
        out->ucType = E_vt_Vector;
        return stream;
    case eFunc_GetMechanicStreamFactor:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamFactor;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetSlideFactorX:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorX;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetSlideFactorY:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorY;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetSlideFactorZ:
        out->u.xValue = pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorZ;
        out->ucType = E_vt_Float;
        return stream;
    case eFunc_GetSpeedAnim:
        out->u.stVector = pstDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim;
        out->ucType = E_vt_Vector;
        break;
    }
    return stream;
}

tdstNodeInterpret_* fn_p_GetHierarchyLink(HIE_tdstSuperObject_* context, tdstNodeInterpret_* stream,
                                  tdstGetSetParam_* out)
{
    HIE_tdstSuperObject_* perso;
    HIE_tdstSuperObject_* father;
    u16 op = AI_NODE_ID(stream);
    s32 sector;
    void* linked;

    fn_vGetUltraOperatorPerso(context, &perso);
    do {
        if ((u8) (u32) stream & 0x100)
            stream = 0;
        if (op == 0x109) {
            father = perso;
            sector = 2;
            father = father->hFatherSta;
            while (father != NULL && father->lTypeOfLinkedObject != sector)
                father = father->hFatherSta;
            if (father != NULL) {
                linked = 0;
                if (father->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv)
                    linked = father->hLinkedObject;
            } else {
                linked = 0;
            }
            out->u.pvValue = linked;
            out->ucType = E_vt_Perso;
        }
    } while (0);
    return stream;
}

tdstNodeInterpret_* fn_p_GetPriviligedActivationZdx(HIE_tdstSuperObject_* superObject,
                                                    tdstNodeInterpret_* stream, u8* p_stValue)
{
    HIE_tdstSuperObject_* hPerso;
    void* payload;
    s32 id;

    u16 type = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &hPerso);
    payload = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) hPerso)->hLinkedObject)->h_CollSet;
    stream = fn_p_stEvalTree(superObject, stream, p_stValue);
    id = *(s32*) p_stValue;
    id--;
    switch (type) {
    case eFunc_GetActivationZDD:
        *(u32*) p_stValue = CS_fn_eGetPrivilegedActivationZone(0, payload, (s16) id);
        p_stValue[12] = 6;
        break;
    case eFunc_GetActivationZDM:
        *(u32*) p_stValue = CS_fn_eGetPrivilegedActivationZone(1, payload, (s16) id);
        p_stValue[12] = 6;
        break;
    case eFunc_GetActivationZDE:
        *(u32*) p_stValue = CS_fn_eGetPrivilegedActivationZone(2, payload, (s16) id);
        p_stValue[12] = 6;
        break;
    case eFunc_GetActivationZDR:
        *(u32*) p_stValue = CS_fn_eGetPrivilegedActivationZone(3, payload, (s16) id);
        p_stValue[12] = 6;
        break;
    }
    return stream;
}

void* fn_p_GetComputationFrequency(HIE_tdstSuperObject_* superObject, u16* p_stTree,
                                   tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* obj;
    void* node;
    s32 type;
    u32 ulFrequency;

    type = p_stTree[-4];
    fn_vGetUltraOperatorPerso(superObject, &obj);
    switch (type) {
    case 0x10F:
        ulFrequency = 1;
        node = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) obj)->hLinkedObject)->h_3dData;
        if (node != 0) {
            ulFrequency = ((gu_tdst3dData_*) node)->ucBrainComputationFrequency;
        }
        p_stValue->u.ucValue = (u8) ulFrequency;
        p_stValue->ucType = E_vt_0To255;
        break;

    case 0x10E:
        ulFrequency = 1;
        node = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) obj)->hLinkedObject)->h_CollSet;
        if (node != 0) {
            ulFrequency = ((CollisionSet*) node)->ucCollComputeFrequency;
        }
        p_stValue->u.ucValue = (u8) ulFrequency;
        p_stValue->ucType = E_vt_0To255;
        break;

    case 0x110:
        p_stValue->u.ucValue = 0;
        p_stValue->ucType = E_vt_0To255;
        break;

    default:
        return p_stTree;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_ComputeJumpImpulsion(HIE_tdstSuperObject_* p_SuperObjPerso,
                                              tdstNodeInterpret_* p_stTree,
                                              tdstGetSetParam_* p_stValue)
{
    MTH3D_tdstVector_ vStart;
    MTH3D_tdstVector_ vDest;
    MTH3D_tdstVector_ vOut;
    MTH3D_tdstVector_ vZero;
    HIE_tdstSuperObject_* perso;
    f32 zero;
    f32 xHeight;
    f32 xApexZ;
    f32 gravity;
    f32 half_g;
    f32 vz;
    f32 delta;
    f32 xTime;
    void* node;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&vZero, zero, zero, zero);

    node = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    vStart = p_stValue->u.stVector;

    node = fn_p_stEvalTree(p_SuperObjPerso, node, p_stValue);
    vDest = p_stValue->u.stVector;

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, node, p_stValue);

    if (p_stValue->ucType == E_vt_Float) {
        xHeight = p_stValue->u.xValue;
    } else {
        xHeight = (f32) p_stValue->u.lValue;
    }

    if (xHeight < 0.0f) {
        p_stValue->u.stVector = vZero;
        p_stValue->ucType = E_vt_Vector;
        return p_stTree;
    }

    xApexZ = xHeight + vDest.xZ;
    if (xApexZ < vStart.xZ) {
        xApexZ = xHeight + vStart.xZ;
    }

    MTH3D_M_vZeroXYZ_inv(&vOut);
    MTH3D_M_vZeroXYZ_inv(&vZero);

    p_stValue->u.stVector = vOut;
    p_stValue->ucType = E_vt_Vector;

    gravity = ((tdstEngineObject_*) perso->hLinkedObject)->h_Dynam->p_stDynamics->m_tdstDynamicsBaseBlock.m_xGravity;
    if (gravity == 0.0f) {
        p_stValue->u.stVector = vZero;
        p_stValue->ucType = E_vt_Vector;
        return p_stTree;
    }

    half_g = gravity * -0.5f;
    vz = MTH_fn_xSqrtOpt2(((xApexZ - vStart.xZ) + (xApexZ - vStart.xZ)) * gravity);
    vOut.xZ = vz;
    delta = vz * vz - half_g * (vStart.xZ - vDest.xZ) * 4.0f;

    if (delta < 0.0f) {
        p_stValue->u.stVector = vZero;
        p_stValue->ucType = E_vt_Vector;
        return p_stTree;
    }

    delta = MTH_fn_xSqrtOpt2(delta);
    xTime = (inv_MTH_fn_xNegate(vz) - delta) / (half_g + half_g);
    vOut.xX = (vDest.xX - vStart.xX) / xTime;
    vOut.xY = (vDest.xY - vStart.xY) / xTime;
    p_stValue->u.stVector = vOut;
    p_stValue->ucType = E_vt_Vector;

    return p_stTree;
}

void* fn_p_GetBooleanInArray(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                             tdstGetSetParam_* out)
{
    HIE_tdstSuperObject_* owner;
    void* data;
    u8 handle;
    u16 code;
    u32 index;
    u32 startWord;
    u32 firstBit;
    u32 endWord;
    u32 endBit;
    u32 bit;
    s32 count;
    s32 word;

    code = AI_NODE_ID(stream);
    if (stream->eType == E_ti_Operator) {
        stream = fn_p_stOperatorGetPerso(superObject, stream + 1, &owner);
    } else {
        owner = superObject;
    }

    handle = (u8) stream->param;
    data = inv_AI_fn_p_stSuperObjectMind(owner);
    stream = fn_p_stEvalTree(superObject, stream + 1, out);
    index = (u32) out->u.lValue - 1;
    out->u.lValue = 0;
    out->ucType = E_vt_Integer;
    startWord = index >> 5;
    firstBit = index & 0x1F;

    switch (code) {
    case eFunc_GetBooleanInArray:
        fn_ucGetDsgVar(handle, startWord & 0xFF, data, out);
        out->u.lValue = (out->u.lValue >> firstBit) & 1;
        out->ucType = E_vt_Integer;
        break;
    case eFunc_GetNumberOfBooleanInArray:
        stream = fn_p_stEvalTree(superObject, stream, out);
        index = (u32) out->u.lValue - 1;
        endWord = index >> 5;
        endBit = index & 0x1F;
        count = 0;
        for (word = startWord; word <= endWord; word++) {
            s32 wordValue;
            u32 lowBit;
            u32 lastBit;

            fn_ucGetDsgVar(handle, word & 0xFF, data, out);
            wordValue = out->u.lValue;
            if (word == startWord)
                lowBit = firstBit;
            else
                lowBit = 0;
            if (word == endWord)
                lastBit = endBit;
            else
                lastBit = 0x1F;
            bit = lowBit;
            while (lastBit >= bit) {
                count += (wordValue >> bit) & 1;
                bit++;
            }
        }
        out->u.lValue = count;
        out->ucType = E_vt_Integer;
        break;
    default:
        break;
    }

    return stream;
}

s32 fn_p_stGetButtonName(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                         tdstGetSetParam_* outParam)
{
    s32 result;

    result = (s32) fn_p_stEvalTree(superObject, stream, outParam);
    outParam->u.pvValue = IPT_fn_szGetStringOf(outParam->u.pvValue);
    outParam->ucType = E_vt_String;
    return result;
}

tdstNodeInterpret_* fn_p_stGetDriversAvailable(HIE_tdstSuperObject_* superObject,
                                               tdstNodeInterpret_* stream,
                                               tdstGetSetParam_* outParam)
{
    s32 type;
    s32 count;

    type = 6;
    count = byte_2542A9[0];
    outParam->ucType = (s8) type;
    outParam->u.lValue = count;
    return stream;
}

tdstNodeInterpret_* fn_p_stGetCurrentLanguageId(HIE_tdstSuperObject_* superObject,
                                                tdstNodeInterpret_* stream,
                                                tdstGetSetParam_* outParam)
{
    s32 type;
    s32 languageId;

    type = 6;
    languageId = g_stEngineStructure.ucCurrentLanguage;
    outParam->ucType = (s8) type;
    outParam->u.lValue = languageId;
    return stream;
}

tdstNodeInterpret_* fn_p_stGetNbLanguages(HIE_tdstSuperObject_* superObject,
                                          tdstNodeInterpret_* stream, tdstGetSetParam_* outParam)
{
    s32 type;
    s32 count;

    type = 6;
    count = inv_TXT_textTableState.nbLanguages;
    outParam->ucType = (s8) type;
    outParam->u.lValue = count;
    return stream;
}

void* fn_p_stGetLanguageText(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                             tdstGetSetParam_* out)
{
    return fn_p_stEvalTree(superObject, stream, out);
}

s32 fn_p_stTextToInt(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                     tdstGetSetParam_* outParam)
{
    tdstGetSetParam_* p_stOutput;
    s32 result;
    void* value;

    p_stOutput = outParam;
    result = (s32) fn_p_stEvalTree(superObject, stream, p_stOutput);
    value = fn_szGetStringFromTextOrStringParam(p_stOutput);
    value = (void*) atoi(value);
    p_stOutput->u.lValue = (s32) value;
    value = (void*) 6;
    p_stOutput->ucType = (u8) (u32) value;
    return result;
}

void* fn_p_stOptionRecupererFunc(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                 tdstGetSetParam_* outParam)
{
    void* line;
    s32 value;
    u16 op = AI_NODE_ID(stream);

    switch (op) {
    case 0x11A:
        line = (void*) g_lSoundVolumeLineMusic;
        value = SND_fn_lGetVolumeLineSound(line);
        break;
    case 0x11B:
        line = (void*) g_lSoundVolumeLineSound;
        value = SND_fn_lGetVolumeLineSound(line);
        break;
    case 0x11C:
        line = (void*) g_lSoundVolumeLineVoices;
        value = SND_fn_lGetVolumeLineSound(line);
        break;
    default:
        return stream;
    }
    outParam->u.lValue = value;
    outParam->ucType = E_vt_Integer;
    return stream;
}

void* func_001EEFD0(void* context, void* stream)
{
    (void) context;
    return stream;
}

void* func_001EEFD8(void* context, void* stream)
{
    (void) context;
    return stream;
}

void* func_001EEFE0(void* context, void* stream)
{
    (void) context;
    return stream;
}

void* gu_fn_p_stOptionSlotIsValid(void* superObject, void* stream, void* out)
{
    return fn_p_stEvalTree(superObject, stream, out);
}

void* fn_p_stGetNameResolution(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                               tdstGetSetParam_* outParam)
{
    tdstGetSetParam_* p_stOutput = outParam;
    void* result = fn_p_stEvalTree(superObject, stream, p_stOutput);
    p_stOutput->u.lValue = (s32) inv_TXT_resolutionName;
    p_stOutput->ucType = E_vt_String;
    return result;
}

void* func_001EF040(void* context, void* stream)
{
    (void) context;
    return stream;
}

tdstNodeInterpret_* fn_p_stGetVideoOptions(HIE_tdstSuperObject_* superObject,
                                           tdstNodeInterpret_* stream, tdstGetSetParam_* outParam)
{
    s32 value;
    s32 type;
    value = 1;
    type = 6;
    outParam->u.lValue = value;
    outParam->ucType = (s8) type;
    return stream;
}

tdstNodeInterpret_* fn_p_stGetMSSoundValues(HIE_tdstSuperObject_* superObject,
                                            tdstNodeInterpret_* stream, tdstGetSetParam_* outParam)
{
    HIE_tdstSuperObject_* pkt;
    tdstEngineObject_* node;
    void* what;
    u16 tag;
    s32 result;

    result = 0;
    tag = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &pkt);
    node = pkt->hLinkedObject;
    what = node->h_MSSound;
    if (what != 0) {
        switch (tag) {
        case eFunc_GetSaturationDistance:
            result = fn_xMSSoundGetSaturationDistance(what) >> 16;
            break;
        case eFunc_GetBackgroundDistance:
            result = fn_xMSSoundGetBackGroundDistance(what) >> 16;
            break;
        default:
            break;
        }
    }
    outParam->u.lValue = result;
    outParam->ucType = E_vt_Integer;
    return stream;
}

void* fn_p_stGetStdGameLimit(HIE_tdstSuperObject_* superObject, u16* stream,
                             tdstGetSetParam_* outParam)
{
    HIE_tdstSuperObject_* handle;
    gu_tdstStandardGame* node;
    s32 type;
    u32 value;
    u32 code;

    value = 0;
    type = stream[-4];
    fn_vGetUltraOperatorPerso(superObject, &handle);
    node = inv_HIE_fn_p_stGetEngineObject(handle)->h_StandardGame;

    if (node != 0) {
        switch (type) {
        case 0x127:
            value = node->ucTooFarLimit;
            break;

        case 0x128:
            value = node->ucTransparencyZoneMin;
            break;

        case 0x129:
            value = node->ucTransparencyZoneMax;
            break;

        default:
            break;
        }
    }
    code = 6;
    outParam->u.lValue = value;
    outParam->ucType = (u8) code;
    return stream;
}

s32 fn_p_stGetCurrentFrame(void* context, char* stream, tdstGetSetParam_* outParam)
{
    HIE_tdstSuperObject_* saved;
    gu_tdst3dData_* p_st3dData;
    s32 value;
    s32 code;

    value = 0;
    code = (s32) AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(context, &saved);
    p_st3dData = ((tdstEngineObject_*) saved->hLinkedObject)->h_3dData;
    if (p_st3dData != 0) {
        if (code == 298) {
            value = p_st3dData->uwCurrentFrame;
        }
    }
    outParam->u.lValue = value;
    outParam->ucType = E_vt_Integer;
    return (s32) stream;
}

tdstNodeInterpret_* fn_p_stComputeXor(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                      tdstGetSetParam_* base)
{
    u16 kind;
    u32 lhs;
    u32 rhs;
    u32 result;

    result = 0;
    kind = AI_NODE_ID(stream);
    stream = fn_p_stEvalTree(superObject, stream, base);
    lhs = base->u.lValue;
    stream = fn_p_stEvalTree(superObject, stream, base);
    rhs = base->u.lValue;

    switch (kind) {
    case eFunc_Xor:
        result = lhs ^ rhs;
        break;
    case eFunc_And:
        result = lhs & rhs;
        break;
    case eFunc_Or:
        result = lhs | rhs;
        break;
    case eFunc_DivUnsigned:
        result = lhs / rhs;
        break;
    case eFunc_MulUnsigned:
        result = lhs * rhs;
        break;
    case eFunc_AddUnsigned:
        result = lhs + rhs;
        break;
    case eFunc_SubUnsigned:
        result = lhs - rhs;
        break;
    default:
        break;
    }
    base->u.lValue = result;
    base->ucType = E_vt_PositiveInteger;
    return stream;
}

void* fn_p_stComputeNot(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                        tdstGetSetParam_* outParam)
{
    tdstGetSetParam_* p_stOutput = outParam;
    void* result = fn_p_stEvalTree(superObject, stream, p_stOutput);
    u32 value = p_stOutput->u.lValue;
    p_stOutput->ucType = E_vt_PositiveInteger;
    p_stOutput->u.lValue = ~value;
    return result;
}

void* func_001EF360(void* context, void* stream, tdstGetSetParam_* out)
{
    switch (AI_NODE_ID(stream)) {
    case 0x57:
        inv_SAV_fn_vEnsureLevelDynamicListsLoaded();
    case 0x58:
        out->u.lValue = inv_fn_lGetLevelDynamicListCount();
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x59:
        stream = fn_p_stEvalTree(context, stream, out);
        out->u.pvValue =
            inv_SAV_fn_szGetSlotPlayerName(inv_SAV_fn_p_stGetSlotByIndex(*(s16*) &out->u));
        out->ucType = E_vt_String;
        break;
    case 0x5A:
        stream = fn_p_stEvalTree(context, stream, out);
        out->u.lValue =
            inv_SAV_fn_uwGetSlotIndex(inv_SAV_fn_p_stGetSlotByIndex(*(s16*) &out->u));
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x5B:
        stream = fn_p_stEvalTree(context, stream, out);
        out->u.pvValue = inv_SAV_fn_szGetSlotComment(inv_SAV_fn_p_stGetSlotByIndex(*(s16*) &out->u));
        out->ucType = E_vt_String;
        break;
    case 0x5D:
        out->u.pvValue = inv_SAV_fn_szGetCurrentPlayerName();
        out->ucType = E_vt_String;
        break;
    case 0x5E:
        out->u.lValue = inv_SAV_fn_wGetCurrentSlotIndex();
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x5F:
        out->u.pvValue = inv_SAV_fn_szGetCurrentSaveComment();
        out->ucType = E_vt_String;
        break;
    case 0x5C:
        inv_fn_vFreeLevelDynamicLists();
        out->u.lValue = 0;
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x61:
        out->u.lValue = inv_SAV_fn_bFormatCardIfNeeded();
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x63:
        out->u.lValue = inv_SAV_fn_bProcessCurrentSaveRequest();
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x62:
        stream = fn_p_stEvalTree(context, stream, out);
        out->u.lValue =
            inv_SAV_fn_bInitCurrentSaveForPlayer((void*) fn_szGetStringFromTextOrStringParam(out));
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x64:
        out->u.lValue = inv_SAV_fn_bCreateNewGameSave();
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x66:
        stream = fn_p_stEvalTree(context, stream, out);
        out->u.lValue = inv_SAV_fn_bSelectOrReplaceSlot(out->u.lValue);
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x65:
        stream = fn_p_stEvalTree(context, stream, out);
        out->u.lValue = inv_SAV_fn_bLoadGameFromSlot(out->u.lValue);
        out->ucType = E_vt_PositiveInteger;
        break;
    case 0x60:
        out->u.lValue = inv_SAV_fn_lCheckFreeSpace();
        out->ucType = E_vt_PositiveInteger;
        break;
    default:
        break;
    }
    return stream;
}

void* func_001EF560(void* context, void* stream, tdstGetSetParam_* outParam)
{
    HIE_tdstSuperObject_* saved;
    u16 op = AI_NODE_ID(stream);

    fn_vGetUltraOperatorPerso(context, &saved);
    switch (op) {
    case 0x133: {
        tdstEngineObject_* p_stEngineObject = saved->hLinkedObject;
        gu_tdst3dData_* p_st3dData = p_stEngineObject->h_3dData;
        outParam->u.xValue = inv_SHW_fn_xGetShadowScaleX(&p_st3dData->shadow);
        outParam->ucType = E_vt_Float;
        break;
    }
    case 0x134: {
        tdstEngineObject_* p_stEngineObject = saved->hLinkedObject;
        gu_tdst3dData_* p_st3dData = p_stEngineObject->h_3dData;
        outParam->u.xValue = inv_SHW_fn_xGetShadowScaleY(&p_st3dData->shadow);
        outParam->ucType = E_vt_Float;
        break;
    }
    default:
        return stream;
    }
    return stream;
}

void* fn_p_stDistanceToPersoCenter(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                    tdstGetSetParam_* p_stValue)
{
    MTH3D_tdstVector_ targetCenter;
    MTH3D_tdstVector_ operatorCenter;
    MTH3D_tdstVector_ delta;
    HIE_tdstSuperObject_* operatorPerso;
    HIE_tdstSuperObject_* target;
    MTH3D_tdstVector_* first;
    MTH3D_tdstVector_* second;
    void* operatorLocalCenter;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &operatorPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    target = ((tdstEngineObject_*) p_stValue->u.pvValue)->h_StandardGame->h_SuperObject;
    operatorLocalCenter = operatorPerso->hCollideBoundingVolume;
    if (target->hCollideBoundingVolume) {
        first = &targetCenter;
        POS_fn_vMulMatrixVertex(first, target->gu_hGlobalMatrix, target->hCollideBoundingVolume);
    } else {
        first = inv_POS_fn_p_stPosMatrixTranslation(target->gu_hGlobalMatrix);
    }
    if (operatorLocalCenter) {
        second = &operatorCenter;
        POS_fn_vMulMatrixVertex(second, operatorPerso->gu_hGlobalMatrix, operatorLocalCenter);
    } else {
        second = inv_POS_fn_p_stPosMatrixTranslation(operatorPerso->gu_hGlobalMatrix);
    }
    inv_MTH3D_fn_vSubXYZ(&delta, first, second);
    p_stValue->ucType = E_vt_Float;
    p_stValue->u.xValue = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ));
    return p_stTree;
}

void* fn_p_stDistanceToPerso(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                             tdstGetSetParam_* p_stValue)
{
    MTH3D_tdstVector_ delta;
    HIE_tdstSuperObject_* hPerso;
    POS_tdstMatrix_* p_stTargetMatrix;
    POS_tdstMatrix_* p_stMyMatrix;
    MTH3D_tdstVector_* position0;
    MTH3D_tdstVector_* position1;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    if (p_stValue->u.pvValue == NULL) {
        p_stValue->u.xValue = flt_393D9C[0];
        p_stValue->ucType = E_vt_Float;
        return p_stTree;
    }
    p_stMyMatrix = hPerso->gu_hGlobalMatrix;
    p_stTargetMatrix = inv_fn_hGetOwnerSuperObject(p_stValue->u.pvValue)->gu_hGlobalMatrix;
    position1 = inv_POS_fn_p_stPosMatrixTranslation(p_stMyMatrix);
    position0 = inv_POS_fn_p_stPosMatrixTranslation(p_stTargetMatrix);
    MTH3D_M_vSubXYZ_inv(&delta, position0, position1);
    p_stValue->ucType = E_vt_Float;
    p_stValue->u.xValue = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ));
    return p_stTree;
}

void* fn_p_stGetPersoAbsolutePosition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                      tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* saved;
    POS_tdstMatrix_* p_stGlobalMatrix;
    tdstGetSetParam_* dst;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    p_stGlobalMatrix = saved->gu_hGlobalMatrix;
    dst = p_stValue;
    dst->u.stVector = *inv_POS_fn_p_stPosMatrixTranslation(p_stGlobalMatrix);
    dst->ucType = E_vt_Vector;
    return p_stTree;
}

void* fn_p_stGetMyAbsolutePosition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                   tdstGetSetParam_* p_stValue)
{
    POS_tdstMatrix_* p_stGlobalMatrix;
    void* p_stNext;

    p_stGlobalMatrix = p_SuperObjPerso->gu_hGlobalMatrix;
    p_stNext = p_stTree;
    p_SuperObjPerso = (void*) 0x15;
    p_stValue->u.stVector = *inv_POS_fn_p_stPosMatrixTranslation(p_stGlobalMatrix);
    p_stValue->ucType = (u8) (u32) p_SuperObjPerso;
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stGetWpAbsolutePosition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                                 tdstGetSetParam_* p_stValue)
{
    WP_tdstWayPoint_* node;
    tdstNodeInterpret_* p_stNext;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    node = p_stValue->u.pvValue;
    if (node != 0) {
        if (node->m_hSuperObject != 0) {
            WP_fnv_WayPoint_ComputeLocation(node, &p_stValue->u.stVector);
            p_stValue->ucType = E_vt_Vector;
        } else {
            p_stValue->u.stVector = node->m_stVertex;
            p_stValue->ucType = E_vt_Vector;
        }
    } else {
        f32 zero;

        zero = 0.0f;
        p_stValue->ucType = E_vt_Vector;
        MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, zero, zero, zero);
    }
    return p_stNext;
}

f32 fn_xInternalDistance(f32* vec, s32 axis)
{
    f32 xFirst;
    f32 xSecond;
    f32 xSquareLength;

    switch (axis & 0xFF) {
    case 1:
        return vec[0];
    case 2:
        return vec[1];
    case 4:
        return vec[2];
    case 3:
        xSecond = vec[1];
    load_x:
        xFirst = vec[0];
    hypot2:
        xSecond = MTH_fn_xSqrOpt1(xSecond);
        xFirst = MTH_fn_xSqrOpt1(xFirst);
        xFirst = xFirst + xSecond;
        return MTH_fn_xSqrtOpt2(xFirst);
    case 5:
        xSecond = vec[2];
        goto load_x;
    case 6:
        xFirst = vec[1];
        xSecond = vec[2];
        goto hypot2;
    case 7:
        xSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(vec[0], vec[1], vec[2]);
        return MTH_fn_xSqrtOpt2(xSquareLength);
    default:
        return 0.0f;
    }
}

f32 fn_xExtendedDistanceBetweenPerso(HIE_tdstSuperObject_* param0,
                                     HIE_tdstSuperObject_* p_SuperObjPerso2, u8 ucFlags)
{
    MTH4D_tdstVector_ current;
    MTH4D_tdstVector_ other;
    u8 matrix[0x60];
    POS_tdstMatrix_* source;
    POS_tdstMatrix_* reference;
    s32 wanted;

    if (ucFlags & 0x80) {
        fn_vGetCenterOfBoundingVolume(param0, (MTH3D_tdstVector_*) &current);
        fn_vGetCenterOfBoundingVolume(p_SuperObjPerso2, (MTH3D_tdstVector_*) &other);
    } else {
        source = (param0)->gu_hGlobalMatrix;
        reference = (p_SuperObjPerso2)->gu_hGlobalMatrix;
        *(MTH3D_tdstVector_*) &current = *(MTH3D_tdstVector_*) &source->stMatrix.stCol_3;
        *(MTH3D_tdstVector_*) &other = *(MTH3D_tdstVector_*) &reference->stMatrix.stCol_3;
    }

    wanted = ucFlags & 7;
    MTH3D_M_vSubXYZ_inv(&current, &current, &other);

    if (wanted != 7) {
        POS_fn_vInvertIsoMatrix((POS_tdstMatrix_*) matrix, p_SuperObjPerso2->gu_hGlobalMatrix);
        POS_fn_vApplyMatrixToVector((MTH3D_tdstVector_*) &current, (POS_tdstMatrix_*) matrix, (MTH3D_tdstVector_*) &current);
    }
    return fn_xInternalDistance(&current, wanted);
}

tdstNodeInterpret_* fn_p_stDistanceFunction(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* local_perso;
    u16 op;
    f32 zero;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &local_perso);
    zero = inv_MTH_fn_xZero();

    switch (op) {
    case eFunc_GetPersoAbsolutePosition:
        *(MTH3D_tdstVector_*) p_stValue =
            *inv_POS_fn_p_stPosMatrixTranslation(local_perso->gu_hGlobalMatrix);
        p_stValue->ucType = E_vt_Vector;
        return p_stTree;

    case eFunc_GetAngleAroundZToPerso: {
        f32 angle1;
        f32 angle2;
        f32 mod1;
        f32 mod2;
        HIE_tdstSuperObject_* p_stTargetPerso;
        MTH3D_tdstVector_* target_pos;
        MTH3D_tdstVector_* my_pos;
        f32 two_pi;
        two_pi = MTH_TWO_PI_inv;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        p_stTargetPerso = inv_fn_hGetOwnerSuperObject(p_stValue->u.pvValue);

        MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, zero, -1.0f, zero);
        POS_fn_vApplyMatrixToVector(&p_stValue->u.stVector, local_perso->gu_hGlobalMatrix, &p_stValue->u.stVector);

        angle1 = fn_xComputeDirectionAngle(&p_stValue->u.stVector);
        mod1 = fn_xRealModuloAround(zero, two_pi, angle1);

        target_pos = inv_POS_fn_p_stPosMatrixTranslation(p_stTargetPerso->gu_hGlobalMatrix);
        my_pos = inv_POS_fn_p_stPosMatrixTranslation(local_perso->gu_hGlobalMatrix);

        MTH3D_M_vSubXYZ_inv(&p_stValue->u.stVector, target_pos, my_pos);

        angle2 = fn_xComputeDirectionAngle(&p_stValue->u.stVector);
        mod2 = fn_xRealModuloAround(zero, two_pi, angle2);

        p_stValue->u.xValue = fn_xComputeBestEcartDirectionAngle(mod1, mod2) * MTH_RAD_TO_DEG_inv;
        p_stValue->ucType = E_vt_Float;
        return p_stTree;
    }

    case eFunc_DistanceToWP:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        p_stValue->u.xValue = fn_xDistanceBetweenPersoAndWP(local_perso, *(void**) p_stValue);
        p_stValue->ucType = E_vt_Float;
        return p_stTree;

    default: {
        void* target;
        s32 lDistanceType;

        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        target = ((tdstEngineObject_*) p_stValue->u.pvValue)->h_StandardGame->h_SuperObject;

        switch (op) {
        case eFunc_DistanceXToPerso:
            lDistanceType = 1;
            break;
        case eFunc_DistanceYToPerso:
            lDistanceType = 2;
            break;
        case eFunc_DistanceZToPerso:
            lDistanceType = 4;
            break;
        case eFunc_DistanceXYToPerso:
            lDistanceType = 3;
            break;
        case eFunc_DistanceXZToPerso:
            lDistanceType = 5;
            break;
        case eFunc_DistanceYZToPerso:
            lDistanceType = 6;
            break;
        case eFunc_DistanceXToPersoCenter:
            lDistanceType = 0x81;
            break;
        case eFunc_DistanceYToPersoCenter:
            lDistanceType = 0x82;
            break;
        case eFunc_DistanceZToPersoCenter:
            lDistanceType = 0x84;
            break;
        case eFunc_DistanceXYToPersoCenter:
            lDistanceType = 0x83;
            break;
        case eFunc_DistanceXZToPersoCenter:
            lDistanceType = 0x85;
            break;
        case eFunc_DistanceYZToPersoCenter:
            lDistanceType = 0x86;
            break;
        default:
            lDistanceType = 0;
            break;
        }

        p_stValue->u.xValue = fn_xExtendedDistanceBetweenPerso(local_perso, target, lDistanceType);
        p_stValue->ucType = E_vt_Float;
        return p_stTree;
    }
    }
}

tdstNodeInterpret_* fn_p_stMathTernarReal(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
#define ASF(dst, p) ((dst) = ((p)->ucType == E_vt_Float) ? (p)->u.xValue : (f32) (p)->u.lValue)

    u16 op;
    f32 left;
    f32 right;
    f32 value;
    s32 condition;

    op = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    if (op == eFunc_TernOp) {
        condition = *(u8*) &p_stValue->u;
    } else {
        ASF(left, p_stValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        ASF(right, p_stValue);
        switch (op) {
        case eFunc_TernInf:
            condition = (left < right);
            break;
        case eFunc_TernSup:
            condition = (left > right);
            break;
        case eFunc_TernInfEq:
            condition = (left <= right);
            break;
        case eFunc_TernSupEq:
            condition = (left >= right);
            break;
        case eFunc_TernEq:
            condition = (left == right);
            break;
        default:
            condition = 0;
            break;
        }
    }

    if (condition) {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        ASF(value, p_stValue);
        p_stTree = fn_p_stSkipThisArgument(p_stTree);
    } else {
        p_stTree = fn_p_stSkipThisArgument(p_stTree);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        ASF(value, p_stValue);
    }

    p_stValue->u.xValue = value;
    p_stValue->ucType = E_vt_Float;
    return p_stTree;
}

static inline s32 inv_AI_fn_lRandomInRange(s32 minimum, s32 maximum)
{
    f32 range = (f32) (maximum - minimum) + 1.0f;

    return (s32) ((f32) rand() * (range * RAND_SCALE_inv)) + minimum;
}

void* fn_p_stMathFunctionInt(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                             tdstGetSetParam_* p_stValue)
{
    s32 value;
    s32 maximum;
    s32 op;

    op = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    if (p_stValue->ucType == E_vt_Float) {
        value = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
    } else {
        value = p_stValue->u.lValue;
    }
    switch (op) {
    case eFunc_Int:
        break;
    case eFunc_RandomInt:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float) {
            maximum = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
        } else {
            maximum = p_stValue->u.lValue;
        }
        value = inv_AI_fn_lRandomInRange(value, maximum);
        break;
    default:
        break;
    }
    p_stValue->u.lValue = value;
    p_stValue->ucType = E_vt_Integer;
    return p_stTree;
}

void* fn_p_stMathAbsoluteValue(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                               tdstGetSetParam_* p_stValue)
{
    void* ctx;
    f32 xValue;
    ctx = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    if (p_stValue->ucType == E_vt_Float) {
        xValue = *(f32*) &p_stValue->u.lValue;
    } else {
        xValue = (f32) p_stValue->u.lValue;
    }
    xValue = inv_MTH_fn_xAbsoluteTied(xValue);
    *(f32*) &p_stValue->u.lValue = xValue;
    p_stValue->ucType = E_vt_Float;
    return ctx;
}

tdstNodeInterpret_* fn_p_stMathFunctionReal(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    u16 op;
    f32 value;

    op = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    if (p_stValue->ucType == E_vt_Float) {
        value = p_stValue->u.xValue;
    } else {
        value = (f32) p_stValue->u.lValue;
    }

    switch (op) {
    case eFunc_Real:
        break;
    case eFunc_Sinus: {
        f32 x = value;
        x = sinf(x * MTH_DEG_TO_RAD_inv);
        value = x;
        break;
    }
    case eFunc_Cosinus: {
        f32 x = value;
        x = cosf(x * MTH_DEG_TO_RAD_inv);
        value = x;
        break;
    }
    case eFunc_Square: {
        f32 x = value;
        x = x * x;
        value = x;
        break;
    }
    case eFunc_SquareRoot: {
        f32 x = value;
        x = MTH_fn_xSqrtOpt2(x);
        value = x;
        break;
    }
    case eFunc_RandomReal: {
        f32 other;
        s32 lRandom;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float)
            other = p_stValue->u.xValue;
        else
            other = (f32) p_stValue->u.lValue;
        {
            f32 xRange = other;
            f32 delta;
            xRange = xRange - value;
            lRandom = rand();
            delta = (f32) lRandom * xRange * RAND_SCALE_inv;
            delta += value;
            value = delta;
        }
        break;
    }

    case eFunc_Sign: {
        f32 x = value;
        if (x < 0.0f)
            x = -1.0f;
        else if (0.0f < x)
            x = 1.0f;
        else
            x = 0.0f;
        value = x;
        break;
    }
    case eFunc_Cube: {
        f32 x = value;
        x = x * x * x;
        value = x;
        break;
    }
    case eFunc_Modulo: {
        f32 lower;
        f32 third;
        f32 range;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float)
            lower = p_stValue->u.xValue;
        else
            lower = (f32) p_stValue->u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float)
            third = p_stValue->u.xValue;
        else
            third = (f32) p_stValue->u.lValue;
        range = third - lower;
        while (third <= value)
            value -= range;
        while (value < lower)
            value += range;
        break;
    }

    case eFunc_MinimumReal: {
        f32 other;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float)
            other = p_stValue->u.xValue;
        else
            other = (f32) p_stValue->u.lValue;
        value = inv_MTH_fn_xMinimum(value, other);
        break;
    }
    case eFunc_MaximumReal: {
        f32 other;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float)
            other = p_stValue->u.xValue;
        else
            other = (f32) p_stValue->u.lValue;
        value = inv_MTH_fn_xMaximum(value, other);
        break;
    }
    case eFunc_DegreeToRadian: {
        f32 x = value;
        x *= MTH_DEG_TO_RAD_inv;
        value = x;
        break;
    }
    case eFunc_RadianToDegree: {
        f32 x = value;
        x *= MTH_RAD_TO_DEG_inv;
        value = x;
        break;
    }
    case eFunc_AbsoluteValue:
        break;
    case eFunc_LimitRealInRange: {
        f32 lower;
        f32 third;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float)
            lower = p_stValue->u.xValue;
        else
            lower = (f32) p_stValue->u.lValue;
        if (value < lower) {
            value = lower;
            p_stTree = fn_p_stSkipThisArgument(p_stTree);
        } else {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
            if (p_stValue->ucType == E_vt_Float)
                third = p_stValue->u.xValue;
            else
                third = (f32) p_stValue->u.lValue;
            if (third < value) {
                value = third;
                break;
            }
            if (third < lower)
                value = lower;
        }
        break;
    }
    default:
        break;
    }

    p_stValue->u.xValue = value;
    p_stValue->ucType = E_vt_Float;
    return p_stTree;
}

void* fn_p_stLocalToGlobal(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* saved;
    void* node;

    node = p_stTree;
    p_stTree = 0;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    node = fn_p_stEvalTree(p_SuperObjPerso, node, p_stValue);
    POS_fn_vRotateVector(&p_stValue->u.stVector, saved->gu_hGlobalMatrix, &p_stValue->u.stVector);
    return node;
}

tdstNodeInterpret_* fn_p_stDummyFunction(HIE_tdstSuperObject_* context, tdstNodeInterpret_* stream)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstNodeInterpret_* node;
    tdstNodeInterpret_* head;

    node = stream;
    head = node - 1;
    fn_vGetUltraOperatorPerso(context, &hNewSuperObjPerso);
    while (node->ucDepth > head->ucDepth) {
        node = fn_p_stSkipThisArgument(node);
    }
    printf("WARNING : fn_p_stDummyFunction\n");
    return node;
}

tdstNodeInterpret_* fn_p_stGlobalToLocal(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                         tdstGetSetParam_* p_stValue)
{
    u8 inverseMatrix[0x60];
    HIE_tdstSuperObject_* node;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    POS_fn_vInvertIsoMatrix((POS_tdstMatrix_*) inverseMatrix, node->gu_hGlobalMatrix);
    POS_fn_vApplyMatrixToVector(&p_stValue->u.stVector, (POS_tdstMatrix_*) inverseMatrix, &p_stValue->u.stVector);
    return p_stTree;
}

typedef struct VectorFunctionWorkspace {
    MTH3D_tdstVector_ v;
    u8 pad0C[4];
    f32 m[16];
    MTH3D_tdstVector_ tmp;
} VectorFunctionWorkspace;

static inline void inv_AI_fn_vMulMatrixVector(VectorFunctionWorkspace* fs, tdstGetSetParam_* out)
{
    if (&fs->v == &out->u.stVector) {
        fs->tmp = fs->v;
        {
            f32 d;
            d = inv_MTH_fn_xMulAdd3Grouped(fs->m[0], fs->m[4], fs->m[8], fs->tmp.xX, fs->tmp.xY,
                                           fs->tmp.xZ);
            out->u.stVector.xX = d;
        }
        {
            f32 d;
            d = inv_MTH_fn_xMulAdd3Grouped(fs->m[1], fs->m[5], fs->m[9], fs->tmp.xX, fs->tmp.xY,
                                           fs->tmp.xZ);
            out->u.stVector.xY = d;
        }
        {
            f32 d;
            d = inv_MTH_fn_xMulAdd3Grouped(fs->m[2], fs->m[6], fs->m[10], fs->tmp.xX, fs->tmp.xY,
                                           fs->tmp.xZ);
            out->u.stVector.xZ = d;
        }
    } else {
        {
            f32 d;
            d = inv_MTH_fn_xMulAdd3Grouped(fs->m[0], fs->m[4], fs->m[8], fs->v.xX, fs->v.xY,
                                           fs->v.xZ);
            out->u.stVector.xX = d;
        }
        {
            f32 d;
            d = inv_MTH_fn_xMulAdd3Grouped(fs->m[1], fs->m[5], fs->m[9], fs->v.xX, fs->v.xY,
                                           fs->v.xZ);
            out->u.stVector.xY = d;
        }
        {
            f32 d;
            d = inv_MTH_fn_xMulAdd3Grouped(fs->m[2], fs->m[6], fs->m[10], fs->v.xX, fs->v.xY,
                                           fs->v.xZ);
            out->u.stVector.xZ = d;
        }
    }
}

void* fn_p_stVectorFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
#ifndef NULL
#define NULL ((void*) 0)
#endif

    VectorFunctionWorkspace stWork;
    tdstGetSetParam_* saved_out;
    u16 op;

    op = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    saved_out = p_stValue;

    switch (op) {
    case eFunc_RotateVector: {
        f32 factor;
        f32 half;
        f32 xSin;
        f32 xCos;
        f32 product;
        f32* p_xMatrix;

        stWork.v = saved_out->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        factor = inv_AI_fn_xAiParamFloat(p_stValue);
        if (factor != 0.0f) {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
            product = factor * MTH_DEG_TO_RAD_inv;
            half = 0.5f;
            half = inv_MTH_fn_xNegateTied(half);
            factor = product * half;
            xSin = sinf(factor);
            p_xMatrix = stWork.m;
            inv_MTH3D_fn_vKTimesXYZ(&p_stValue->u.stVector, xSin, &p_stValue->u.stVector);
            xCos = cosf(factor);
            POS_fn_vQuatToMat((MTH4D_tdstMatrix_*) p_xMatrix, p_stValue->u.stVector.xX, p_stValue->u.stVector.xY, p_stValue->u.stVector.xZ, xCos);
            inv_AI_fn_vMulMatrixVector(&stWork, p_stValue);
            break;
        }
        p_stTree = fn_p_stSkipThisArgument(p_stTree);
        p_stValue->u.stVector = stWork.v;
        p_stValue->ucType = E_vt_Vector;
        break;
    }

    case eFunc_VectorCombination:
    case eFunc_TemporalVectorCombination: {
        f32 factor;
        f32 xRatio;
        f32 xPower;
        s32 lCurveType;

        stWork.v = saved_out->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        xRatio = inv_AI_fn_xAiParamFloat(p_stValue);
        if (op == eFunc_VectorCombination) {
            factor = xRatio;
        } else if (xRatio == 0.0f) {
            factor = xRatio;
        } else {
            lCurveType = fn_lGetTemporalCurveType();
#ifdef PC_PORT
            if (inv_GAM_fn_xGetEngineClockTicks() != (f32) lCurveType) {
                factor = pc_curve_powf(xRatio, inv_GAM_fn_xGetEngineClockTicks(), lCurveType);
            } else
#endif
            switch (lCurveType) {
            case 1:
                factor = xRatio;
                break;
            case 2:
                factor = MTH_fn_xSqrOpt1(xRatio);
                break;
            case 3:
                xPower = MTH_fn_xSqrOpt1(xRatio);
                factor = xPower * xRatio;
                break;
            case 4:
                xRatio = MTH_fn_xSqrOpt1(xRatio);
                factor = MTH_fn_xSqrOpt1(xRatio);
                break;
            case 5:
                factor = MTH_fn_xSqrOpt1(xRatio);
                xPower = MTH_fn_xSqrOpt1(factor);
                factor = xPower * xRatio;
                break;
            default:
                xPower = MTH_fn_xSqrOpt1(xRatio);
                factor = xPower * xRatio;
                factor = MTH_fn_xSqrOpt1(factor);
                break;
            }
        }
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        inv_MTH3D_fn_vSubXYZ(&stWork.v, &stWork.v, &p_stValue->u.stVector);
        inv_MTH3D_fn_vKTimesXYZ(&stWork.v, factor, &stWork.v);
        inv_MTH3D_fn_vAddXYZ(&p_stValue->u.stVector, &stWork.v, &p_stValue->u.stVector);
        return p_stTree;
    }

    case eFunc_VectorContribution: {
        f32 xSquareLength;
        f32 xLength;
        f32 xInvLength;
        f32 dot;

        stWork.v = saved_out->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (!MTH3D_M_bIsZeroXYZ_inv(&p_stValue->u.stVector)) {
            xSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stValue->u.stVector.xX, p_stValue->u.stVector.xY, p_stValue->u.stVector.xZ);
            xLength = MTH_fn_xSqrtOpt2(xSquareLength);
            if (xLength != 0.0f) {
                xInvLength = 1.0f / xLength;
                inv_MTH3D_fn_vKTimesXYZ(&p_stValue->u.stVector, xInvLength, &p_stValue->u.stVector);
            }
            dot = inv_MTH_fn_xMulAdd3Grouped(p_stValue->u.stVector.xX, p_stValue->u.stVector.xY,
                                   p_stValue->u.stVector.xZ, stWork.v.xX, stWork.v.xY, stWork.v.xZ);
            inv_MTH3D_fn_vKTimesXYZ(&p_stValue->u.stVector, dot, &p_stValue->u.stVector);
            break;
        }
        saved_out->u.stVector = stWork.v;
        break;
    }

    case eFunc_ScaledVector: {
        f32 scale;

        stWork.v = saved_out->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        scale = inv_AI_fn_xAiParamFloat(p_stValue);
        inv_MTH3D_fn_vKTimesXYZ(&stWork.v, scale, &stWork.v);
        p_stValue->u.stVector = stWork.v;
        p_stValue->ucType = E_vt_Vector;
        return p_stTree;
    }

    case eFunc_GetVectorNorm: {
        f32 x;
        f32 y;
        f32 z;
        f32 xSquareLength;
        f32 xLength;

        x = p_stValue->u.stVector.xX;
        y = p_stValue->u.stVector.xY;
        z = p_stValue->u.stVector.xZ;
        xSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
        p_stValue->ucType = E_vt_Float;
        xLength = MTH_fn_xSqrtOpt2(xSquareLength);
        p_stValue->u.xValue = xLength;
        return p_stTree;
    }
    }
    return p_stTree;
}

void* fn_p_stRealFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* outParam)
{
    void* nextStream;
    f32 lo;
    f32 x;
    f32 hi;
    f32 result;
    s32 curveType;

    nextStream = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, outParam);
    if (outParam->ucType == E_vt_Float) {
        lo = outParam->u.xValue;
    } else {
        lo = (f32) outParam->u.lValue;
    }
    nextStream = fn_p_stEvalTree(p_SuperObjPerso, nextStream, outParam);
    if (outParam->ucType == E_vt_Float) {
        x = outParam->u.xValue;
    } else {
        x = (f32) outParam->u.lValue;
    }
    nextStream = fn_p_stEvalTree(p_SuperObjPerso, nextStream, outParam);
    if (outParam->ucType == E_vt_Float) {
        hi = outParam->u.xValue;
    } else {
        hi = (f32) outParam->u.lValue;
    }

    curveType = fn_lGetTemporalCurveType();
#ifdef PC_PORT
    if (inv_GAM_fn_xGetEngineClockTicks() != (f32) curveType) {
        result = pc_curve_powf(x, inv_GAM_fn_xGetEngineClockTicks(), curveType);
    } else
#endif
    switch (curveType) {
    case 1:
        result = x;
        break;
    case 2:
        result = MTH_fn_xSqrOpt1(x);
        break;
    case 3:
        result = MTH_fn_xSqrOpt1(x);
        result = result * x;
        break;
    case 4:
        result = MTH_fn_xSqrOpt1(x);
        result = MTH_fn_xSqrOpt1(result);
        break;
    case 5:
        result = MTH_fn_xSqrOpt1(x);
        result = MTH_fn_xSqrOpt1(result);
        result = result * x;
        break;
    default:
        result = MTH_fn_xSqrOpt1(x);
        result = result * x;
        result = MTH_fn_xSqrOpt1(result);
        break;
    }
    lo = lo - hi;
    outParam->ucType = E_vt_Float;
    result = result * lo;
    result = result + hi;
    outParam->u.xValue = result;
    return nextStream;
}

tdstNodeInterpret_* func_001F0CB0(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                  tdstGetSetParam_* outParam)
{
    u32 ulPriority;
    tdstNodeInterpret_* result;
    s32 lType;
    void* list;
    void* entry;

    result = fn_p_stEvalTree(superObject, stream, outParam);
    list = outParam->u.pvValue;
    lType = 6;
    entry = ((tdstEngineObject_*) list)->h_StandardGame;
    ulPriority = ((gu_tdstStandardGame*) entry)->queueKey0C;
    outParam->ucType = (u8) lType;
    ulPriority &= 0x7F;
    outParam->u.lValue = (s32) ulPriority;
    return result;
}

void* fn_p_stHitPointsAndHitPointsMaxFunction(HIE_tdstSuperObject_* p_SuperObjPerso,
                                              tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    u16 op;
    void* perso;
    u8 amount;
    s32 value;

    value = 0;
    op = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    perso = (void*) p_stValue->u.lValue;
    switch (op) {
    case eFunc_GetHitPoints:
        value = fn_ucGetHitPoints(perso);
        break;
    case eFunc_AddAndGetHitPoints:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float) {
            s32 bits;
            bits = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
            amount = (u8) bits;
        } else {
            amount = *(u8*) &p_stValue->u.lValue;
        }
        fn_vAddHitPoints(perso, amount);
        value = fn_ucGetHitPoints(perso);
        break;
    case eFunc_SubAndGetHitPoints:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float) {
            s32 bits;
            bits = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
            amount = (u8) bits;
        } else {
            amount = *(u8*) &p_stValue->u.lValue;
        }
        value = fn_ucSubHitPoints(perso, amount);
        break;
    case eFunc_GetHitPointsMax:
        value = fn_ucGetHitPointsMax(perso);
        break;
    case eFunc_AddAndGetHitPointsMax:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float) {
            s32 bits;
            bits = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
            amount = (u8) bits;
        } else {
            amount = *(u8*) &p_stValue->u.lValue;
        }
        fn_vAddHitPointsMax(perso, amount);
        value = fn_ucGetHitPointsMax(perso);
        break;
    case eFunc_SubAndGetHitPointsMax:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (p_stValue->ucType == E_vt_Float) {
            s32 bits;
            bits = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
            amount = (u8) bits;
        } else {
            amount = *(u8*) &p_stValue->u.lValue;
        }
        value = fn_ucSubHitPointsMax(perso, amount);
        break;
    default:
        break;
    }
    p_stValue->u.lValue = value;
    p_stValue->ucType = E_vt_Integer;
    return p_stTree;
}

void* fn_p_stTimeFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    u16 ulSavedTime;
    s32 ulCurrentTime;
    s32 eFuncId;

    ulSavedTime = AI_NODE_ID(p_stTree);
    ulCurrentTime = inv_fn_ulGetEngineClock();
    if (ulSavedTime != eFunc_GetTime) {
        if (ulSavedTime == eFunc_GetElapsedTime) {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
            eFuncId = *(s32*) p_stValue;
            if ((u32) ulCurrentTime >= (u32) eFuncId)
                ulCurrentTime = ulCurrentTime - eFuncId;
            else
                ulCurrentTime = eFuncId - ulCurrentTime;
        }
    }
    *(s32*) p_stValue = ulCurrentTime;
    p_stValue->ucType = E_vt_PositiveInteger;
    return p_stTree;
}

void* fn_p_stListFunction(s32 superObject, u8* stream, tdstGetSetParam_* base)
{
    u16 kind;
    u32 lhs;
    u32 result;

    kind = AI_NODE_ID(stream);
    stream = fn_p_stEvalTree(superObject, stream, base);
    lhs = base->u.lValue;
    switch (kind) {
    case 0x33:
        stream = fn_p_stEvalTree(superObject, stream, base);
        if (fn_p_stGetPersoInList((void*) lhs, *(u8*) base) != 0) {
            if (fn_p_stGetPersoInList((void*) lhs, *(u8*) base)->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
                result = (u32) fn_p_stGetPersoInList((void*) lhs, *(u8*) base)->hLinkedObject;
            } else {
                result = 0;
            }
        } else {
            result = 0;
        }
        base->u.lValue = result;
        base->ucType = E_vt_Perso;
        return stream;
    case 0x32:
        base->u.lValue = *(u8*) lhs;
        base->ucType = E_vt_Integer;
        return stream;
    default:
        break;
    }
    return stream;
}

void* fn_p_stInputFunction(void* context, void* stream)
{
    (void) context;
    return stream;
}

void* fn_p_stMiscFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    void* stream;
    u16 op;

    stream = p_stTree;
    op = AI_NODE_ID(stream);

    switch (op) {
    case eFunc_GetSubMapId:
        p_stValue->u.lValue = ((u32) fn_lGetSubMapNumber()) >> 16;
        p_stValue->ucType = E_vt_Integer;
        return stream;

    case eFunc_GetGlobalCounter:
        p_stValue->u.lValue = g_stEngineStructure.ucLevelGlobalCounter;
        p_stValue->ucType = E_vt_Integer;
        return stream;

    case eFunc_CountGeneratedObjects: {
        void* hPerso;
        stream = fn_p_stEvalTree(p_SuperObjPerso, stream, p_stValue);
        hPerso = (void*) p_stValue->u.lValue;
        p_stValue->u.lValue =
            fn_ulGetNumberOfAlwaysBelongingToMe(((tdstEngineObject_*) hPerso)->h_StandardGame->h_SuperObject);
        p_stValue->ucType = E_vt_Integer;
        return stream;
    }

    case eFunc_CastIntegerToChannel: {
        void* resultStream;
        resultStream = fn_p_stEvalTree(p_SuperObjPerso, stream, p_stValue);
        p_stValue->ucType = E_vt_Module;
        return resultStream;
    }

    case eFunc_GenerateObject: {
        void* alwaysObj;
        HIE_tdstSuperObject_* result;
        void* value;
        POS_tdstMatrix_ mtx;
        u16 type;

        stream = fn_p_stEvalTree(p_SuperObjPerso, stream, p_stValue);
        alwaysObj = (void*) p_stValue->u.lValue;
        stream = fn_p_stEvalTree(p_SuperObjPerso, stream, p_stValue);
        result = 0;

        if (alwaysObj != 0 && fn_ucIsAnAlwaysObject(alwaysObj)) {
            type = ((tdstEngineObject_*) alwaysObj)->h_StandardGame->otObjectPersonalType;
            gu_MTH4D_M_vSetIdentityMatrix(&mtx);
            if (mtx.ucType == MAT_eTransform_Identity) {
                mtx.ucType = MAT_eTransform_Translation;
            }
            MTH4D_fn_vCopy3DTo4DVector(&mtx.stMatrix.stCol_3, &p_stValue->u.stVector);
            result = fn_p_stAllocateAlways(type, gp_stDynamicWorld, p_SuperObjPerso, 0, &mtx);
        }

        if (result != 0) {
            if (result->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
                value = result->hLinkedObject;
            } else {
                value = 0;
            }
        } else {
            value = 0;
        }
        p_stValue->u.lValue = (s32) value;
        p_stValue->ucType = E_vt_Perso;
        break;
    }
    }

    return stream;
}

void* fn_p_stGameMaterialFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* saved;
    tdstBrain_* hBrain;
    struct GMT_tdstGameMaterial_* hGameMaterial;
    void* object;
    s32 op = AI_NODE_ID(p_stTree);
    s32 value = 0xCDCDCDCD;
    GMT_tdstCollideMaterial_* index;
    s32 result;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    switch (op) {
    case eFunc_GetLastTraversedMaterialType:
    case eFunc_GetLastTraversedMaterial:
        hBrain = inv_HIE_fn_p_stGetEngineObject(saved)->h_Brain;
        if (hBrain != 0) {
            hGameMaterial = hBrain->hLastCollidedGoThroughMaterial;
            if (GMT_fn_b_ulIsValid(hGameMaterial) != 0) {
                if (op == eFunc_GetLastTraversedMaterialType) {
                    index = GMT_fn_hGetCollideMaterial(hGameMaterial);
                    result = 0;
                    if (index != (GMT_tdstCollideMaterial_*) -1)
                        result = GMT_fn_hGetCollideMaterialIdentifier(index);
                    p_stValue->u.lValue = result;
                    p_stValue->ucType = E_vt_Mask;
                }
            } else {
                p_stValue->u.lValue = 0;
            }
            p_stValue->ucType = E_vt_Mask;
        }
        break;
    case eFunc_GetCurrentCollidedGMT:
        object = fn_pstGetDNMReport(saved);
        if (object != 0)
            value = (s32) ((DNM_stReport_*) object)->m_stGround.hCollidedMaterial;
    default:
        p_stValue->u.lValue = value;
        p_stValue->ucType = E_vt_GameMaterial;
    }
    return p_stTree;
}

void* fn_p_stVisualGameMaterialFunction(HIE_tdstSuperObject_* superObject,
                                        tdstNodeInterpret_* stream, tdstGetSetParam_* p_stValue)
{
    GEO_tdstColor_ color;
    u32 ulMaterialValue;
    f32 xFirstCoef;
    f32 xSecondCoef;
    u16 sel = AI_NODE_ID(stream);
    GliMaterialData* hMaterial;

    stream = fn_p_stEvalTree(superObject, stream, p_stValue);
    hMaterial = p_stValue->u.pvValue;

    if (hMaterial != 0) {
        switch (sel) {
        case eFunc_GetVisualGMTColor:
            fn_vConvertColorToParam(&color, p_stValue);
            return stream;
        case eFunc_GetVisualGMTSpecularCoef:
            fn_vConvertColorToParam(&color, p_stValue);
            return stream;
        case eFunc_GetVisualGMTSpecularExponent:
            p_stValue->u.ulValue = ulMaterialValue;
            p_stValue->ucType = E_vt_Integer;
            return stream;
        case eFunc_GetVisualGMTDiffuseCoef:
            GLI_xGetMaterialColor(hMaterial, &color);
            fn_vConvertColorToParam(&color, p_stValue);
            return stream;
        case eFunc_GetVisualGMTAmbientCoef:
            GLI_xGetMaterialAmbientCoef(hMaterial, &color);
            fn_vConvertColorToParam(&color, p_stValue);
            return stream;
        case eFunc_GetVisualGMTTextureScrollingCoefU:
            GLI_xGetMaterialTextureScrollingCoef(hMaterial, 0, &ulMaterialValue, &xFirstCoef, &xSecondCoef);
            p_stValue->u.xValue = xFirstCoef;
            p_stValue->ucType = E_vt_Float;
            return stream;
        case eFunc_GetVisualGMTTextureScrollingCoefV:
            GLI_xGetMaterialTextureScrollingCoef(hMaterial, 0, &ulMaterialValue, &xFirstCoef, &xSecondCoef);
            p_stValue->u.xValue = xSecondCoef;
            p_stValue->ucType = E_vt_Float;
            return stream;
        case eFunc_GetVisualGMTFrame:
            p_stValue->u.ulValue = GLI_lGetCurrentAnimatedTextureNumber(hMaterial);
            p_stValue->ucType = E_vt_Integer;
            return stream;
        case eFunc_GetVisualGMTNumberOfFrames:
            p_stValue->u.ulValue = GLI_lGetMaterialNumberOfAnimatedTexture(hMaterial);
            p_stValue->ucType = E_vt_Integer;
            return stream;
        default:
            goto zero_result;
        }
    }
zero_result:
    p_stValue->u.ulValue = 0;
    p_stValue->ucType = E_vt_Integer;
    return stream;
}

void* inv_fn_p_stEvalAndClampColorComponent(void* superObject, void* stream, void* destination)
{
    s32* pOut = destination;

    tdstGetSetParam_* pScratch;
    void* result;
    s32 lValue;

    pScratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = pScratch + 1;
    result = fn_p_stEvalTree(superObject, stream, pScratch);
    if (pScratch->ucType == E_vt_Float) {
        lValue = inv_MTH_fn_lToIntegerOrdered(pScratch->u.xValue);
    } else {
        lValue = pScratch->u.lValue;
    }
    if (lValue >= 101) {
        lValue = 100;
    } else {
        lValue = (lValue > -1) ? lValue : 0;
    }
    *pOut = lValue;
    inv_AI_parameterPoolCursor = pScratch;
    return result;
}

void* inv_fn_p_stSetColorR(void* context, void* stream, u8* outParam)
{
    u8 component;
    void* result;

    result = inv_fn_p_stEvalAndClampColorComponent(context, stream, &component);
    *outParam = component;
    return result;
}

void* inv_fn_p_stSetColorG(void* context, void* stream, u8* outParam)
{
    u8 component;
    void* result;

    result = inv_fn_p_stEvalAndClampColorComponent(context, stream, &component);
    outParam[1] = component;
    return result;
}

void* inv_fn_p_stSetColorB(void* context, void* stream, u8* outParam)
{
    u8 component;
    void* result;

    result = inv_fn_p_stEvalAndClampColorComponent(context, stream, &component);
    outParam[2] = component;
    return result;
}

void* inv_fn_p_stSetColorA(void* context, void* stream, u8* outParam)
{
    u8 component;
    void* result;

    result = inv_fn_p_stEvalAndClampColorComponent(context, stream, &component);
    outParam[3] = component;
    return result;
}

void* inv_fn_p_stEvalRGB(void* context, void* stream, u8* outParam)
{
    void* step;

    outParam[3] = 0;
    step = inv_fn_p_stSetColorR(context, stream, outParam);
    step = inv_fn_p_stSetColorG(context, step, outParam);
    step = inv_fn_p_stSetColorB(context, step, outParam);
    outParam[0xC] = 0x28;
    return step;
}

void* inv_fn_p_stEvalRGBA(void* context, void* stream, void* outParam)
{
    void* p_stNext = inv_fn_p_stEvalRGB(context, stream, outParam);
    return inv_fn_p_stSetColorA(context, p_stNext, outParam);
}

u32 fn_lAddColor(u32 color1, u32 color2)
{
    s32 a, b, g, r;
    s32 lowBytes;
    s32 maskedByte2;
    s32 byte2;
    s32 packed;

    a = (s32) (color1 & 0xFF) + (s32) (color2 & 0xFF);
    b = (s32) ((color1 >> 8) & 0xFF) + (s32) ((color2 >> 16) & 0xFF);
    g = (s32) ((color1 >> 16) & 0xFF) + (s32) ((color2 >> 8) & 0xFF);
    r = (s32) (color1 >> 24) + (s32) (color2 >> 24);

    if (a < 0) {
        a = 0;
    } else if (a > 100) {
        a = 100;
    }
    if (b < 0) {
        b = 0;
    } else if (b > 100) {
        b = 100;
    }
    if (g < 0) {
        g = 0;
    } else if (g > 100) {
        g = 100;
    }
    if (r < 0) {
        r = 0;
    } else if (r > 100) {
        r = 100;
    }

    packed &= ~0xFF;
    packed |= (a & 0xFF);
    packed &= ~0xFF00;
    packed |= ((b & 0xFF) << 8);
    packed &= ~0xFF0000;
    lowBytes = packed & 0xFFFF;
    maskedByte2 = packed & 0xFF0000;
    byte2 = (g & 0xFF) << 16;
    packed = lowBytes | byte2 | maskedByte2;
    return (u32) (packed | (r << 24));
}

void* fn_p_stColorFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    void* ctx = p_SuperObjPerso;
    tdstGetSetParam_* p_stOutput = p_stValue;
    u16 opcode;
    s32 op;
    s32 c = 0;
    u32 base;
    s32 code;

    opcode = AI_NODE_ID(p_stTree);
    switch (opcode) {
    case eFunc_ColorRedGreenBlue:
        p_stOutput->u.ulValue = 0;
        op = (u32) inv_fn_p_stEvalRGB(ctx, p_stTree, (u8*) p_stOutput);
        goto return_op;
    case eFunc_ColorRedGreenBlueAlpha:
        p_stOutput->u.ulValue = 0;
        op = (u32) inv_fn_p_stEvalRGBA(ctx, p_stTree, p_stOutput);
        goto return_op;
    case eFunc_AddColor:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        base = p_stOutput->u.ulValue;
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        p_stOutput->u.ulValue = fn_lAddColor(base, p_stOutput->u.ulValue);
        p_stOutput->ucType = E_vt_Color;
        return p_stTree;
    case eFunc_AddRed:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        base = p_stOutput->u.ulValue;
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        if (p_stOutput->ucType == E_vt_Float) {
            op = inv_MTH_fn_lToInteger(p_stOutput->u.xValue);
        } else {
            op = p_stOutput->u.aucValue[0];
        }
        code = (u8) op;
        p_stOutput->u.ulValue = fn_lAddColor(base, code);
        p_stOutput->ucType = E_vt_Color;
        return p_stTree;
    case eFunc_AddGreen:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        base = p_stOutput->u.ulValue;
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        if (p_stOutput->ucType == E_vt_Float) {
            op = inv_MTH_fn_lToInteger(p_stOutput->u.xValue);
        } else {
            op = p_stOutput->u.aucValue[0];
        }
        code = (u8) op << 8;
        p_stOutput->u.ulValue = fn_lAddColor(base, code);
        p_stOutput->ucType = E_vt_Color;
        return p_stTree;
    case eFunc_AddBlue:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        base = p_stOutput->u.ulValue;
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        if (p_stOutput->ucType == E_vt_Float) {
            op = inv_MTH_fn_lToInteger(p_stOutput->u.xValue);
        } else {
            op = p_stOutput->u.aucValue[0];
        }
        code = (u8) op << 16;
        p_stOutput->u.ulValue = fn_lAddColor(base, code);
        p_stOutput->ucType = E_vt_Color;
        return p_stTree;
    case eFunc_AddAlpha: {
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        base = p_stOutput->u.ulValue;
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        if (p_stOutput->ucType == E_vt_Float) {
            op = inv_MTH_fn_lToInteger(p_stOutput->u.xValue);
        } else {
            op = p_stOutput->u.aucValue[0];
        }
        p_stOutput->u.ulValue = fn_lAddColor(base, op << 24);
        p_stOutput->ucType = E_vt_Color;
        return p_stTree;
    }
    case eFunc_ColorRed:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        c = p_stOutput->u.aucValue[0];
        break;
    case eFunc_ColorGreen:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        c = p_stOutput->u.aucValue[1];
        break;
    case eFunc_ColorBlue:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        c = p_stOutput->u.aucValue[2];
        break;
    case eFunc_ColorAlpha:
        p_stTree = fn_p_stEvalTree(ctx, p_stTree, p_stOutput);
        c = p_stOutput->u.aucValue[3];
        break;
    }

    if (c < 0) {
        c = 0;
    } else if (c > 100) {
        c = 100;
    }
    p_stOutput->u.lValue = c;
    p_stOutput->ucType = E_vt_Integer;
    return p_stTree;
return_op:
    return (void*) (u32) op;
}

tdstNodeInterpret_* fn_p_st_SPO_GetDrawFlag(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                            tdstGetSetParam_* p_stValue)
{
    typedef struct {
        s32 _hdr[11];
        s32 mask;
    } SxdTable;

    SxdTable* table;
    s32 lBit;
    s32 lReturn;
    tdstNodeInterpret_* p_stNext;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);

    lReturn = -1;
    table = (SxdTable*) p_stValue->u.lValue;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stValue);

    lBit = p_stValue->u.lValue;
    if (table != 0 && lBit >= 0 && lBit < 0x20) {
        lReturn = (table->mask >> lBit) & 1;
    }

    p_stValue->u.lValue = lReturn;
    p_stValue->ucType = E_vt_Integer;

    return p_stNext;
}
