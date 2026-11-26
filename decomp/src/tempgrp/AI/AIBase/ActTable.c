#include "common.h"

#include "AI/AI.h"

#include "ps2/libc.h"

static inline tdstActionTableEntry_* inv_AI_fn_p_stEntryAt(tdstIntelligence_* intelligence, u8 index)
{
    return (tdstActionTableEntry_*) (index * sizeof(tdstActionTableEntry_) +
                                     (u32) intelligence->p_stActionTable->p_stEntry);
}

s32 fn_ucIsRuleInActionTable(tdstIntelligence_* p_stIntelligence, u8 ucNumRule)
{
    tdstActionTable_* table = p_stIntelligence->p_stActionTable;
    u32 count = table->ucNbEntries;
    tdstActionTableEntry_* entry;
    u8 index;

    if (table->ucNbEntriesUsed != 0) {
        entry = table->p_stEntry;
        for (index = 0; index < count; index++, entry++) {
            if (entry->bUsed && entry->ucNumRule == ucNumRule) {
                return index;
            }
        }
    }
    return count;
}

u8 fn_ucIsRuleAndNodeInActionTable(tdstIntelligence_* p_stIntelligence, u8 ucNumRule, tdstNodeInterpret_* p_stNode)
{
    tdstActionTable_* table = p_stIntelligence->p_stActionTable;
    s32 count = table->ucNbEntries;
    tdstActionTableEntry_* entry;
    s32 index;

    if (table->ucNbEntriesUsed != 0) {
        entry = table->p_stEntry;
        for (index = 0; index < count; index++, entry++) {
            if (entry->bUsed && entry->ucNumRule == ucNumRule && entry->p_stNode == p_stNode) {
                return (u8) index;
            }
        }
    }
    return count;
}

u8 fn_ucFindPlaceInActionTable(tdstIntelligence_* p_stIntelligence)
{
    tdstActionTable_* table = p_stIntelligence->p_stActionTable;
    s32 count = table->ucNbEntries;
    tdstActionTableEntry_* entry;
    s32 index;

    if (table->ucNbEntriesUsed < count) {
        entry = table->p_stEntry;
        for (index = 0; index < count; index++, entry++) {
            if (!entry->bUsed) {
                return (u8) index;
            }
        }
    }
    return count;
}

void fn_vInitActionTableEntry(tdstIntelligence_* intelligence, u8 index, s32 rule,
                              tdstNodeInterpret_* node, s32 inUse)
{
    inv_AI_fn_p_stEntryAt(intelligence, index)->bUseDefaultActionReturn = g_ucUseDefaultActionReturn;
    inv_AI_fn_p_stEntryAt(intelligence, index)->ucNewActionReturn = g_ucNewActionReturn;
    inv_AI_fn_p_stEntryAt(intelligence, index)->bUsed = inUse;
    inv_AI_fn_p_stEntryAt(intelligence, index)->p_stNode = node;
    inv_AI_fn_p_stEntryAt(intelligence, index)->ucNumRule = rule;
    memset(&inv_AI_fn_p_stEntryAt(intelligence, index)->stActionParam, 0, sizeof(tdstActionParam_));
}

void fn_vUseCurrentActionTableEntry(tdstIntelligence_* p_stIntelligence, u8 ucNRule,
                                    tdstNodeInterpret_* p_stNode)
{
    u8 ucCurrentEntry = p_stIntelligence->p_stActionTable->ucCurrentEntry;

    if (!inv_AI_fn_p_stEntryAt(p_stIntelligence, ucCurrentEntry)->bUsed) {
        fn_vInitActionTableEntry(p_stIntelligence, ucCurrentEntry, ucNRule, p_stNode, 1);
        p_stIntelligence->p_stActionTable->ucNbEntriesUsed++;
    }
}

void fn_vUnuseCurrentActionTableEntry(tdstIntelligence_* p_stIntelligence)
{
    u8 ucCurrentEntry = p_stIntelligence->p_stActionTable->ucCurrentEntry;

    if (inv_AI_fn_p_stEntryAt(p_stIntelligence, ucCurrentEntry)->bUsed) {
        fn_vInitActionTableEntry(p_stIntelligence, ucCurrentEntry, 0, NULL, 0);
        p_stIntelligence->p_stActionTable->ucNbEntriesUsed--;
    }
}

void fn_vUnuseAllRulesWhichHaveGreaterNumRule(tdstIntelligence_* p_stIntelligence, u8 ucNumRule)
{
    tdstActionTable_* table = p_stIntelligence->p_stActionTable;
    tdstActionTableEntry_* entry;
    s32 count;
    s32 index;

    if (table->ucNbEntriesUsed == 0) {
        return;
    }
    count = table->ucNbEntries;
    entry = table->p_stEntry;
    for (index = 0; index < count; index++, entry++) {
        if (entry->bUsed && ucNumRule < entry->ucNumRule) {
            fn_vInitActionTableEntry(p_stIntelligence, index, 0, NULL, 0);
            p_stIntelligence->p_stActionTable->ucNbEntriesUsed--;
        }
    }
}

void fn_vUnuseAllRulesFromSchedule(tdstIntelligence_* p_stIntelligence)
{
    tdstActionTable_* table = p_stIntelligence->p_stActionTable;
    tdstActionTableEntry_* entry;
    u8 count;
    s32 index;

    if (table->ucNbEntriesUsed == 0) {
        return;
    }
    count = table->ucNbEntries;
    entry = table->p_stEntry;
    for (index = 0; index < count; index++, entry++) {
        if (entry->bUsed && entry->ucNumRule == 0) {
            fn_vInitActionTableEntry(p_stIntelligence, index, 0, NULL, 0);
            count--;
        }
    }
    p_stIntelligence->p_stActionTable->ucNbEntriesUsed = count;
}

void fn_vInitActionTable(tdstIntelligence_* p_stIntelligence)
{
    s32 count;
    s32 index;

    p_stIntelligence->p_stActionTable->ucNbEntriesUsed = 0;
    p_stIntelligence->p_stActionTable->ucCurrentEntry = 0;
    count = p_stIntelligence->p_stActionTable->ucNbEntries;
    for (index = 0; index < count; index++) {
        fn_vInitActionTableEntry(p_stIntelligence, index, 0, NULL, 0);
    }
}

tdstNodeInterpret_* fn_p_stGetTableAction(tdstIntelligence_* p_stIntelligence, u8 ucNEntry)
{
    g_ucUseDefaultActionReturn = inv_AI_fn_p_stEntryAt(p_stIntelligence, ucNEntry)->bUseDefaultActionReturn;
    g_ucNewActionReturn = inv_AI_fn_p_stEntryAt(p_stIntelligence, ucNEntry)->ucNewActionReturn;
    return inv_AI_fn_p_stEntryAt(p_stIntelligence, ucNEntry)->p_stNode;
}
