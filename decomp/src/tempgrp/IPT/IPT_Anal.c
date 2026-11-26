#include "common.h"

#include "IPT/IPT.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "INO/INO.h"

void IPT_fn_vReadInput(void)
{
    IPT_tdstEntryElement_* entry;
    s32 valid;
    u32 i;

    INO_fn_wReadAllDevices();
    if (IPT_g_hInputStructure.historicMode == 2) {
        INO_fn_wReadJoystick(1, 2);
    }
    IPT_fn_vActualizeHistoric();
    for (i = 0; i < IPT_g_hInputStructure.entryCount; i++) {
        entry = &IPT_g_hInputStructure.entries[i];
        if (entry->bIsActivate != 1) {
            continue;
        }
        valid = IPT_fn_bIsEntryActionValid(entry);
        if (valid == 1 && IPT_fn_bIsValidated(entry)) {
            s32 state = entry->lState + 1;

            entry->lState = state < 1001 ? state : 1000;
        } else if (valid == 0 && IPT_fn_bIsInvalidated(entry)) {
            s32 state = entry->lState - 1;

            entry->lState = state > -1001 ? state : -1000;
        } else if (valid == 0 && IPT_fn_bIsValidated(entry)) {
            entry->lState = -1;
        } else if (valid == 1 && IPT_fn_bIsInvalidated(entry)) {
            entry->lState = 1;
        }
    }
}

#ifdef PC_PORT
static u8 IPT_fn_bIsJustValidated_game(IPT_tdstEntryElement_* _hEntryElement);

u8 IPT_fn_bIsJustValidated(IPT_tdstEntryElement_* _hEntryElement)
{
    if (pc_mod_entry_now((PcAddress) _hEntryElement)) {
        return 1;
    }
    return IPT_fn_bIsJustValidated_game(_hEntryElement);
}

#define IPT_fn_bIsJustValidated IPT_fn_bIsJustValidated_game
#endif
u8 IPT_fn_bIsJustValidated(IPT_tdstEntryElement_* _hEntryElement)
{
    s32 result = 0;

    if (_hEntryElement != NULL) {
        result = _hEntryElement->lState == 1;
    }
    return result;
}
#ifdef PC_PORT
#undef IPT_fn_bIsJustValidated
#endif

u8 IPT_fn_bIsValidated(IPT_tdstEntryElement_* _hEntryElement)
{
    s32 result = 0;

    if (_hEntryElement != NULL) {
        result = _hEntryElement->lState > 0;
    }
    return result;
}

u8 IPT_fn_bIsJustInvalidated(IPT_tdstEntryElement_* _hEntryElement)
{
    s32 result = 0;

    if (_hEntryElement != NULL) {
        result = _hEntryElement->lState == -1;
    }
    return result;
}

u8 IPT_fn_bIsInvalidated(IPT_tdstEntryElement_* _hEntryElement)
{
    s32 invalidated = 0;

    if (_hEntryElement == NULL || _hEntryElement->lState < 0) {
        invalidated = 1;
    }
    return invalidated;
}

f32 IPT_fn_xGetAnalogicValue(IPT_tdstEntryElement_* _hEntryElement)
{
    f32 result = 0.0f;

    if (_hEntryElement != NULL) {
        result = _hEntryElement->xAnalogicValue;
    }
    return result;
}
