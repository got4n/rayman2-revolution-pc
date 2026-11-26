#include "common.h"

#include "AI/AI.h"
#include "HIE/HIE.h"

#include "MTH/MTH.h"
#include "ps2/libc.h"

typedef void (*FieldConvertFn)(tdstGetSetParam_* param);

typedef struct FieldValueType {
    u32 code;
    FieldConvertFn convertGet;
    FieldConvertFn convertSet;
} FieldValueType;

extern FieldValueType a_stFieldTypeTable[];

void fn_vSetFieldTypeComport(tdstGetSetParam_* p_stGetSetParam)
{
}

void fn_vSetFieldTypeVector(tdstGetSetParam_* p_stGetSetParam)
{
}

void fn_vSetFieldTypeFloat(tdstGetSetParam_* p_stGetSetParam)
{
    if (p_stGetSetParam->ucType != E_vt_Float) {
        f32 value = p_stGetSetParam->u.lValue;

        p_stGetSetParam->ucType = E_vt_Float;
        p_stGetSetParam->u.xValue = value;
    }
}

void fn_vSetFieldTypeInteger(tdstGetSetParam_* p_stGetSetParam)
{
#ifdef PC_PORT
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
#else
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
#endif
}

void fn_vSetFieldType0To255(tdstGetSetParam_* p_stGetSetParam)
{
#ifdef PC_PORT
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
#else
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
#endif
}

void fn_ucSetFieldValue(u16 eFieldId, HIE_tdstSuperObject_* hSuperObjPerso, tdstGetSetParam_* p_stGetSetParam)
{
    u8 valueType = a_stFieldTable[eFieldId].valueType;
    AIFieldAccessFn set = fn_p_fn_pGetFieldSetFunctionPtr(eFieldId);

    a_stFieldTypeTable[valueType].convertSet(p_stGetSetParam);
    set(hSuperObjPerso, p_stGetSetParam);
}

void fn_vGetFieldTypeComport(tdstGetSetParam_* p_stGetSetParam)
{
}

void fn_vGetFieldTypeVector(tdstGetSetParam_* p_stGetSetParam)
{
}

void fn_vGetFieldTypeFloat(tdstGetSetParam_* p_stGetSetParam)
{
    if (p_stGetSetParam->ucType != E_vt_Float) {
        f32 value = p_stGetSetParam->u.lValue;

        p_stGetSetParam->ucType = E_vt_Float;
        p_stGetSetParam->u.xValue = value;
    }
}

void fn_vGetFieldTypeInteger(tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;

    if (p_stGetSetParam->ucType == E_vt_Float) {
        value = inv_MTH_fn_lToIntegerOrdered(p_stGetSetParam->u.xValue);
        p_stGetSetParam->u.lValue = value;
        p_stGetSetParam->ucType = E_vt_Integer;
    }
}

void fn_vGetFieldType0To255(tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;

    if (p_stGetSetParam->ucType == E_vt_Float) {
        value = inv_MTH_fn_lToIntegerOrdered(p_stGetSetParam->u.xValue);
        p_stGetSetParam->u.lValue = value;
        p_stGetSetParam->ucType = E_vt_Integer;
    }
}

void fn_ucGetFieldValue(u16 eFieldId, HIE_tdstSuperObject_* hSuperObjPerso, tdstGetSetParam_* p_stGetSetParam)
{
    u8 valueType = a_stFieldTable[eFieldId].valueType;
    AIFieldAccessFn get = fn_p_fn_pGetFieldGetFunctionPtr(eFieldId);
    FieldConvertFn convert = a_stFieldTypeTable[valueType].convertGet;

    memset(&p_stGetSetParam->u, 0, sizeof(p_stGetSetParam->u));
    p_stGetSetParam->ucType = E_vt_None;
    get(hSuperObjPerso, p_stGetSetParam);
    convert(p_stGetSetParam);
}
