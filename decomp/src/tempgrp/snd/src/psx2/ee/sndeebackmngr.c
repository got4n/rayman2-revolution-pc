#include "common.h"
#include "snd/snd.h"

#define BACK_DATA_COUNT 48

#define BACK_DATA_ALLOCATED 1

typedef struct tdstBACKMngr {
    s32 constructed : 1;
    s32 : 31;
    tdstBACKData data[BACK_DATA_COUNT];
} tdstBACKMngr;

typedef s32 (*BACKParseFn)(tdstBACKData* data, void* context);

static inline tdstBACKData* inv_SND_fn_p_stValidSlot(tdstBACKMngr* manager, tdstBACKData* slot)
{
    tdstBACKData* valid = NULL;

    if (slot >= &manager->data[0] && slot <= &manager->data[BACK_DATA_COUNT]) {
        u8* offset = (u8*) slot - sizeof(s32);

        if (((offset - (u8*) manager) & 0xF) == 0) {
            valid = slot;
        }
    }
    return valid;
}

s32 fn_iConstructTdstBACKMngr(tdstBACKMngr* manager)
{
    tdstBACKData* data;
    tdstBACKData* end = &manager->data[BACK_DATA_COUNT];

    for (data = &manager->data[0]; data < end; data++) {
        fn_iConstructTdstBACKData(data);
    }
    manager->constructed = 1;
    return 0;
}

s32 fn_iDestructTdstBACKMngr(tdstBACKMngr* manager)
{
    tdstBACKData* data;
    tdstBACKData* end;

    if (manager->constructed) {
        data = &manager->data[0];
        end = &manager->data[BACK_DATA_COUNT];
        while (data < end) {
            fn_iConstructTdstBACKData(data);
            data++;
        }
    }
    return 0;
}

tdstBACKData* fn_pstParseTdstBACKMngr(tdstBACKMngr* manager, void* context, BACKParseFn parse)
{
    tdstBACKData* match = NULL;
    tdstBACKData* data = &manager->data[0];

    while (data < &manager->data[BACK_DATA_COUNT]) {
        if (parse(data, context) != 0) {
            match = data;
            break;
        }
        data++;
    }
    return match;
}

s32 fn_iGetNumFreeVoiceCallbackTdstBACKMngr(tdstBACKData* data, s32* count)
{
    if (((data->flags & BACK_DATA_ALLOCATED) ^ BACK_DATA_ALLOCATED) != 0) {
        *count = *count + 1;
    }
    return 0;
}

s32 fn_iAllocateVoiceCallbackTdstBACKMngr(tdstBACKData* data)
{
    return (data->flags & BACK_DATA_ALLOCATED) ^ BACK_DATA_ALLOCATED;
}

s32 fn_iGetNumFreeVoiceTdstBACKMngr(tdstBACKMngr* manager)
{
    s32 count = 0;

    fn_pstParseTdstBACKMngr(manager, &count, (BACKParseFn) fn_iGetNumFreeVoiceCallbackTdstBACKMngr);
    return count;
}

u32 fn_u32AllocateVoiceTdstBACKMngr(tdstBACKMngr* manager, tdstBACKCall* source)
{
    u32 voice = 0xFFFFFFFF;
    tdstBACKData* slot = fn_pstParseTdstBACKMngr(manager, NULL, (BACKParseFn) fn_iAllocateVoiceCallbackTdstBACKMngr);

    if (slot != NULL) {
        voice = (u32) fn_u32AllocateTdstBACKData(slot, source);
    }
    return voice;
}

u32 fn_u32DesallocateVoiceTdstBACKMngr(tdstBACKMngr* manager, tdstBACKData* slot)
{
    tdstBACKData* valid = inv_SND_fn_p_stValidSlot(manager, slot);
    u32 result = 0xFFFFFFFF;

    if (valid != NULL) {
        result = fn_u32DesallocateTdstBACKData(valid);
    }
    return result;
}

s32 fn_iOperateCallTdstBACKMngr(tdstBACKMngr* manager, tdstBACKData* slot)
{
    tdstBACKData* valid = inv_SND_fn_p_stValidSlot(manager, slot);
    s32 result = -1;

    if (valid != NULL) {
        result = fn_iOperateCallTdstBACKData(valid);
    }
    return result;
}

tdstBACKData* func_001661C8(tdstBACKMngr* manager, tdstBACKData* slot)
{
    return inv_SND_fn_p_stValidSlot(manager, slot);
}
