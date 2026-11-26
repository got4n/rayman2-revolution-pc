#include "common.h"

#include "GAM/GAM.h"
#include "SAI/SAI.h"

SAI_tdstListEntry_* fn_hSearchAnElementInList(void* table, void* key, u32 flags);

s32 SAI_fn_bInitAllWhen(u16 _eUserFlags)
{
    return SAI_fn_bInitAllTableWhen(&SAI_g_stInternalStructure, _eUserFlags);
}

void* SAI_fn_p_vGetTableValuePointerOf(void* _p_vValue, u32 _eFlags)
{
    SAI_tdstListEntry_* hListEntry;
    void* p_vReturn;
    u32 verifiedFlags;

    verifiedFlags = SAI_fn_eVerifyFlags(_eFlags);
    p_vReturn = NULL;
    hListEntry = fn_hSearchAnElementInList(&SAI_g_stInternalStructure, _p_vValue, verifiedFlags);
    if (hListEntry != NULL) {
        switch (hListEntry->eFlags & 0xFC00) {
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x7000:
        case 0x8000:
            p_vReturn = &hListEntry->uData;
            break;
        case 0x5000:
            p_vReturn = hListEntry->uData.stArrayData.p_ucPointer;
            break;
        default:
            p_vReturn = NULL;
            break;
        }
    }
    return p_vReturn;
}

s32 SAI_fn_ucPlayerSaveValues(void)
{
    return 0x9D7C;
}

s32 SAI_fn_ucLoadPlayerValues(void)
{
    return 0x9D7C;
}
