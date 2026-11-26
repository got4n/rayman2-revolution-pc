#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"

#include "ps2/copy_types.h"

#define DSG_VAR_KEEP_FIRST_BYTE 10

void ReinitDesignerVariables(AI_tdstMind_* mind);
void func_001DDC18(tdstEngineObject_* object);
void func_001DDE88(tdstEngineObject_* object);
void fn_vReinitAIComport(AI_tdstMind_* mind);
char* fn_p_cSimulateAllocDsgMemBuffer(char** arena, u32 size);
tdstActionTable_* fn_p_stSimulateAllocActionTable(char** arena, u8 count);

static inline RevoDsgDescription* inv_AI_fn_p_stMindDsgDescription(AI_tdstMind_* mind)
{
    return inv_AI_fn_p_stModelOf(mind)->p_stDsgVar;
}

static inline RevoDsgVarInfo* inv_AI_fn_p_stDsgVariableAt(tdstDsgMem_* memory, u32 tableOffset)
{
    return (RevoDsgVarInfo*) (tableOffset + (u32) (*memory->p_p_stDsgVar)->variables);
}

static inline RevoDsgVarInfo* inv_AI_fn_p_stMindDsgVariableAt(AI_tdstMind_* mind, u32 tableOffset)
{
    return inv_AI_fn_p_stDsgVariableAt(mind->p_stDsgMem, tableOffset);
}

AI_tdstMind_* fn_pstAllocMind(void)
{
    AI_tdstMind_* mind = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(AI_tdstMind_));

    mind->p_stAIModel = NULL;
    mind->p_stIntelligence = NULL;
    mind->p_stReflex = NULL;
    mind->p_stDsgMem = NULL;
    mind->bDoingIntel = 0;
    return mind;
}

void fn_vInitAI(s32 unused)
{
    g_ucNewActionReturn = 32;
    g_ucUseDefaultActionReturn = 1;
}

s32 fn_bDoIntelligencePerso(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    tdstEngineObject_* object = p_SuperObjPerso->hLinkedObject;
    tdstBrain_* brain = object->h_Brain;
    AI_tdstMind_* mind = brain->p_stMind;
    tdstIntelligence_* intelligence = mind->p_stIntelligence;

    mind->bDoingIntel = 1;
    if (intelligence != NULL) {
        return fn_bIntelligenceEngine(p_SuperObjPerso, intelligence);
    }
    return 0;
}

s32 fn_bDoReflexPerso(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    tdstEngineObject_* object = p_SuperObjPerso->hLinkedObject;
    tdstBrain_* brain = object->h_Brain;
    AI_tdstMind_* mind = brain->p_stMind;
    tdstIntelligence_* reflex = mind->p_stReflex;

    mind->bDoingIntel = 0;
    if (reflex != NULL) {
        return fn_bIntelligenceEngine(p_SuperObjPerso, reflex);
    }
    return 0;
}

void fn_vReinitDesignerVariablesWithFlags(AI_tdstMind_* p_stMind, u8 eObjIniType)
{
    u8* current;
    u8* initial;
    u8* defaults;
    u32 stride;
    u32 tableOffset;
    u32 offset;
    u32 size;
    u8 type;
    u8 i;

    if (p_stMind == NULL || p_stMind->p_stDsgMem == NULL) {
        return;
    }
    current = p_stMind->p_stDsgMem->p_cDsgMemBuffer;
    if (p_stMind->p_stDsgMem->p_cDsgMemBufferInit == NULL) {
        initial = inv_AI_fn_p_stMindDsgDescription(p_stMind)->defaultValues;
    } else {
        initial = p_stMind->p_stDsgMem->p_cDsgMemBufferInit;
    }
    i = 0;
    if (inv_AI_fn_p_stMindDsgDescription(p_stMind)->variableCount == 0) {
        return;
    }
    stride = sizeof(RevoDsgVarInfo);
    do {
        tableOffset = i * stride;
        type = inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->typeIndex;
        if (type == DSG_VAR_KEEP_FIRST_BYTE) {
            u32 flagOffset = inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->bufferOffset;

            *(current + flagOffset) = 0;
            *(current + flagOffset + 1) = *(initial + flagOffset + 1);
        } else if (inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->reinitThreshold >= eObjIniType) {
            offset = inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->bufferOffset;
            if (fn_GetDsgVarType(type) == E_vt_Array) {
                defaults = inv_AI_fn_p_stMindDsgDescription(p_stMind)->defaultValues;
                size = fn_ulSizeOfDsgVar(
                    type, (defaults + inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->bufferOffset)[5]);
            } else {
                size = fn_ulSizeOfDsgVar(type, 0);
            }
            memcpy(current + offset, initial + offset, size);
        }
        i++;
    } while (i < inv_AI_fn_p_stMindDsgDescription(p_stMind)->variableCount);
}

void ReinitDesignerVariables(AI_tdstMind_* p_stMind)
{
    u8* current;
    u8* initial;
    u8* defaults;
    u32 tableOffset;
    u32 offset;
    u32 size;
    u8 type;
    u8 i;

    if (p_stMind == NULL || p_stMind->p_stDsgMem == NULL) {
        return;
    }
    current = p_stMind->p_stDsgMem->p_cDsgMemBuffer;
    if (p_stMind->p_stDsgMem->p_cDsgMemBufferInit == NULL) {
        initial = inv_AI_fn_p_stMindDsgDescription(p_stMind)->defaultValues;
    } else {
        initial = p_stMind->p_stDsgMem->p_cDsgMemBufferInit;
    }
    for (i = 0; i < inv_AI_fn_p_stMindDsgDescription(p_stMind)->variableCount; i++) {
        tableOffset = i * sizeof(RevoDsgVarInfo);
        type = inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->typeIndex;
        if (inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->policyFlags == 0) {
            offset = inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->bufferOffset;
            if (fn_GetDsgVarType(type) == E_vt_Array) {
                defaults = inv_AI_fn_p_stMindDsgDescription(p_stMind)->defaultValues;
                size = fn_ulSizeOfDsgVar(
                    type, (defaults + inv_AI_fn_p_stMindDsgVariableAt(p_stMind, tableOffset)->bufferOffset)[5]);
            } else {
                size = fn_ulSizeOfDsgVar(type, 0);
            }
            memcpy(current + offset, initial + offset, size);
        }
    }
}

void func_001DDC18(tdstEngineObject_* object)
{
    tdstBrain_* brain = object->h_Brain;
    AI_tdstMind_* mind;
    tdstDsgMem_* memory;
    RevoDsgVarInfo* variable;
    u8* cursor;
    u8* values;
    u8* value;
    u32 size;
    u8 type;
    u8 i;

    if (brain == NULL) {
        return;
    }
    mind = brain->p_stMind;
    if (mind == NULL || mind->p_stDsgMem == NULL) {
        return;
    }
    for (i = 0; i < inv_AI_fn_p_stMindDsgDescription(mind)->variableCount; i++) {
        if (inv_AI_fn_p_stMindDsgVariableAt(mind, i * sizeof(RevoDsgVarInfo))->policyFlags != 0) {
            break;
        }
    }
    if (i == inv_AI_fn_p_stMindDsgDescription(mind)->variableCount) {
        return;
    }
    cursor = inv_SAV_fn_p_ucGetOrCreateSaveRecord(object->h_StandardGame->inv_uwSaveId);
    if (cursor == NULL) {
        return;
    }
    for (i = 0; i < inv_AI_fn_p_stMindDsgDescription(mind)->variableCount; i++) {
        memory = mind->p_stDsgMem;
        variable = inv_AI_fn_p_stDsgVariableAt(memory, i * sizeof(RevoDsgVarInfo));
        if (variable->policyFlags != 0) {
            values = memory->p_cDsgMemBuffer;
            value = values + variable->bufferOffset;
            type = variable->typeIndex;
            size = 0;
            switch (type) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 11:
            case 14:
                size = fn_ulSizeOfDsgVar(type, 0);
                break;
            case 21:
            case 22:
            case 23:
                size = fn_ulSizeOfDsgVar(type, value[5]);
                break;
            }
            cursor = inv_SAV_fn_p_ucWriteAligned(cursor, value, size);
        }
    }
}

void func_001DDDD0(tdstEngineObject_* object)
{
    tdstBrain_* brain;
    AI_tdstMind_* mind;

    if (object == NULL) {
        return;
    }
    brain = object->h_Brain;
    if (brain == NULL) {
        return;
    }
    if ((object->h_StandardGame->ucMiscFlags >> 4) & 1) {
        return;
    }
    mind = brain->p_stMind;
    if (mind != NULL && mind->p_stDsgMem != NULL && inv_AI_fn_p_stMindDsgDescription(mind) != NULL) {
        func_001DDC18(object);
    }
    if ((object->h_StandardGame->ulCustomBits & 0x80000000) == 0) {
        return;
    }
    inv_SAV_fn_vWriteHeader(0, &object->h_StandardGame->ucHitPoints, 1);
    inv_SAV_fn_vWriteHeader(1, &object->h_StandardGame->ucHitPointsMax, 1);
}

void func_001DDE88(tdstEngineObject_* object)
{
    tdstBrain_* brain = object->h_Brain;
    AI_tdstMind_* mind;
    tdstDsgMem_* memory;
    RevoDsgVarInfo* variable;
    u8* cursor;
    u8* initialValues;
    u8* currentValues;
    u8* saved;
    u8* current;
    u32 size;
    u8 type;
    u8 i;

    if (brain == NULL) {
        return;
    }
    mind = brain->p_stMind;
    if (mind == NULL) {
        return;
    }
    if (mind->p_stDsgMem == NULL) {
        return;
    }
    if (mind->p_stDsgMem->p_cDsgMemBufferInit == NULL) {
        return;
    }
    cursor = inv_SAV_fn_p_ucFindSaveRecord(object->h_StandardGame->inv_uwSaveId);
    if (cursor == NULL) {
        return;
    }
    for (i = 0; i < inv_AI_fn_p_stMindDsgDescription(mind)->variableCount; i++) {
        memory = mind->p_stDsgMem;
        variable = inv_AI_fn_p_stDsgVariableAt(memory, i * sizeof(RevoDsgVarInfo));
        if (variable->policyFlags != 0) {
            initialValues = memory->p_cDsgMemBufferInit;
            currentValues = memory->p_cDsgMemBuffer;
            saved = initialValues + variable->bufferOffset;
            current = currentValues + variable->bufferOffset;
            type = variable->typeIndex;
            size = 0;
            switch (type) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 11:
            case 14:
                size = fn_ulSizeOfDsgVar(type, 0);
                break;
            case 21:
            case 22:
            case 23:
                size = fn_ulSizeOfDsgVar(type, saved[5]);
                break;
            }
            cursor = inv_SAV_fn_p_ucReadAligned(cursor, saved, size);
            memcpy(current, saved, size);
        }
    }
}

void func_001DDFF8(tdstEngineObject_* object)
{
    tdstBrain_* brain;
    AI_tdstMind_* mind;

    if (object == NULL) {
        return;
    }
    brain = object->h_Brain;
    if (brain == NULL) {
        return;
    }
    mind = brain->p_stMind;
    if (mind != NULL && mind->p_stDsgMem != NULL && inv_AI_fn_p_stMindDsgDescription(mind) != NULL) {
        func_001DDE88(object);
    }
    if ((object->h_StandardGame->ulCustomBits & 0x80000000) == 0) {
        return;
    }
    if (inv_SAV_fn_p_ucFindSaveRecord(object->h_StandardGame->inv_uwSaveId) == NULL) {
        return;
    }
    inv_SAV_fn_vReadHeader(0, &object->h_StandardGame->ucHitPoints, 1);
    object->h_StandardGame->ucSaveHitPoints = object->h_StandardGame->ucHitPoints;
    inv_SAV_fn_vReadHeader(1, &object->h_StandardGame->ucHitPointsMax, 1);
    object->h_StandardGame->ucSaveHitPointsMax = object->h_StandardGame->ucHitPointsMax;
}

void fn_vSecondPassOfMindLoading(AI_tdstMind_* p_stMind)
{
}

void fn_vReinitAIComport(AI_tdstMind_* mind)
{
    fn_vSetUltraOperatorPerso(0, 0);
    if (mind != NULL) {
        if (mind->p_stIntelligence != NULL) {
            fn_vReinitComport(mind->p_stIntelligence);
        }
        if (mind->p_stReflex != NULL) {
            fn_vReinitComport(mind->p_stReflex);
        }
    }
}

void fn_vReinitAI(AI_tdstMind_* p_stMind)
{
    if (p_stMind != NULL) {
        fn_vReinitAIComport(p_stMind);
        ReinitDesignerVariables(p_stMind);
    }
}

void fn_vReinitAIComportWithFlags(AI_tdstMind_* mind, u8 flags)
{
    fn_vReinitAIComport(mind);
}

void fn_vReinitAIWithFlags(AI_tdstMind_* p_stMind, u8 eObjIniType)
{
    if (p_stMind != NULL) {
        fn_vReinitAIComportWithFlags(p_stMind, eObjIniType);
        fn_vReinitDesignerVariablesWithFlags(p_stMind, eObjIniType);
    }
}

char* fn_p_cSimulateAllocDsgMemBuffer(char** arena, u32 size)
{
    char* buffer = *arena;

    *arena = buffer + size;
    memset(buffer, 0, size);
    return buffer;
}

tdstDsgMem_* fn_p_stSimulateAllocDsgMem(char** p_ptr, RevoDsgDescription** p_p_stDsgVar, u32 ulSize)
{
    tdstDsgMem_* p_stDsgMem = (tdstDsgMem_*) *p_ptr;

    memset(p_stDsgMem, 0, sizeof(tdstDsgMem_));
    *p_ptr += sizeof(tdstDsgMem_);
    p_stDsgMem->p_p_stDsgVar = p_p_stDsgVar;
    p_stDsgMem->p_cDsgMemBufferInit = NULL;
    p_stDsgMem->p_cDsgMemBuffer = fn_p_cSimulateAllocDsgMemBuffer(p_ptr, ulSize);
    return p_stDsgMem;
}

tdstActionTableEntry_* fn_p_stSimulateAllocActionTableEntry(char** arena, u8 count)
{
    u32 size = count * sizeof(tdstActionTableEntry_);
    tdstActionTableEntry_* entries = (tdstActionTableEntry_*) *arena;

    memset(entries, 0, size);
    *arena += size;
    return entries;
}

tdstActionTable_* fn_p_stSimulateAllocActionTable(char** p_ptr, u8 ucActionTableNbEntry)
{
    tdstActionTable_* table;
    tdstActionTableEntry_* entry;
    u8 i;

    if (ucActionTableNbEntry == 0) {
        return NULL;
    }
    table = (tdstActionTable_*) *p_ptr;
    memset(table, 0, sizeof(tdstActionTable_));
    *p_ptr += sizeof(tdstActionTable_);
    table->ucNbEntries = ucActionTableNbEntry;
    table->p_stEntry = fn_p_stSimulateAllocActionTableEntry(p_ptr, ucActionTableNbEntry);
    table->ucNbEntriesUsed = 0;
    entry = table->p_stEntry;
    table->ucCurrentEntry = 0;
    for (i = 0; i < ucActionTableNbEntry; i++) {
        entry->bUseDefaultActionReturn = g_ucUseDefaultActionReturn;
        entry->ucNewActionReturn = g_ucNewActionReturn;
        entry++;
    }
    return table;
}

static inline tdstScriptAI_* inv_AI_fn_p_stSlotScript(tdstScriptAI_** slot)
{
    return *slot;
}

tdstIntelligence_* fn_p_stSimulateAllocIntelligence(char** p_ptr, tdstScriptAI_** p_p_stScriptAI,
                                                    u8 actionCapacity)
{
    tdstIntelligence_* intelligence = NULL;

    if (inv_AI_fn_p_stSlotScript(p_p_stScriptAI)->ulNbComport != 0) {
        intelligence = (tdstIntelligence_*) *p_ptr;
        memset(intelligence, 0, sizeof(tdstIntelligence_));
        *p_ptr += sizeof(tdstIntelligence_);
        intelligence->p_p_stScriptAI = (void**) p_p_stScriptAI;
        intelligence->initialBehavior = inv_AI_fn_p_stSlotScript(p_p_stScriptAI)->ulNoComportInitDefault;
        intelligence->p_stActionTable = fn_p_stSimulateAllocActionTable(p_ptr, actionCapacity);
    }
    return intelligence;
}

void fn_vSimulateMindAllocationFromAnotherMind(AI_tdstMind_* p_stMindSrc, AI_tdstMind_* p_stMindDst)
{
    AI_tdstAIModel_* model;
    tdstScriptAI_** scriptSlot;
    tdstIntelligence_* allocated;
    tdstIntelligence_* intelligence;
    tdstDsgMem_* memory;
    char* arena;
    u32 address;

    if (p_stMindSrc == NULL || p_stMindDst == NULL) {
        return;
    }
    model = p_stMindSrc->p_stAIModel;
    arena = (char*) (p_stMindDst + 1);
    p_stMindDst->p_stAIModel = model;
    address = (u32) model & 0xFFFF0000u;
    address |= (u32) model & 0xFFFFu;
    scriptSlot = (tdstScriptAI_**) address;
    if (*scriptSlot != NULL) {
        allocated = fn_p_stSimulateAllocIntelligence(&arena, scriptSlot,
                                                     (*scriptSlot)->ucActionTableNbEntry);
        p_stMindDst->p_stIntelligence = allocated;
        intelligence = p_stMindSrc->p_stIntelligence;
        if (intelligence != NULL) {
            allocated->initialBehavior = intelligence->initialBehavior;
        }
    } else {
        p_stMindDst->p_stIntelligence = NULL;
    }
    intelligence = p_stMindSrc->p_stIntelligence;
    if (intelligence != NULL) {
        p_stMindDst->p_stIntelligence->initialBehavior = intelligence->initialBehavior;
    }
    if (model->p_stScriptAIReflex != NULL) {
        allocated = fn_p_stSimulateAllocIntelligence(
            &arena, &model->p_stScriptAIReflex, model->p_stScriptAIReflex->ucActionTableNbEntry);
        p_stMindDst->p_stReflex = allocated;
        intelligence = p_stMindSrc->p_stReflex;
        if (intelligence != NULL) {
            allocated->initialBehavior = intelligence->initialBehavior;
        }
    } else {
        p_stMindDst->p_stReflex = NULL;
    }
    p_stMindDst->p_stDsgMem = NULL;
    if (inv_AI_fn_p_stMindDsgDescription(p_stMindDst) != NULL &&
        inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->nominalBufferSize != 0) {
        p_stMindDst->p_stDsgMem =
            fn_p_stSimulateAllocDsgMem(&arena, &inv_AI_fn_p_stModelOf(p_stMindDst)->p_stDsgVar,
                                       inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->nominalBufferSize);
        *p_stMindDst->p_stDsgMem->p_p_stDsgVar = *p_stMindSrc->p_stDsgMem->p_p_stDsgVar;
        if (p_stMindSrc->p_stDsgMem->p_cDsgMemBufferInit != NULL) {
            p_stMindDst->p_stDsgMem->p_cDsgMemBufferInit = fn_p_cSimulateAllocDsgMemBuffer(
                &arena, inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->nominalBufferSize);
            memcpy(p_stMindDst->p_stDsgMem->p_cDsgMemBufferInit,
                   p_stMindSrc->p_stDsgMem->p_cDsgMemBufferInit,
                   inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->nominalBufferSize);
        }
    }
    intelligence = p_stMindDst->p_stIntelligence;
    if (intelligence != NULL) {
        intelligence->p_stCurrentComport = intelligence->initialBehavior;
        intelligence->p_stPrevComport = intelligence->initialBehavior;
    }
    intelligence = p_stMindDst->p_stReflex;
    if (intelligence != NULL) {
        intelligence->p_stCurrentComport = intelligence->initialBehavior;
        intelligence->p_stPrevComport = intelligence->initialBehavior;
    }
    memory = p_stMindDst->p_stDsgMem;
    if (memory != NULL) {
        if (memory->p_cDsgMemBufferInit == NULL) {
            memcpy(memory->p_cDsgMemBuffer, inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->defaultValues,
                   inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->nominalBufferSize);
        } else {
            memcpy(memory->p_cDsgMemBuffer, memory->p_cDsgMemBufferInit,
                   inv_AI_fn_p_stMindDsgDescription(p_stMindDst)->nominalBufferSize);
        }
    }
}
