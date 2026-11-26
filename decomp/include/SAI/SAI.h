#ifndef REVO_SAI_SAI_H
#define REVO_SAI_SAI_H

#include "ps2/types.h"

typedef struct tdstPointerData_ {
    void* p_vPointer;
    u16 uwBlocAndModuleID;
} tdstPointerData_;

typedef struct tdstArrayData_ {
    void* p_ucPointer;
    u32 ulArraySize;
} tdstArrayData_;

typedef struct tdstBooleanData_ {
    u8 ucBitPosition;
    u8 ucCharData;
} tdstBooleanData_;

typedef union SAI_tduData_ {
    u8 ucCharData;
    u16 uwShortData;
    u32 ulLongData;
    u64 i64Long64Data;
    tdstPointerData_ stPointerData;
    tdstArrayData_ stArrayData;
    tdstBooleanData_ stBooleanData;
} SAI_tduData_;

typedef struct SAI_tdstListEntry_ SAI_tdstListEntry_;

struct SAI_tdstListEntry_ {
    SAI_tdstListEntry_* p_stNextEntry;
    union {
        void* any;
        u8* byte;
        u16* halfword;
        u32* word;
        u64* doubleword;
    } p_vDataPointer;

    SAI_tduData_ uData;
    u16 eFlags;
    u8 ucMemoryLevel;
    u8 ucDummy;
};

typedef struct SAI_tdstInternalStructure_ {
    SAI_tdstListEntry_* p_stFirstEntry;
    SAI_tdstListEntry_* p_stLastEntry;
} SAI_tdstInternalStructure_;

extern SAI_tdstInternalStructure_ SAI_g_stInternalStructure;

struct SaveSlot;
extern s32 inv_SAV_fn_bFormatCardIfNeeded(void);
extern s32 inv_SAV_fn_lCheckFreeSpace(void);
extern u32 SAI_fn_eVerifyFlags(u16 eFlags);
extern s32 SAI_fn_bRestoreAnElementFromList(SAI_tdstListEntry_* hListEntry);
extern struct SaveSlot* inv_SAV_fn_p_stInsertSlotSortedByDate(struct SaveSlot* list,
                                                              struct SaveSlot* node);
extern s32 inv_fn_bDeleteSaveGame(struct SaveSlot* save);
extern u8 SAI_fn_bInitAllTableWhen(SAI_tdstInternalStructure_* p_stInternalStructure, u16 eUserFlags);
extern struct SaveSlot* func_00145878(struct SaveSlot* head);

extern s32 SAI_fn_bInitAllWhen(u16 _eUserFlags);
extern s32 SAI_fn_ucLoadPlayerValues(void);
extern s32 SAI_fn_ucPlayerSaveValues(void);

#endif
