#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"

#define DSG_RESULT_TYPE(type, source) ((type) + (s32) (source) - (s32) (source))

typedef struct tdstArray_ {
    void* unk00;
    u8 eType;
    u8 ucMaxSize;
    u8 pad06[2];
    u8 d_ArrayElement[1];
} tdstArray_;

s32 fn_ulSizeOfDsgVar(u8 eDsgVarTypeId, u8 ucNbElt)
{
    switch (eDsgVarTypeId) {
    case eDsgVarType_Integer:
    case eDsgVarType_PositiveInteger:
    case eDsgVarType_Float:
    case eDsgVarType_WayPoint:
    case eDsgVarType_Perso:
    case eDsgVarType_Comport:
    case eDsgVarType_Action:
    case eDsgVarType_Text:
    case eDsgVarType_GameMaterial:
    case eDsgVarType_VisualMaterial:
    case eDsgVarType_SoundEvent:
    case eDsgVarType_Caps:
    case eDsgVarType_Graph:
    case eDsgVarType_SOLinks:
    case inv_eDsgVarType_Module:
    case eDsgVarType_SuperObject:
    case eDsgVarType_Input:
        return 4;
    case eDsgVarType_Boolean:
    case eDsgVarType__128To127:
    case eDsgVarType_0To255:
        return 1;
    case eDsgVarType__32768To32767:
    case eDsgVarType_0To65535:
        return 2;
    case eDsgVarType_List:
        return (ucNbElt << 2) + 4;
    case eDsgVarType_Vector:
        return 12;
    case eDsgVarType_VectorArray:
        return ucNbElt * 12 + 8;
    case eDsgVarType_PersoArray:
    case eDsgVarType_FloatArray:
    case eDsgVarType_IntegerArray:
    case eDsgVarType_WayPointArray:
    case eDsgVarType_TextArray:
    case eDsgVarType_SoundEventArray:
    case inv_eDsgVarType_Array1D:
        return (ucNbElt << 2) + 8;
    default:
        return 0;
    }
}

s32 fn_ucGetDsgVarFloat(f32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    f32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Float;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.xValue = value;
    return type;
}

#ifdef PC_PORT
s32 fn_ucGetDsgVarPerso(HIE_tdstSuperObject_** pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
#else
void fn_ucGetDsgVarPerso(HIE_tdstSuperObject_** pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
#endif
{
    HIE_tdstSuperObject_* superObject = *pMyVarAddr;
    void* object;

    if (superObject != NULL) {
        if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            object = superObject->hLinkedObject;
        } else {
            object = NULL;
        }
    } else {
        object = NULL;
    }
    p_stGetSetParam->u.pvValue = object;
    p_stGetSetParam->ucType = E_vt_Perso;
#ifdef PC_PORT
    return 0xB;
#endif
}

s32 fn_ucGetDsgVarSuperObject(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;
    value = *pMyVarAddr;
    type = E_vt_SuperObject;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

#ifdef PC_PORT
s32 fn_ucGetDsgVarVector(MTH3D_tdstVector_* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
#else
void fn_ucGetDsgVarVector(MTH3D_tdstVector_* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
#endif
{
    p_stGetSetParam->u.stVector = *pMyVarAddr;
    p_stGetSetParam->ucType = E_vt_Vector;
#ifdef PC_PORT
    return 0x15;
#endif
}

s32 fn_ucGetDsgVarCaps(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Caps;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarGraph(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Graph;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarWayPoint(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_WayPoint;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarAction(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Action;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarComport(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Comport;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarText(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Text;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarList(s32 pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 type;

    type = E_vt_List;
    p_stGetSetParam->u.lValue = pMyVarAddr;
    p_stGetSetParam->ucType = type;
    return type;
}

s32 fn_ucGetDsgVarBoolean(s8* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVar_128To127(s8* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVar0To255(u8* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVar_32768To32767(s16* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVar0To65535(u16* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarInteger(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarPositiveInteger(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Integer;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarGameMaterial(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_GameMaterial;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarVisualMaterial(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_VisualMaterial;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarSoundEvent(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_SoundEvent;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarSOLinks(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_SOLinks;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

s32 func_001DE8C8(s32* source, void* unused, tdstGetSetParam_* out)
{
    s32 value;
    s32 type;

    value = *source;
    type = E_vt_Module;
    out->ucType = DSG_RESULT_TYPE(type, source);
    out->u.lValue = value;
    return type;
}

s32 fn_ucGetDsgVarInput(s32* pMyVarAddr, void* ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;
    s32 type;

    value = *pMyVarAddr;
    type = E_vt_Button;
    p_stGetSetParam->ucType = DSG_RESULT_TYPE(type, pMyVarAddr);
    p_stGetSetParam->u.lValue = value;
    return type;
}

#ifdef PC_PORT
s32 fn_ucGetDsgVarArray(tdstArray_* pMyVarAddr, u8 ucIndex, tdstGetSetParam_* p_stGetSetParam)
#else
void fn_ucGetDsgVarArray(tdstArray_* pMyVarAddr, u8 ucIndex, tdstGetSetParam_* p_stGetSetParam)
#endif
{
    u8* elements = pMyVarAddr->d_ArrayElement;
    s32 type = pMyVarAddr->eType;
#ifdef PC_PORT
    DsgVarGetFn get;
#else
    void (*get)(void*, u8, void*);
#endif
    void* element;

    get = a_stDsgVarTypeTable[type].get;
    if (type == eDsgVarType_Vector) {
        element = elements + ucIndex * 12;
    } else {
        element = elements + ucIndex * 4;
    }
#ifdef PC_PORT
    return get(element, 0, p_stGetSetParam);
#else
    get(element, 0, p_stGetSetParam);
#endif
}

void fn_ucGetDsgVar(s32 ucId, s32 ucIndex, AI_tdstMind_* p_stPerso, tdstGetSetParam_* p_stGetSetParam)
{
    tdstDsgMem_* memory = p_stPerso->p_stDsgMem;
    RevoDsgDescription* description = *memory->p_p_stDsgVar;
    RevoDsgVarInfo* info = (RevoDsgVarInfo*) ((ucId & 0xFF) * sizeof(RevoDsgVarInfo) +
                                              (s32) description->variables);
    u8* values = memory->p_cDsgMemBuffer;

    a_stDsgVarTypeTable[info->typeIndex].get(values + info->bufferOffset, ucIndex & 0xFF, p_stGetSetParam);
}

void fn_ucSetDsgVarFloat(f32* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    f32 value;

    if (p_stGetSetParam->ucType == E_vt_Float) {
        value = p_stGetSetParam->u.xValue;
    } else {
        value = p_stGetSetParam->u.lValue;
    }
    *pMyVarAddr = value;
}

void fn_ucSetDsgVarPerso(void** pMyVarAddr, s32 ucIndex, HIE_tdstSuperObject_** p_stGetSetParam)
{
    HIE_tdstSuperObject_* superObject = *p_stGetSetParam;

    if (superObject != NULL) {
        *pMyVarAddr = inv_HIE_fn_p_stGetEngineObject(superObject)->h_MSWay;
    } else {
        *pMyVarAddr = NULL;
    }
}

void fn_ucSetDsgVarSuperObject(void** pMyVarAddr, s32 ucIndex, HIE_tdstSuperObject_** p_stGetSetParam)
{
    HIE_tdstSuperObject_* superObject = *p_stGetSetParam;

    if (superObject != NULL) {
        *pMyVarAddr = inv_HIE_fn_p_stGetEngineObject(superObject)->h_MSWay;
    } else {
        *pMyVarAddr = NULL;
    }
}

void fn_ucSetDsgVarAction(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarComport(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarVector(MTH3D_tdstVector_* pMyVarAddr, void* ucIndex, MTH3D_tdstVector_* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarText(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarCaps(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarGraph(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarWayPoint(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarList(tdstList_* pMyVarAddr, s32 ucIndex, tdstList_** p_stGetSetParam)
{
    tdstList_* src = *p_stGetSetParam;
    pMyVarAddr->ucNbElt =
        pMyVarAddr->ucMaxSize < src->ucNbElt ? pMyVarAddr->ucMaxSize : src->ucNbElt;
    memcpy(pMyVarAddr->values, src->values, pMyVarAddr->ucNbElt << 2);
}

void fn_ucSetDsgVarBoolean(u8* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.ucValue;
}

void fn_ucSetDsgVar_128To127(u8* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.ucValue;
}

void fn_ucSetDsgVar0To255(u8* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.ucValue;
}

void fn_ucSetDsgVar_32768To32767(u16* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.uwValue;
}

void fn_ucSetDsgVar0To65535(u16* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.uwValue;
}

void fn_ucSetDsgVarInteger(s32* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.lValue;
}

void fn_ucSetDsgVarPositiveInteger(s32* pMyVarAddr, s32 ucIndex, tdstGetSetParam_* p_stGetSetParam)
{
    fn_ucIsThereFloatIfYesConvertItToInteger(p_stGetSetParam);
    *pMyVarAddr = p_stGetSetParam->u.lValue;
}

void fn_ucSetDsgVarGameMaterial(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarVisualMaterial(s32* pMyVarAddr, s32 ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarSoundEvent(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarSOLinks(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void func_001DECF0(s32* destination, void* unused, s32* source)
{
    *destination = *source;
}

void fn_ucSetDsgVarInput(s32* pMyVarAddr, void* ucIndex, s32* p_stGetSetParam)
{
    *pMyVarAddr = *p_stGetSetParam;
}

void fn_ucSetDsgVarArray(tdstArray_* pMyVarAddr, u8 ucIndex, void* p_stGetSetParam)
{
    u8* elements = pMyVarAddr->d_ArrayElement;
    s32 type = pMyVarAddr->eType;
    DsgVarAccessFn set;
    void* element;

    set = a_stDsgVarTypeTable[type].set;
    if (type == eDsgVarType_Vector) {
        element = elements + ucIndex * 12;
    } else {
        element = elements + ucIndex * 4;
    }
    set(element, 0, p_stGetSetParam);
}

void fn_ucSetDsgVar(s32 ucId, s32 ucIndex, AI_tdstMind_* p_stPerso, void* p_stGetSetParam)
{
    tdstDsgMem_* memory = p_stPerso->p_stDsgMem;
    RevoDsgDescription* description = *memory->p_p_stDsgVar;
    RevoDsgVarInfo* info = (RevoDsgVarInfo*) ((ucId & 0xFF) * sizeof(RevoDsgVarInfo) +
                                              (s32) description->variables);
    u8* values = memory->p_cDsgMemBuffer;

    a_stDsgVarTypeTable[info->typeIndex].set(values + info->bufferOffset, ucIndex & 0xFF, p_stGetSetParam);
}
