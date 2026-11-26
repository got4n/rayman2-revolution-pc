#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"

extern tdstIntelligence_* g_p_stIntelligence;
extern u8 ucIsInSchedule;
extern u8 inv_AI_ucIntelligenceDepth;

void inv_AI_fn_vEvalInitComport(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object;
    tdstBrain_* brain;

    ucIsInSchedule = 0;
    inv_AI_ucIntelligenceDepth++;
    object = superObject->hLinkedObject;
    brain = object->h_Brain;
    if (brain != NULL) {
        AI_tdstMind_* mind = brain->p_stMind;
        tdstComport_* comport = inv_AI_fn_p_stModelOf(mind)->inv_p_stInitComport;

        if (comport != NULL) {
            tdstGetSetParam_* param = inv_AI_parameterPoolCursor;
            tdstNodeInterpret_* tree = *comport->p_stRules;
            tdstIntelligence_* intelligence = mind->p_stIntelligence;

            inv_AI_parameterPoolCursor = param + 1;
            g_p_stIntelligence = intelligence;
            fn_p_stIntelligenceEvalTreeEngine(superObject, tree, param);
            inv_AI_parameterPoolCursor = param;
        }
    }
    inv_AI_ucIntelligenceDepth--;
}

void func_001DEE58(HIE_tdstSuperObject_* superObject, tdstEngineObject_* unused)
{
    tdstEngineObject_* object;
    tdstBrain_* brain;

    inv_AI_ucIntelligenceDepth++;
    ucIsInSchedule = 0;
    object = superObject->hLinkedObject;
    brain = object->h_Brain;
    if (brain != NULL) {
        tdstComport_* comport = inv_AI_fn_p_stModelOf(brain->p_stMind)->inv_p_stDesinitComport;

        if (comport != NULL) {
            tdstGetSetParam_* param = inv_AI_parameterPoolCursor;
            tdstNodeInterpret_* tree = *comport->p_stRules;

            inv_AI_parameterPoolCursor = param + 1;
            fn_p_stIntelligenceEvalTreeEngine(superObject, tree, param);
            inv_AI_parameterPoolCursor = param;
        }
    }
    inv_AI_ucIntelligenceDepth--;
}

typedef void* (*AIEvalFn)(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                          tdstGetSetParam_* out);
typedef void* (*AIConditionFn)(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                               s32* result);
typedef void* (*AIProcedureFn)(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream);
typedef tdstNodeInterpret_* (*AIMetaActionInitFn)(HIE_tdstSuperObject_* superObject,
                                                  tdstNodeInterpret_* stream);
typedef void* (*AIMetaActionFn)(HIE_tdstSuperObject_* superObject);

typedef struct AIMacro {
    tdstNodeInterpret_* first;
    tdstNodeInterpret_* current;
} AIMacro;

typedef struct tdstMetaActionEntry_ {
    AIMetaActionInitFn p_fn_p_stMetaActionInitFunction;
    AIMetaActionFn p_fn_ucMetaActionFunction;
    char szMetaActionTypeParam[9];
} tdstMetaActionEntry_;

extern AIEvalFn a_stTypeTable[];
extern AIEvalFn a_stKeyWordTable[];
extern AIConditionFn a_stConditionTable[];
extern AIEvalFn a_stFunctionTable[];
extern AIEvalFn a_stOperatorTable[];
extern AIProcedureFn a_stProcedureTable[];
extern tdstMetaActionEntry_ a_stMetaActionTable[];

extern u8 g_ucIsInMacro;
extern tdstNodeInterpret_* g_p_stTreeMacro;
extern u8 g_ucNRule;
extern u8 bSomethingDone;

tdstNodeInterpret_* fn_p_stGetTableAction(tdstIntelligence_* intelligence, u32 index);

static inline tdstComport_* inv_AI_fn_p_stBehaviorOf(tdstIntelligence_* intelligence)
{
    return intelligence->p_stCurrentComport;
}

void* fn_p_stEvalMacro(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    AIMacro* macro;
    tdstNodeInterpret_* saved;
    tdstNodeInterpret_* action;
    u32 index;
    u8 flags;

    macro = (AIMacro*) p_stTree->param;
    saved = g_p_stTreeMacro;
    g_ucIsInMacro++;
    g_p_stTreeMacro = p_stTree;
    index = fn_ucIsRuleInActionTable(g_p_stIntelligence, g_ucNRule);
    if (index >= g_p_stIntelligence->p_stActionTable->ucNbEntries ||
        fn_p_stGetTableAction(g_p_stIntelligence, index) != g_p_stTreeMacro) {
        macro->current = macro->first;
    }
    fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, macro->current, p_stValue);
    flags = p_stValue->u.aucValue[0];
    g_ucIsInMacro--;
    if (flags & 0x10) {
        macro->current = macro->first;
        p_stValue->ucType = E_vt_MetaActionReturn;
        p_stValue->u.lValue = 0;
        g_p_stTreeMacro = saved;
        return p_stTree + 1;
    }
    if (flags & 8) {
        macro->current = macro->first;
        g_p_stTreeMacro = saved;
        return p_stTree + 1;
    }
    index = fn_ucIsRuleInActionTable(g_p_stIntelligence, g_ucNRule);
    if (index >= g_p_stIntelligence->p_stActionTable->ucNbEntries) {
        return p_stTree;
    }
    action = fn_p_stGetTableAction(g_p_stIntelligence, index);
    if (action->eType != E_ti_MacroRef) {
        macro->current = action;
        fn_vUnuseCurrentActionTableEntry(g_p_stIntelligence);
        fn_vUseCurrentActionTableEntry(g_p_stIntelligence, g_ucNRule, g_p_stTreeMacro);
    }
    g_p_stTreeMacro = saved;
    return p_stTree;
}

void* fn_p_stEvalColor(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Color;
    return p_stTree + 1;
}

void* fn_p_stEvalModelRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Model;
    return p_stTree + 1;
}

void* fn_p_stEvalField(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    fn_ucGetFieldValue(p_stTree->param, p_SuperObjPerso, p_stValue);
    return p_stTree + 1;
}

void* fn_p_stEvalDsgVarRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    tdstEngineObject_* object = p_SuperObjPerso->hLinkedObject;
    tdstBrain_* brain = object->h_Brain;

    fn_ucGetDsgVar((u8) p_stTree->param, 0, brain->p_stMind, p_stValue);
    return p_stTree + 1;
}

void* fn_p_stEvalConstant(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Integer;
    return p_stTree + 1;
}

void* fn_p_stEvalButton(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                        tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Button;
    return p_stTree + 1;
}

void* fn_p_stEvalSuperObjectRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_SuperObject;
    return p_stTree + 1;
}

void* fn_p_stEvalObjectTableRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_ObjectTable;
    return p_stTree + 1;
}

void* fn_p_stEvalPersoRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Perso;
    return p_stTree + 1;
}

void* fn_p_stEvalActionRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Action;
    return p_stTree + 1;
}

typedef union NodeReal {
    u32 bits;
    f32 value;
} NodeReal;

void* fn_p_stEvalReal(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                      tdstGetSetParam_* p_stValue)
{
    NodeReal real;
    f32 value;

    real.bits = p_stTree->param;
    value = real.value;

    p_stValue->ucType = E_vt_Float;
    p_stValue->u.xValue = value;
    return p_stTree + 1;
}

void* fn_p_stEvalVector(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                        tdstGetSetParam_* p_stValue)
{
    f32 x;
    f32 y;
    f32 z;

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree + 1, p_stValue);
    x = inv_AI_fn_xAiParamFloat(p_stValue);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    y = inv_AI_fn_xAiParamFloat(p_stValue);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    z = inv_AI_fn_xAiParamFloat(p_stValue);
    p_stValue->ucType = E_vt_Vector;
    MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, x, y, z);
    return p_stTree;
}

void* fn_p_stEvalConstantVector(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                tdstGetSetParam_* p_stValue)
{
    MTH3D_tdstVector_* source = (MTH3D_tdstVector_*) p_stTree->param;

    p_stValue->u.stVector = *source;
    p_stValue->ucType = E_vt_Vector;
    return p_stTree + 1;
}

void* fn_p_stEvalWayPointRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                             tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_WayPoint;
    return p_stTree + 1;
}

void* fn_p_stEvalModuleRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Module;
    return p_stTree + 1;
}

void* fn_p_stEvalDsgVarId(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_DsgVarId;
    return p_stTree + 1;
}

void* fn_p_stEvalMask(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                      tdstGetSetParam_* p_stValue)
{
    s32 value = (u16) p_stTree->param;

    p_stValue->ucType = E_vt_Mask;
    p_stValue->u.lValue = value;
    return p_stTree + 1;
}

void* fn_p_stEvalString(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                        tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_String;
    return p_stTree + 1;
}

void* fn_p_stEvalSoundEventRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                               tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_SoundEvent;
    return p_stTree + 1;
}

void* fn_p_stEvalFamilyRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Family;
    return p_stTree + 1;
}

void* fn_p_stEvalEndOfTree(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0x10;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return p_stTree;
}

void* fn_p_stEvalBeginMacro(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return p_stTree + 1;
}

void* fn_p_stEvalEndMacro(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return p_stTree + 1;
}

void* fn_p_stEvalComportRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Comport;
    return p_stTree + 1;
}

void* fn_p_stEvalTextRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                         tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Text;
    return p_stTree + 1;
}

void* fn_p_stEvalSOLinksRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                   tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_SOLinks;
    return p_stTree + 1;
}

void* fn_p_stEvalGameMaterialRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                 tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_GameMaterial;
    return p_stTree + 1;
}

void* fn_p_stEvalVisualMaterialRef(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                   tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_VisualMaterial;
    return p_stTree + 1;
}

void* fn_p_stEvalLight(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Light;
    return p_stTree + 1;
}

void* fn_p_stEvalCaps(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                      tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Caps;
    return p_stTree + 1;
}

void* fn_p_stEvalGraph(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = p_stTree->param;
    p_stValue->ucType = E_vt_Graph;
    return p_stTree + 1;
}

void* fn_p_stEvalNULL(HIE_tdstSuperObject_* p_stContext, tdstNodeInterpret_* p_stTree)
{
    return p_stTree + 1;
}

void* fn_p_stEvalKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                         tdstGetSetParam_* p_stValue)
{
    return a_stKeyWordTable[(u16) p_stTree->param](p_SuperObjPerso, p_stTree + 1, p_stValue);
}

void* fn_p_stEvalCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    s32 result = 0;
    void* next = a_stConditionTable[(u16) p_stTree->param](p_SuperObjPerso, p_stTree + 1, &result);

    p_stValue->ucType = E_vt_Integer;
    p_stValue->u.lValue = result != 0;
    return next;
}

void* fn_p_stEvalFunction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    return a_stFunctionTable[(u16) p_stTree->param](p_SuperObjPerso, p_stTree + 1, p_stValue);
}

void* fn_p_stEvalOperator(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          tdstGetSetParam_* p_stValue)
{
    return a_stOperatorTable[(u16) p_stTree->param](p_SuperObjPerso, p_stTree + 1, p_stValue);
}

void* fn_p_stEvalProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           tdstGetSetParam_* p_stValue)
{
    u32 index;
    u32 flags;
    AIProcedureFn procedure;
    void* next;

    index = (u16) p_stTree->param;
    ucProcedureActionReturn = 0;
    procedure = a_stProcedureTable[index];
    next = procedure(p_SuperObjPerso, p_stTree + 1);
    flags = ucProcedureActionReturn;
    p_stValue->ucType = E_vt_MetaActionReturn;
    p_stValue->u.lValue = flags & 0xFB;
    return next;
}

tdstNodeInterpret_* fn_p_stSwapNextParameter(tdstNodeInterpret_* p_stTree)
{
    u8 ucDepth = p_stTree->ucDepth;

    p_stTree++;
    while (p_stTree->ucDepth >= ucDepth && p_stTree->eType != E_ti_EndTree) {
        p_stTree++;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stSwapNextParameter2(tdstNodeInterpret_* p_stTree)
{
    u8 ucDepth = p_stTree[-1].ucDepth;

    while (ucDepth < p_stTree->ucDepth && p_stTree->eType != E_ti_EndTree) {
        p_stTree++;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stSkipThisArgument(tdstNodeInterpret_* p_stTree)
{
    u32 ucDepth = p_stTree->ucDepth;

    p_stTree++;
    while (ucDepth < p_stTree->ucDepth && p_stTree->eType != E_ti_EndTree) {
        p_stTree++;
    }
    return p_stTree;
}

void* func_001DF6A0(HIE_tdstSuperObject_* superObject, u16 index)
{
    return a_stMetaActionTable[index].p_fn_ucMetaActionFunction(superObject);
}

void* func_001DF6D8(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream, u16 index,
                    tdstGetSetParam_* out)
{
    void* next = a_stMetaActionTable[index].p_fn_p_stMetaActionInitFunction(superObject, stream);

    out->u.pvValue = func_001DF6A0(superObject, index);
    out->ucType = E_vt_MetaActionReturn;
    return next;
}

void* fn_p_stEvalMetaAction(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            tdstGetSetParam_* p_stValue)
{
    tdstNodeInterpret_* p_stTreeInit;
    u16 eMetaActionId;

    p_stTreeInit = g_p_stTreeMacro;
    if (g_ucIsInMacro == 0) {
        p_stTreeInit = p_stTree;
    }
    g_p_stIntelligence->p_stActionTable->ucCurrentEntry =
        fn_ucIsRuleAndNodeInActionTable(g_p_stIntelligence, g_ucNRule, p_stTreeInit);
    if (fn_ucIsRuleAndNodeInActionTable(g_p_stIntelligence, g_ucNRule, p_stTreeInit) <
        g_p_stIntelligence->p_stActionTable->ucNbEntries) {
        eMetaActionId = p_stTree->param;
        p_stTree += p_stTree->uwNodeToSkip;
        p_stValue->u.pvValue = func_001DF6A0(p_SuperObjPerso, eMetaActionId);
        p_stValue->ucType = E_vt_MetaActionReturn;
        bSomethingDone = 1;
        return p_stTree;
    }
    fn_vUnuseAllRulesWhichHaveGreaterNumRule(g_p_stIntelligence, g_ucNRule);
    g_p_stIntelligence->p_stActionTable->ucCurrentEntry = fn_ucFindPlaceInActionTable(g_p_stIntelligence);
    if (fn_ucFindPlaceInActionTable(g_p_stIntelligence) < g_p_stIntelligence->p_stActionTable->ucNbEntries) {
        fn_vUseCurrentActionTableEntry(g_p_stIntelligence, g_ucNRule, p_stTree);
        eMetaActionId = p_stTree->param;
        bSomethingDone = 1;
        return func_001DF6D8(p_SuperObjPerso, p_stTree + 1, eMetaActionId, p_stValue);
    }
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return NULL;
}

void* fn_p_stEvalTree(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                      tdstGetSetParam_* p_stValue)
{
    return a_stTypeTable[p_stTree->eType](p_SuperObjPerso, p_stTree, p_stValue);
}

u8 fn_ucGetNewActionReturnValue(tdstGetSetParam_* result, tdstNodeInterpret_* node, u8 depth)
{
    u8 saved = g_ucNewActionReturn;
    u8 flags = result->u.aucValue[0];

    if ((saved & 0x20) == 0) {
        if (g_ucUseDefaultActionReturn == 0) {
            if ((flags & 4) == 0) {
                flags &= 0x1C;
                flags = saved | flags;
            } else if (node->ucDepth != depth) {
                flags &= 0x1C;
                flags = saved | flags;
            }
            g_ucNewActionReturn = 0x20;
            g_ucUseDefaultActionReturn = 1;
        } else {
            g_ucUseDefaultActionReturn = 0;
        }
    }
    return flags;
}

tdstNodeInterpret_* fn_p_stIntelligenceEvalTreeEngine(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                      tdstNodeInterpret_* p_stTree,
                                                      tdstGetSetParam_* p_stValue)
{
    u8 ucDepthInit;
    u32 ucActionReturn;

next:
    ucDepthInit = p_stTree->ucDepth;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    ucActionReturn = fn_ucGetNewActionReturnValue(p_stValue, p_stTree, ucDepthInit);
    if (ucActionReturn & 0x18) {
        p_stValue->u.lValue = ucActionReturn;
        p_stValue->ucType = E_vt_MetaActionReturn;
        return p_stTree;
    }
    if (ucActionReturn & 4) {
        fn_vUnuseCurrentActionTableEntry(g_p_stIntelligence);
    }
    if (ucActionReturn & 1) {
        fn_vUnuseAllRulesWhichHaveGreaterNumRule(g_p_stIntelligence, g_ucNRule);
        p_stValue->u.lValue = ucActionReturn;
        p_stValue->ucType = E_vt_MetaActionReturn;
        return p_stTree;
    }
    if (p_stTree->eType != E_ti_EndTree) {
        if (ucActionReturn & 2) {
            goto done;
        }
        if (ucIsInSchedule != 0) {
            g_p_stIntelligence->p_stCurrentSchedule = p_stTree;
        }
        goto next;
    }
    ucActionReturn = 0x10;
done:
    p_stValue->u.lValue = ucActionReturn;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return p_stTree;
}

s32 fn_ucIntelligenceRulesEngine(HIE_tdstSuperObject_* p_SuperObjPerso, tdstIntelligence_* p_stIntelligence)
{
    tdstGetSetParam_* result;
    tdstNodeInterpret_* node;
    s32 value;
    s32 rule;
    s32 next;
    s32 last;
    s32 entryCount;

    result = inv_AI_parameterPoolCursor;
    ucIsInSchedule = 0;
    g_p_stIntelligence = p_stIntelligence;
    inv_AI_parameterPoolCursor = result + 1;
    value = 0;

restart:
    rule = 0;
    entryCount = p_stIntelligence->p_stActionTable->ucNbEntries;
    last = (s8) (inv_AI_fn_p_stBehaviorOf(p_stIntelligence)->ucNbRules - 1);
    if (last < 0) {
        goto done;
    }
    do {
        next = rule + 1;
        g_ucNRule = next;
        value = fn_ucIsRuleInActionTable(p_stIntelligence, g_ucNRule);
        if ((u32) value < (u32) entryCount) {
            node = fn_p_stGetTableAction(p_stIntelligence, value);
        } else {
            node = inv_AI_fn_p_stBehaviorOf(p_stIntelligence)->p_stRules[rule];
        }
        fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, node, result);
        value = result->u.aucValue[0];
        if (value & 1) {
            value = 1;
            goto done;
        }
        if (value & 8) {
            bSomethingDone = 0;
            goto restart;
        }
        if ((value & 0x10) == 0) {
            value = 0;
            goto done;
        }
        rule = (s8) next;
    } while (rule <= last);
done:
    inv_AI_parameterPoolCursor = result;
    return value;
}

s32 fn_ucIntelligenceScheduleEngine(HIE_tdstSuperObject_* p_SuperObjPerso,
                                    tdstIntelligence_* p_stIntelligence)
{
    tdstNodeInterpret_* node;
    tdstGetSetParam_* result;
    u32 index;
    s32 flags;

    ucIsInSchedule = 1;
    node = p_stIntelligence->p_stCurrentSchedule;
    g_p_stIntelligence = p_stIntelligence;
    if (node == NULL) {
        return 0x10;
    }

    g_ucNRule = 0;
    index = fn_ucIsRuleInActionTable(p_stIntelligence, 0);
    if (index < p_stIntelligence->p_stActionTable->ucNbEntries) {
        node = fn_p_stGetTableAction(p_stIntelligence, index);
    }
    if (node->eType == E_ti_EndTree) {
        node = inv_AI_fn_p_stBehaviorOf(p_stIntelligence)->p_stSchedule;
    }

    result = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = result + 1;
    node = fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, node, result);
    flags = result->u.aucValue[0];
    inv_AI_parameterPoolCursor = result;
    if ((flags & 0xA) == 0) {
        p_stIntelligence->p_stCurrentSchedule = node;
    }
    return flags;
}

void func_001DFC28(tdstIntelligence_* intelligence)
{
    u32 index = fn_ucIsRuleInActionTable(intelligence, 0);

    if (index < intelligence->p_stActionTable->ucNbEntries) {
        intelligence->p_stCurrentSchedule = fn_p_stGetTableAction(intelligence, index);
        fn_vUnuseAllRulesFromSchedule(intelligence);
    }
}

s32 fn_bIntelligenceEngine(HIE_tdstSuperObject_* p_SuperObjPerso, tdstIntelligence_* p_stIntelligence)
{
    u8 ReturnValue = 0;
    s32 ucReturn;

    do {
        bSomethingDone = 0;
        ucReturn = fn_ucIntelligenceRulesEngine(p_SuperObjPerso, p_stIntelligence);
        if ((ucReturn & 8) == 0) {
            if ((ucReturn & 1) != 0 || bSomethingDone != 0) {
                func_001DFC28(p_stIntelligence);
            } else {
                ucReturn = fn_ucIntelligenceScheduleEngine(p_SuperObjPerso, p_stIntelligence);
            }
        }
        ReturnValue |= bSomethingDone;
    } while ((ucReturn & 9) == 8);
    return ReturnValue;
}
