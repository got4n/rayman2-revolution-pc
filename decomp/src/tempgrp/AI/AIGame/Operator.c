#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

typedef struct AffectTarget {
    u8 index;
    u8 subIndex;
    u8 component;
    u16 field;
    u8 kind;
    HIE_tdstSuperObject_* perso;
} AffectTarget;

extern HIE_tdstSuperObject_* g_hCurrentSuperObjPerso;

void* fn_p_stScalarOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
    tdstGetSetParam_* operand;
    u16 operation;
    u8 ucIsThereFloat;
    f32 xReal;
    s32 lval;

    operation = AI_NODE_ID(p_stTree);
    if (operation == eOperator_ScalarUnaryMinus) {
        p_stValue->u.lValue = 0;
        p_stValue->ucType = E_vt_Integer;
    } else {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (operation == eOperator_ScalarMulScalar || operation == eOperator_ScalarDivScalar) {
            if ((p_stValue->ucType == E_vt_Float && p_stValue->u.xValue == 0.0f) ||
                (p_stValue->ucType != E_vt_Float && (f32) p_stValue->u.lValue == 0.0f)) {
                return fn_p_stSkipThisArgument(p_stTree);
            }
        }
    }
    operand = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = operand + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, operand);
    ucIsThereFloat = fn_ucAreThereFloatIfYesConvertThem(p_stValue, operand);
    switch (operation) {
    case eOperator_ScalarPlusScalar:
        if (ucIsThereFloat) {
            xReal = operand->u.xValue;
            p_stValue->ucType = E_vt_Float;
            p_stValue->u.xValue = p_stValue->u.xValue + xReal;
        } else {
            lval = operand->u.lValue;
            p_stValue->ucType = E_vt_Integer;
            p_stValue->u.lValue = p_stValue->u.lValue + lval;
        }
        break;
    case eOperator_ScalarMinusScalar:
    case eOperator_ScalarUnaryMinus:
        if (ucIsThereFloat) {
            xReal = operand->u.xValue;
            p_stValue->ucType = E_vt_Float;
            p_stValue->u.xValue = p_stValue->u.xValue - xReal;
        } else {
            lval = operand->u.lValue;
            p_stValue->ucType = E_vt_Integer;
            p_stValue->u.lValue = p_stValue->u.lValue - lval;
        }
        break;
    case eOperator_ScalarMulScalar:
        if (ucIsThereFloat) {
            xReal = operand->u.xValue;
            p_stValue->ucType = E_vt_Float;
            p_stValue->u.xValue = p_stValue->u.xValue * xReal;
        } else {
            lval = operand->u.lValue;
            p_stValue->ucType = E_vt_Integer;
            p_stValue->u.lValue = p_stValue->u.lValue * lval;
        }
        break;
    case eOperator_ScalarDivScalar:
        if (ucIsThereFloat) {
            xReal = operand->u.xValue;
            p_stValue->ucType = E_vt_Float;
            p_stValue->u.xValue = p_stValue->u.xValue / xReal;
        } else {
            p_stValue->u.lValue = p_stValue->u.lValue / operand->u.lValue;
            p_stValue->ucType = E_vt_Integer;
        }
        break;
    default:
        p_stValue->u.lValue = 0;
        p_stValue->ucType = E_vt_Integer;
        break;
    }
    inv_AI_parameterPoolCursor = operand;
    return p_stTree;
}

void* fn_p_stVectorOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
    tdstGetSetParam_* vector;
    tdstGetSetParam_* second;
    tdstGetSetParam_* other;
    u32 id;
    f32 scale;
    u32 operation;

    vector = inv_AI_parameterPoolCursor;
    second = vector + 1;
    other = NULL;
    id = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = vector + 2;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, vector);
    scale = 1.0f;
    if (id != eOperator_VectorUnaryMinus) {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, second);
        operation = id - 0x10;
        if ((u16) operation < 2) {
            other = second;
        } else if (second->ucType == E_vt_Float) {
            scale = second->u.xValue;
        } else {
            scale = (f32) second->u.lValue;
        }
    } else {
        operation = 4;
    }

    p_stValue->ucType = E_vt_Vector;
    switch (operation) {
    case 0:
        inv_MTH3D_fn_vAddXYZ(&p_stValue->u.stVector, &vector->u.stVector, &other->u.stVector);
        break;
    case 1:
        inv_MTH3D_fn_vSubXYZ(&p_stValue->u.stVector, &vector->u.stVector, &other->u.stVector);
        break;
    case 2:
        inv_MTH3D_fn_vKTimesXYZ(&p_stValue->u.stVector, scale, &vector->u.stVector);
        break;
    case 3:
        MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, vector->u.stVector.xX / scale,
                            vector->u.stVector.xY / scale, vector->u.stVector.xZ / scale);
        break;
    case 4:
        MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, -vector->u.stVector.xX, -vector->u.stVector.xY,
                            -vector->u.stVector.xZ);
        break;
    default:
        p_stValue->u.stVector.xZ = p_stValue->u.stVector.xY = p_stValue->u.stVector.xX = 0.0f;
        break;
    }
    inv_AI_parameterPoolCursor = vector;
    return p_stTree;
}

void* fn_p_stOperatorGetPerso(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                              HIE_tdstSuperObject_** hSuperObjPersoFound)
{
    tdstGetSetParam_* param;
    tdstEngineObject_* object;
    HIE_tdstSuperObject_* owner;
    void* next;

    param = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = param + 1;
    next = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    object = param->u.pvValue;
    owner = object->h_StandardGame->h_SuperObject;
    inv_AI_parameterPoolCursor = param;
    *hSuperObjPersoFound = owner;
    return next;
}

void fn_vOperatorGetField(u16 field, HIE_tdstSuperObject_* perso, tdstGetSetParam_* param)
{
    fn_ucGetFieldValue(field, perso, param);
}

void inv_fn_vOperatorSetField(u16 field, HIE_tdstSuperObject_* perso, tdstGetSetParam_* param)
{
    fn_ucSetFieldValue(field, perso, param);
}

void fn_vOperatorGetDsgVar(u8 index, u8 subIndex, HIE_tdstSuperObject_* perso,
                           tdstGetSetParam_* param)
{
    fn_ucGetDsgVar(index, subIndex, inv_AI_fn_p_stGetMindSO(perso), param);
}

void fn_vOperatorSetDsgVar(u8 index, u8 subIndex, HIE_tdstSuperObject_* perso,
                           tdstGetSetParam_* param)
{
    fn_ucSetDsgVar(index, subIndex, inv_AI_fn_p_stGetMindSO(perso), param);
}

tdstNodeInterpret_* fn_p_stArrayOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso = p_SuperObjPerso;
    u8 index;

    if (p_stTree->eType == E_ti_Operator) {
        p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree + 1, &perso);
    }
    index = p_stTree->param;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree + 1, p_stValue);
    fn_vOperatorGetDsgVar(index, p_stValue->u.aucValue[0], perso, p_stValue);
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stDotOperator(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                        tdstNodeInterpret_* p_stTree,
                                                        tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* hSuperObjPerso;
    tdstNodeInterpret_* p_stNext;
    u8 kind;

    p_stNext = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree, &hSuperObjPerso);
    kind = p_stNext->eType;
    if (kind == E_ti_DsgVarRef) {
        u8 index = p_stNext->param;
        p_stNext++;
        fn_vOperatorGetDsgVar(index, 0, hSuperObjPerso, p_stValue);
    } else if (kind == E_ti_Field) {
        u16 field = p_stNext->param;
        p_stNext++;
        fn_vOperatorGetField(field, hSuperObjPerso, p_stValue);
    } else if (kind == E_ti_Operator) {
        if ((u16) p_stNext->param == eOperator_ModelCast) {
            p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stValue);
        }
    }
    return p_stNext;
}

void* fn_p_stVectorDotOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                    tdstGetSetParam_* p_stValue)
{
    u16 component;

    component = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    switch (component) {
    case eOperator_GetVectorX:
        p_stValue->ucType = E_vt_Float;
        break;
    case eOperator_GetVectorY:
        p_stValue->ucType = E_vt_Float;
        p_stValue->u.stVector.xX = p_stValue->u.stVector.xY;
        break;
    case eOperator_GetVectorZ:
        p_stValue->ucType = E_vt_Float;
        p_stValue->u.stVector.xX = p_stValue->u.stVector.xZ;
        break;
    default:
        break;
    }
    return p_stTree;
}

void fn_vGetFieldOrDsgVarForAffectOperator(HIE_tdstSuperObject_* perso, u8 kind, u8 index,
                                           u8 subIndex, u16 field, tdstGetSetParam_* param)
{
    if (kind == E_ti_DsgVarRef) {
        fn_vOperatorGetDsgVar(index, subIndex, perso, param);
    } else if (kind == E_ti_Field) {
        fn_vOperatorGetField(field, perso, param);
    }
}

void inv_fn_vSetFieldOrDsgVarForAffectOperator(HIE_tdstSuperObject_* perso, u8 kind, u8 index,
                                               u8 subIndex, u16 field, tdstGetSetParam_* param)
{
    if (kind == E_ti_DsgVarRef) {
        fn_vOperatorSetDsgVar(index, subIndex, perso, param);
    } else if (kind == E_ti_Field) {
        inv_fn_vOperatorSetField(field, perso, param);
    }
}

void fn_vSetVectorComponentForAffectOperator(u8 ucXYZflag, MTH3D_tdstVector_* p_stVector, f32 xReal)
{
    switch (ucXYZflag) {
    case 1:
        p_stVector->xX = xReal;
        break;
    case 2:
        p_stVector->xY = xReal;
        break;
    case 3:
        p_stVector->xZ = xReal;
        break;
    default:
        break;
    }
}

void fn_vBuildVectorForAffectOperator(u8 component, tdstGetSetParam_* param, f32 value)
{
    f32 zero = 0.0f;

    MTH3D_M_vSetXYZ_inv(&param->u.stVector, zero, zero, zero);
    fn_vSetVectorComponentForAffectOperator(component, &param->u.stVector, value);
    param->ucType = E_vt_Vector;
}

void* fn_p_stAffectOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
    tdstGetSetParam_* value;
    tdstGetSetParam_* current;
    MTH3D_tdstVector_* other;
    MTH3D_tdstVector_* result;
    MTH3D_tdstVector_* source;
    f32 x;
    u16 op;
    u8 count;
    u8 mode;
    u8 hasFloat;
    AffectTarget target;
    f32 floatResult;
    s32 integerResult;

    count = 0;
    value = inv_AI_parameterPoolCursor;
    current = value + 1;
    target.index = 0xFF;
    target.subIndex = 0;
    target.component = 0;
    op = AI_NODE_ID(p_stTree);
    target.field = 13;
    target.perso = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = value + 2;
    if (op == eOperator_AffectArray) {
        count = fn_ucGetTreeNbParam(p_stTree) - 1;
    }
    p_stTree = inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
        p_SuperObjPerso, p_stTree, p_stValue, &target.perso, &target.index, &target.subIndex, &target.component,
        &target.field, &target.kind);
    if (op == eOperator_Affect) {
        if (target.component != 0) {
            fn_vGetFieldOrDsgVarForAffectOperator(target.perso, target.kind, target.index,
                                                  target.subIndex, target.field, p_stValue);
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, value);
            fn_vSetVectorComponentForAffectOperator(target.component, &p_stValue->u.stVector,
                                                    inv_AI_fn_xAiParamFloat(value));
        } else {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        }
    } else if (op == eOperator_AffectArray) {
        while (count != 0) {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
            fn_vOperatorSetDsgVar(target.index, target.subIndex, target.perso, p_stValue);
            target.subIndex++;
            count--;
        }
        inv_AI_parameterPoolCursor = value;
        p_stValue->ucType = E_vt_MetaActionReturn;
        p_stValue->u.lValue = 0;
        return p_stTree;
    } else {
        fn_vGetFieldOrDsgVarForAffectOperator(target.perso, target.kind, target.index,
                                              target.subIndex, target.field, current);
        if (op == eOperator_PlusPlusAffect || op == eOperator_MinusMinusAffect) {
            if (target.component == 0) {
                value->u.lValue = 1;
                value->ucType = E_vt_Integer;
            } else {
                fn_vBuildVectorForAffectOperator(target.component, value, 1.0f);
            }
        } else {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, value);
            if (target.component != 0) {
                fn_vBuildVectorForAffectOperator(target.component, value, inv_AI_fn_xAiParamFloat(value));
            }
        }
        mode = current->ucType == E_vt_Vector;
        if (mode != 0) {
            mode = 2;
            if (value->ucType != E_vt_Vector) {
                if (op == eOperator_SetVectorX || op == eOperator_SetVectorY || op == eOperator_SetVectorZ) {
                    mode = 1;
                } else if (op == eOperator_MulAffect) {
                    mode = 1;
                } else if (op == eOperator_DivAffect) {
                    mode = 1;
                } else if (target.component != 0) {
                    mode = 1;
                } else {
                    f32 zero = 0.0f;
                    MTH3D_M_vSetXYZ_inv(&value->u.stVector, zero, zero, zero);
                }
            }
        } else if (value->ucType == E_vt_Vector) {
            value->u.lValue = 0;
            value->ucType = E_vt_Float;
        }
        if (mode != 0) {
            other = NULL;
            result = &p_stValue->u.stVector;
            p_stValue->ucType = E_vt_Vector;
            x = 0.0f;
            source = &current->u.stVector;
            if (mode == 1) {
                x = value->ucType == E_vt_Float ? value->u.xValue : (f32) value->u.lValue;
                *result = *source;
            } else {
                other = &value->u.stVector;
            }
            switch (op) {
            case eOperator_SetVectorX:
                p_stValue->u.stVector.xX = x;
                break;
            case eOperator_SetVectorY:
                p_stValue->u.stVector.xY = x;
                break;
            case eOperator_SetVectorZ:
                p_stValue->u.stVector.xZ = x;
                break;
            case eOperator_MulAffect:
                if (target.component == 0) {
                    p_stValue->u.stVector.xX = x * current->u.stVector.xX;
                    p_stValue->u.stVector.xY = x * current->u.stVector.xY;
                } else {
                    *result = *source;
                    switch (target.component) {
                    case 1:
                        x = other->xX;
                        p_stValue->u.stVector.xX = current->u.stVector.xX * x;
                        break;
                    case 2:
                        x = other->xY;
                        p_stValue->u.stVector.xY = current->u.stVector.xY * x;
                        break;
                    case 3:
                        x = other->xZ;
                        p_stValue->u.stVector.xZ = current->u.stVector.xZ * x;
                        break;
                    }
                    break;
                }
                p_stValue->u.stVector.xZ = current->u.stVector.xZ * x;
                break;
            case eOperator_DivAffect:
                if (target.component == 0) {
                    p_stValue->u.stVector.xX = current->u.stVector.xX / x;
                    p_stValue->u.stVector.xY = current->u.stVector.xY / x;
                } else {
                    *result = *source;
                    switch (target.component) {
                    case 1:
                        x = other->xX;
                        p_stValue->u.stVector.xX = current->u.stVector.xX / x;
                        break;
                    case 2:
                        x = other->xY;
                        p_stValue->u.stVector.xY = current->u.stVector.xY / x;
                        break;
                    case 3:
                        x = other->xZ;
                        p_stValue->u.stVector.xZ = current->u.stVector.xZ / x;
                        break;
                    }
                    break;
                }
                p_stValue->u.stVector.xZ = current->u.stVector.xZ / x;
                break;
            case eOperator_PlusAffect:
            case eOperator_PlusPlusAffect:
                inv_MTH3D_fn_vAddXYZ(&p_stValue->u.stVector, &current->u.stVector, other);
                break;
            case eOperator_MinusAffect:
            case eOperator_MinusMinusAffect:
                inv_MTH3D_fn_vSubXYZ(&p_stValue->u.stVector, &current->u.stVector, other);
                break;
            default: {
                f32 zero = 0.0f;
                MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, zero, zero, zero);
                break;
            }
            }
        } else {
            hasFloat = fn_ucAreThereFloatIfYesConvertThem(value, current);
            p_stValue->u.lValue = 0;
            p_stValue->ucType = E_vt_Integer;
            switch (op) {
            case eOperator_PlusAffect:
            case eOperator_PlusPlusAffect:
                if (hasFloat != 0) {
                    floatResult = current->u.xValue + value->u.xValue;
                    p_stValue->ucType = E_vt_Float;
                    p_stValue->u.xValue = floatResult;
                } else {
                    integerResult = current->u.lValue + value->u.lValue;
                    p_stValue->ucType = E_vt_Integer;
                    p_stValue->u.lValue = integerResult;
                }
                break;
            case eOperator_MinusAffect:
            case eOperator_MinusMinusAffect:
                if (hasFloat != 0) {
                    floatResult = current->u.xValue - value->u.xValue;
                    p_stValue->ucType = E_vt_Float;
                    p_stValue->u.xValue = floatResult;
                } else {
                    integerResult = current->u.lValue - value->u.lValue;
                    p_stValue->ucType = E_vt_Integer;
                    p_stValue->u.lValue = integerResult;
                }
                break;
            case eOperator_MulAffect:
                if (hasFloat != 0) {
                    floatResult = current->u.xValue * value->u.xValue;
                    p_stValue->ucType = E_vt_Float;
                    p_stValue->u.xValue = floatResult;
                } else {
                    p_stValue->u.lValue = current->u.lValue * value->u.lValue;
                    p_stValue->ucType = E_vt_Integer;
                }
                break;
            case eOperator_DivAffect:
                if (hasFloat != 0) {
                    floatResult = current->u.xValue / value->u.xValue;
                    p_stValue->ucType = E_vt_Float;
                    p_stValue->u.xValue = floatResult;
                } else {
                    integerResult = current->u.lValue / value->u.lValue;
                    p_stValue->ucType = E_vt_Integer;
                    p_stValue->u.lValue = integerResult;
                }
                break;
            }
        }
    }

    inv_fn_vSetFieldOrDsgVarForAffectOperator(target.perso, target.kind, target.index,
                                              target.subIndex, target.field, p_stValue);
    inv_AI_parameterPoolCursor = value;
    p_stValue->ucType = E_vt_MetaActionReturn;
    p_stValue->u.lValue = 0;
    return p_stTree;
}

void fn_vSetUltraOperatorPerso(HIE_tdstSuperObject_* hCurrentSuperObjPerso, HIE_tdstSuperObject_* hNewCurrentSuperObjPerso)
{
    if (hCurrentSuperObjPerso == hNewCurrentSuperObjPerso) {
        g_hCurrentSuperObjPerso = NULL;
    } else {
        g_hCurrentSuperObjPerso = hNewCurrentSuperObjPerso;
    }
}

void fn_vGetUltraOperatorPerso(HIE_tdstSuperObject_* hCurrentSuperObjPerso, HIE_tdstSuperObject_** hNewSuperObjPerso)
{
    HIE_tdstSuperObject_* ultra = g_hCurrentSuperObjPerso;

    if (ultra != NULL) {
        *hNewSuperObjPerso = ultra;
        g_hCurrentSuperObjPerso = NULL;
    } else {
        *hNewSuperObjPerso = hCurrentSuperObjPerso;
    }
}

void* fn_p_stUltraOperator(HIE_tdstSuperObject_* hCurrentSuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    void* next = fn_p_stOperatorGetPerso(hCurrentSuperObjPerso, p_stTree, &perso);

    fn_vSetUltraOperatorPerso(hCurrentSuperObjPerso, perso);
    return fn_p_stEvalTree(hCurrentSuperObjPerso, next, p_stValue);
}

void* fn_p_stModelCastOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                               tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    void* next;
    void* object;

    next = fn_p_stOperatorGetPerso(p_SuperObjPerso, fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue), &perso);
    if (perso != NULL) {
        if (perso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            object = perso->hLinkedObject;
        } else {
            object = NULL;
        }
    } else {
        object = NULL;
    }
    p_stValue->u.pvValue = object;
    p_stValue->ucType = E_vt_Perso;
    return next;
}

tdstNodeInterpret_* inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
    HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream, tdstGetSetParam_* param,
    HIE_tdstSuperObject_** perso, u8* index, u8* subIndex, u8* component, u16* field, u8* kind)
{
    s32 bits;

    *kind = stream->eType;
    *perso = superObject;
    *field = 13;
    *index = 0xFF;
    *subIndex = 0;
    *component = 0;

    if (*kind == E_ti_Operator) {
        if ((u16) stream->param == eOperator_GetVectorX) {
            *component = 1;
        } else if ((u16) stream->param == eOperator_GetVectorY) {
            *component = 2;
        } else if ((u16) stream->param == eOperator_GetVectorZ) {
            *component = 3;
        }
        if (*component != 0) {
            stream++;
            *kind = stream->eType;
        }
        if (*kind == E_ti_Operator) {
            if ((u16) stream->param == eOperator_Dot) {
                stream = fn_p_stOperatorGetPerso(superObject, stream + 1, perso);
                *kind = stream->eType;
            } else {
                stream++;
                *kind = stream->eType;
                if (*kind == E_ti_Operator && (u16) stream->param == eOperator_Dot) {
                    stream = fn_p_stOperatorGetPerso(superObject, stream + 1, perso);
                }
                *index = stream->param;
                stream++;
                *kind = E_ti_Operator;
            }
        }
    }

    if (*kind == E_ti_DsgVarRef) {
        *index = stream->param;
        stream++;
    } else if (*kind == E_ti_Operator) {
        stream = fn_p_stEvalTree(superObject, stream, param);
        if (param->ucType == E_vt_Float) {
            bits = inv_MTH_fn_lToIntegerOrdered(param->u.xValue);
        } else {
            bits = param->u.aucValue[0];
        }
        *subIndex = bits;
        *kind = E_ti_DsgVarRef;
    } else if (*kind == E_ti_Field) {
        *field = stream->param;
        stream++;
    }

    if (*kind == E_ti_DsgVarRef) {
        fn_ucGetDsgVar(*index, *subIndex, inv_AI_fn_p_stGetMindSO(*perso), param);
    } else if (*kind == E_ti_Field) {
        fn_ucGetFieldValue(*field, *perso, param);
    }
    return stream;
}

static inline u8 inv_AI_fn_ucParamByte(tdstGetSetParam_* param)
{
    u8 amount = param->u.aucValue[0];

    if (param->ucType == E_vt_Float) {
        s32 converted;
        converted = inv_MTH_fn_lToIntegerOrdered(param->u.xValue);
        amount = converted;
    }
    return amount;
}

void* inv_fn_p_stSetHitPoints(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                              tdstEngineObject_* object)
{
    tdstGetSetParam_ argument;
    void* next;

    next = fn_p_stEvalTree(superObject, stream, &argument);
    fn_vSetHitPoints(object, inv_AI_fn_ucParamByte(&argument));
    return next;
}

void* inv_fn_p_stAddHitPoints(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                              tdstEngineObject_* object)
{
    tdstGetSetParam_ argument;
    void* next;

    next = fn_p_stEvalTree(superObject, stream, &argument);
    fn_vAddHitPoints(object, inv_AI_fn_ucParamByte(&argument));
    return next;
}

void* inv_fn_p_stSubHitPoints(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                              tdstEngineObject_* object)
{
    tdstGetSetParam_ argument;
    void* next;

    next = fn_p_stEvalTree(superObject, stream, &argument);
    fn_ucSubHitPoints(object, inv_AI_fn_ucParamByte(&argument));
    return next;
}

void* inv_fn_p_stSetHitPointsMax(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                 tdstEngineObject_* object)
{
    tdstGetSetParam_ argument;
    void* next;

    next = fn_p_stEvalTree(superObject, stream, &argument);
    fn_vSetHitPointsMax(object, inv_AI_fn_ucParamByte(&argument));
    return next;
}

void* inv_fn_p_stAddHitPointsMax(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                 tdstEngineObject_* object)
{
    tdstGetSetParam_ argument;
    void* next;

    next = fn_p_stEvalTree(superObject, stream, &argument);
    fn_vAddHitPointsMax(object, inv_AI_fn_ucParamByte(&argument));
    return next;
}

void* inv_fn_p_stSubHitPointsMax(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                 tdstEngineObject_* object)
{
    tdstGetSetParam_ argument;
    void* next;

    next = fn_p_stEvalTree(superObject, stream, &argument);
    fn_ucSubHitPointsMax(object, inv_AI_fn_ucParamByte(&argument));
    return next;
}
