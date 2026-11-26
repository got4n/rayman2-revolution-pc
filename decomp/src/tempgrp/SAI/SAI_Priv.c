#include "common.h"

#include "SAI/SAI.h"
#include "SVG/SVG.h"

#include "ps2/libc.h"
#include "GAM/GAM.h"

#define SAI_ELEMENT_ID_MASK 0x3FF
#define SAI_ELEMENT_KIND_MASK 0xFC00
#define SAI_ELEMENT_BYTE 0x1000
#define SAI_ELEMENT_HALFWORD 0x2000
#define SAI_ELEMENT_WORD 0x3000
#define SAI_ELEMENT_DOUBLEWORD 0x4000
#define SAI_ELEMENT_BLOCK 0x5000
#define SAI_ELEMENT_WORD_ALT 0x7000
#define SAI_ELEMENT_BITS 0x8000

s32 strcasecmp(const char* left, const char* right);

s32 inv_fn_bDeleteSaveGame(SaveSlot* save)
{
    char directory[0x100];
    s32 result;

    j_sprintf_SaveDirectoryName(directory, save->info.name);
    result = SVG_fn_s32DestroySaveGame(0, 0, directory);
    return (u32) result > 0;
}

SaveSlot* func_00145878(SaveSlot* head)
{
    s16 n = 0;
    SaveSlot* cur = head;
    SaveSlot* previous = NULL;
    SaveSlot* current;

    if (inv_SAV_stCurrentSlot->info.valid) {
        do {
            if (cur == NULL) {
                goto number;
            }
            current = inv_SAV_stCurrentSlot;
            if (cur->info.modificationTime == current->info.modificationTime &&
                strcasecmp(cur->info.comment, current->info.comment) == 0 &&
                strcasecmp(cur->info.playerName, current->info.playerName) == 0 &&
                cur->info.slotIndex == current->info.slotIndex) {
                if (cur != head) {
                    previous->next = cur->next;
                    cur->next = head;
                    head = cur;
                }
                current->id = 0;
                break;
            }
            previous = cur;
            cur = cur->next;
        } while (1);
    }
number:
    for (cur = head; cur != NULL; cur = cur->next) {
        cur->id = n++;
    }
    return head;
}

SaveSlot* inv_SAV_fn_p_stInsertSlotSortedByDate(SaveSlot* list, SaveSlot* node)
{
    SaveSlot* cur = list;
    SaveSlot* prev = NULL;
    u32 first;
    u32 key;

    if (list == NULL) {
        return node;
    }
    first = cur->info.modificationTime;
    key = node->info.modificationTime;
    if (first > key) {
        do {
            prev = cur;
            cur = prev->next;
        } while (cur != NULL && cur->info.modificationTime > key);
    }
    node->next = cur;
    if (prev != NULL) {
        prev->next = node;
        return list;
    }
    return node;
}

s32 inv_SAV_fn_bFormatCardIfNeeded(void)
{
    if (inv_SAV_fn_lIsUnformattedDeviceReady() != -1) {
        return (u32) SVG_fn_s32Format(0, 0) > 0;
    }
    return 0;
}

s32 inv_SAV_fn_lCheckFreeSpace(void)
{
    return func_00140840(0, 0, 0x9D7C);
}

u32 SAI_fn_eVerifyFlags(u16 eFlags)
{
    eFlags &= SAI_ELEMENT_ID_MASK;
    if ((eFlags & 0xC0) != 0) {
        eFlags |= 4;
    }
    if ((eFlags & 0x300) != 0) {
        eFlags |= 8;
    }
    return eFlags;
}

SAI_tdstListEntry_* fn_hSearchAnElementInList(SAI_tdstInternalStructure_* p_stInternalStructure, void* p_vData, u16 eUserFlags)
{
    SAI_tdstListEntry_* hListEntry;

    for (hListEntry = p_stInternalStructure->p_stFirstEntry; hListEntry != NULL; hListEntry = hListEntry->p_stNextEntry) {
        if (hListEntry->p_vDataPointer.any == p_vData && (hListEntry->eFlags & SAI_ELEMENT_ID_MASK) == eUserFlags) {
            return hListEntry;
        }
    }
    return NULL;
}

s32 SAI_fn_bRestoreAnElementFromList(SAI_tdstListEntry_* hListEntry)
{
    s32 bReturn = 1;

    switch (hListEntry->eFlags & SAI_ELEMENT_KIND_MASK) {
    case SAI_ELEMENT_BITS:
        *hListEntry->p_vDataPointer.byte =
            *hListEntry->p_vDataPointer.byte & (u8) ~hListEntry->uData.stBooleanData.ucCharData;
        *hListEntry->p_vDataPointer.byte =
            *hListEntry->p_vDataPointer.byte | hListEntry->uData.stBooleanData.ucCharData;
        break;
    case SAI_ELEMENT_BYTE:
        *hListEntry->p_vDataPointer.byte = hListEntry->uData.ucCharData;
        break;
    case SAI_ELEMENT_HALFWORD:
        *hListEntry->p_vDataPointer.halfword = hListEntry->uData.uwShortData;
        break;
    case SAI_ELEMENT_DOUBLEWORD:
        *hListEntry->p_vDataPointer.doubleword = hListEntry->uData.i64Long64Data;
        break;
    case SAI_ELEMENT_WORD_ALT:
        *hListEntry->p_vDataPointer.word = hListEntry->uData.ulLongData;
        break;
    case SAI_ELEMENT_WORD:
        *hListEntry->p_vDataPointer.word = hListEntry->uData.ulLongData;
        break;
    case SAI_ELEMENT_BLOCK:
        memcpy(hListEntry->p_vDataPointer.any, hListEntry->uData.stArrayData.p_ucPointer,
               hListEntry->uData.stArrayData.ulArraySize);
        break;
    default:
        bReturn = 0;
        break;
    }
    return bReturn;
}

u8 SAI_fn_bInitAllTableWhen(SAI_tdstInternalStructure_* p_stInternalStructure, u16 eUserFlags)
{
    SAI_tdstListEntry_* hListEntry;

    for (hListEntry = p_stInternalStructure->p_stFirstEntry; hListEntry != NULL; hListEntry = hListEntry->p_stNextEntry) {
        if ((hListEntry->eFlags & eUserFlags) == eUserFlags) {
            SAI_fn_bRestoreAnElementFromList(hListEntry);
        }
    }
    return 1;
}
