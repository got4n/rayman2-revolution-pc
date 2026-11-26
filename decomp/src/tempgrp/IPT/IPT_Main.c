#include "common.h"

#include "IPT/IPT.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "AI/AI.h"
#include "GAM/GAM.h"
#include "INO/INO.h"
#include "ps2/PS2.h"

#define IPT_KEYWORD_JOYSTICK_BUTTON -4070
#define IPT_KEYWORD_ACTION_PARAMETER -4086

#define IPT_DEVICE_INACTIVE 1
#define IPT_DEVICE_PAD 3
#define IPT_DEVICE_JOYSTICK 4

#define IPT_FIRST_DEVICE 2
#define IPT_DEVICE_SLOTS 18
#define IPT_FIRST_BUTTON 10
#define IPT_BUTTON_END 26

typedef struct IPT_tdstKeyAndPadDefine_ {
    u16 uwBasedKey;
    char* p_szFrenchKey;
    char* p_szAmericanKey;
} IPT_tdstKeyAndPadDefine_;

typedef struct IptObjectView {
    u8 unk00[0x0C];
    void* unk0C;
} IptObjectView;

extern s32 inv_IPT_disabledDeviceMask;
extern s32 inv_IPT_ulInputFrameCounter;
extern stDevice* unk_330CA0[];
extern IPT_tdstKeyAndPadDefine_* inv_IPT_keyNameTable;

char* IPT_fn_p_szGetNameOfKeyName(u16 id, u8 kind);

void IPT_fn_vInitInput(s16 _sHistoricSize)
{
    s32 i;
    stDevice* device;

    func_0011DC30();
    IPT_g_hInputStructure.historicMode = 1;
    for (i = 0; i < 2; i++) {
        device = func_0011DC60(1, i, 2);
        if (device != NULL) {
            IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + i] = device;
            INO_fn_wActivateDevice(device);
            IPT_g_hInputStructure.deviceState[IPT_FIRST_DEVICE + i] = IPT_DEVICE_PAD;
            IPT_g_hInputStructure.historicMode = 2;
        }
    }
}

void IPT_fn_vResetInputEntry(void)
{
    u32 i;

    for (i = 0; i < IPT_g_hInputStructure.entryCount; i++) {
        IPT_g_hInputStructure.entries[i].lState = -2;
    }
}

char* IPT_fn_szGetEntryActionName(IPT_tdstEntryElement_* entry)
{
    char* result = NULL;

    if (entry != NULL) {
        result = entry->p_szActionName;
    }
    return result;
}

IPT_tdstEntryElement_* inv_IPT_fn_p_stGetEntry(u32 index)
{
    if (index < IPT_g_hInputStructure.entryCount) {
        return &IPT_g_hInputStructure.entries[index];
    }
    return NULL;
}

void IPT_fn_vActivateAllEntryElements(void)
{
    IPT_tdstEntryElement_* hEntryElement = IPT_g_hInputStructure.entries;
    u32 ulI;

    for (ulI = 0; ulI < IPT_g_hInputStructure.entryCount; ulI++) {
        IPT_fn_vActivateEntryElement(hEntryElement);
        hEntryElement++;
    }
}

void IPT_fn_vActivateEntryElement(IPT_tdstEntryElement_* _hEntryElement)
{
    if (_hEntryElement != NULL) {
        _hEntryElement->bIsActivate = 1;
        _hEntryElement->lState = -2;
    }
}

void IPT_fn_vDesactivateEntryElement(IPT_tdstEntryElement_* _hEntryElement)
{
    if (_hEntryElement != NULL) {
        _hEntryElement->bIsActivate = 0;
        _hEntryElement->lState = -2;
    }
}

void func_001203D0(IptObjectView* object, void* value)
{
    object->unk0C = value;
}

void IPT_fn_vDesactiveJoystickAndPadControl(void)
{
    u32 i;

    for (i = IPT_FIRST_DEVICE; i < IPT_DEVICE_SLOTS; i++) {
        if (IPT_g_hInputStructure.deviceState[i] == IPT_DEVICE_PAD ||
            IPT_g_hInputStructure.deviceState[i] == IPT_DEVICE_JOYSTICK) {
            IPT_g_hInputStructure.deviceState[i] = IPT_DEVICE_INACTIVE;
            INO_fn_wDeactivateDevice(IPT_g_hInputStructure.devices[i]);
        }
    }
}

void IPT_fn_vActiveJoystickControl(void)
{
    u32 i;

    for (i = IPT_FIRST_DEVICE; i < IPT_DEVICE_SLOTS; i++) {
        if (IPT_g_hInputStructure.deviceState[i] == IPT_DEVICE_INACTIVE) {
            IPT_g_hInputStructure.deviceState[i] = IPT_DEVICE_JOYSTICK;
            INO_fn_wActivateDevice(IPT_g_hInputStructure.devices[i]);
        }
    }
}

IPT_tdstEntryElement_* IPT_fn_hSearchElementInOptionWithKeyWord(s16 _swEntryType, s16 swPadNumber, s16 _swKeyValue)
{
    IPT_tdstEntryElement_* hEntryElement = NULL;
    IPT_tdstEntryElement_* hAction;
    u32 i;

    for (i = 0; i < IPT_g_hInputStructure.entryCount; i++) {
        hAction = &IPT_g_hInputStructure.entries[i];
        if (hAction->p_szActionName != NULL && _swEntryType == IPT_KEYWORD_JOYSTICK_BUTTON &&
            hAction->d_stKeyWordElementArray[0].value == _swEntryType &&
            hAction->d_stKeyWordElementArray[1].value == swPadNumber &&
            hAction->d_stKeyWordElementArray[2].value == _swKeyValue) {
            hEntryElement = hAction;
        }
    }
    return hEntryElement;
}

u8 IPT_fn_bModifyIPTAction(IPT_tdstEntryElement_* entry, IPT_tdstEntryElement_* _hReplacingElement)
{
    s32 lI;

    for (lI = 0; lI < entry->ulNumberOfKeyWordElement; lI++) {
        if (entry->d_stKeyWordElementArray[lI].value == IPT_KEYWORD_ACTION_PARAMETER) {
            entry->d_stKeyWordElementArray[lI + 1].hEntryElement = _hReplacingElement;
            return 1;
        }
    }
    return 0;
}

u8 IPT_fn_bChangeIPTValue(IPT_tdstEntryElement_* _hElement, u8 ucWhichTest)
{
    tdu_KeyWordElementUnion_* keyword = _hElement->d_stKeyWordElementArray;
    s32 changed = 0;
    s32 accepted = 0;
    s16 deviceIndex;
    stDevice* device;
    u32* mask;
    u32 button;
    IPT_tdstEntryElement_* other;

    if (keyword->value == IPT_KEYWORD_JOYSTICK_BUTTON) {
        keyword++;
        deviceIndex = keyword->value;
        keyword++;
        device = IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex];
        mask = IPT_g_ulPadAndJoyButtonFlag;
        button = IPT_FIRST_BUTTON;
        do {
            if ((device->m_p_stState->m_ulButtons & *mask) == *mask &&
                (INO_fn_ulJoyPreviousButtons(device) & *mask) != *mask) {
                break;
            }
            button++;
            mask++;
        } while (button < IPT_BUTTON_END);
        if (button < IPT_BUTTON_END) {
            other = IPT_fn_hSearchElementInOptionWithKeyWord(
                IPT_KEYWORD_JOYSTICK_BUTTON, deviceIndex, inv_IPT_a_wButtonKeyWordValues[button]);
            if (ucWhichTest == 0) {
                accepted = 1;
            } else if (ucWhichTest == 2) {
                if (other == NULL || other == _hElement) {
                    accepted = 1;
                }
            } else if (ucWhichTest == 1) {
                accepted = 1;
                if (other != NULL && other != _hElement) {
                    other->d_stKeyWordElementArray[2].value = keyword->value;
                }
            }
            if (accepted) {
                changed = 1;
                keyword->value = inv_IPT_a_wButtonKeyWordValues[button];
            }
        }
    }
    return changed;
}

char* IPT_fn_szGetStringOf(IPT_tdstEntryElement_* _hElement)
{
    tdu_KeyWordElementUnion_* keywords = _hElement->d_stKeyWordElementArray;

    if (keywords[0].value == IPT_KEYWORD_JOYSTICK_BUTTON) {
        return IPT_fn_p_szGetNameOfKeyName(keywords[2].code | 0x8000, 0);
    }
    return NULL;
}

void* func_00120778(void)
{
    return AI_fn_p_vTrueAlloc(gp_6F74, 0x10);
}

char* IPT_fn_p_szGetNameOfKeyName(u16 id, u8 kind)
{
    IPT_tdstKeyAndPadDefine_* table = inv_IPT_keyNameTable;
    IPT_tdstKeyAndPadDefine_* record;
    char* result = NULL;
    s32 i = 0;

    if (table[0].uwBasedKey != 0xFFFF && id != table[0].uwBasedKey) {
        do {
            i++;
        } while (table[i].uwBasedKey != 0xFFFF && id != table[i].uwBasedKey);
    }
    record = &table[i];
    if (record->uwBasedKey != 0xFFFF) {
        if (kind == 0) {
            result = record->p_szFrenchKey;
        } else if (kind == 1) {
            result = record->p_szAmericanKey;
        }
    }
    return result;
}

#ifdef PC_PORT
static u8 IPT_fn_bAButtonPadJustPressed_game(void);

u8 IPT_fn_bAButtonPadJustPressed(void)
{
    if (pc_mod_any_button_now()) {
        return 1;
    }
    return IPT_fn_bAButtonPadJustPressed_game();
}

#define IPT_fn_bAButtonPadJustPressed IPT_fn_bAButtonPadJustPressed_game
#endif
u8 IPT_fn_bAButtonPadJustPressed(void)
{
    stDevice* device;
    u32 pressed;
    s32 j;
    s32 i;

    if (IPT_g_hInputStructure.historicMode == 2) {
        for (j = IPT_FIRST_DEVICE; j < IPT_DEVICE_SLOTS; j++) {
            device = IPT_g_hInputStructure.devices[j];
            for (i = IPT_FIRST_BUTTON; i < IPT_BUTTON_END; i++) {
                pressed = device->m_p_stState->m_ulButtons &
                          IPT_g_ulPadAndJoyButtonFlag[i - IPT_FIRST_BUTTON];
                if (pressed == IPT_g_ulPadAndJoyButtonFlag[i - IPT_FIRST_BUTTON] &&
                    (INO_fn_ulJoyPreviousButtons(device) & pressed) != pressed) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
#ifdef PC_PORT
#undef IPT_fn_bAButtonPadJustPressed
#endif

IPT_tdstEntryElement_* inv_IPT_fn_hGetJoystickAxeEntry(s16 entry, s16 field)
{
    IPT_tdstEntryElement_** record = inv_IPT_a_pEntryElementRecords[entry];

    switch (field) {
    case 0:
        return record[0];
    case 1:
        return record[1];
    case 2:
        return record[2];
    case 3:
        return record[3];
    default:
        return 0;
    }
}

s32 func_00120970(s16 value)
{
    return value;
}

s32 inv_IPT_fn_bIsPadConnected(s32 deviceIndex)
{
    return IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex]->m_p_stState->m_ucStatus &
           1;
}

s32 func_001209A8(void)
{
    stDevice** device = unk_330CA0;
    s32 remaining = 1;

    do {
        remaining--;
        if (*device != NULL && ((*device)->m_p_stState->m_ucStatus & 1)) {
            return 1;
        }
        device++;
    } while (remaining >= 0);
    return 0;
}

s32 IPT_fn_bIsJoystickButtonPressed(s32 deviceIndex, s32 buttonMask)
{
    stDevice* device = IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex];

    return (~device->m_p_stState->m_ulButtons & buttonMask) == 0;
}

s32 IPT_fn_bIsJoystickButtonJustPressed(u32 deviceIndex, u32 buttonMask)
{
    stDevice* device = IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex];
    u32 pressed = device->m_p_stState->m_ulButtons & buttonMask;
    u32 previous;
    s32 result = 0;

    if (pressed == buttonMask) {
        previous = INO_fn_ulJoyPreviousButtons(device) & pressed;
        result = previous != pressed;
    }
    return result;
}

s32 inv_IPT_fn_lGetPadWithButton8Released(void)
{
    stDevice* device;
    s32 i;

    for (i = 1; i >= 0; i--) {
        if (((inv_IPT_disabledDeviceMask >> i) & 1) == 0) {
            device = IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + i];
            if ((device->m_p_stState->m_ulButtons & 8) == 0 &&
                (INO_fn_ulJoyPreviousButtons(device) & 8) != 0) {
                return i;
            }
        }
    }
    return -1;
}

s32 inv_IPT_fn_lGetPadWithButton8Pressed(void)
{
    s32 i;

    for (i = 1; i >= 0; i--) {
        if (((inv_IPT_disabledDeviceMask >> i) & 1) == 0 &&
            IPT_fn_bIsJoystickButtonPressed(i, 8) != 0) {
            return i;
        }
    }
    return -1;
}

s32 inv_IPT_fn_bActivateJoystick(s32 deviceIndex)
{
    return INO_fn_wActivateDevice(IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex]) ==
           1;
}

s32 IPT_fn_vDesactiveJoystickControl(s32 deviceIndex)
{
    return INO_fn_wDeactivateDevice(
               IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex]) == 1;
}

void inv_IPT_fn_bSwapJoystickDevices(s32 first, s32 second)
{
    s32 firstSlot = IPT_FIRST_DEVICE + first;
    s32 secondSlot = IPT_FIRST_DEVICE + second;
    stDevice* device;
    u8 state;

    device = IPT_g_hInputStructure.devices[firstSlot];
    IPT_g_hInputStructure.devices[firstSlot] = IPT_g_hInputStructure.devices[secondSlot];
    IPT_g_hInputStructure.devices[secondSlot] = device;
    state = IPT_g_hInputStructure.deviceState[firstSlot];
    IPT_g_hInputStructure.deviceState[firstSlot] = IPT_g_hInputStructure.deviceState[secondSlot];
    IPT_g_hInputStructure.deviceState[secondSlot] = state;
    inv_IPT_disabledDeviceMask |= 1 << first;
    IPT_fn_vDesactiveJoystickControl(first);
    inv_IPT_fn_bActivateJoystick(first);
}

void inv_IPT_fn_vEnableAllPads(void)
{
    inv_IPT_disabledDeviceMask = 0;
}

u32 inv_IPT_fn_ulGetInputFrameCounter(void)
{
    return inv_IPT_ulInputFrameCounter;
}

void inv_IPT_fn_vIncrementInputFrameCounter(void)
{
    inv_IPT_ulInputFrameCounter++;
}

void inv_IPT_fn_vInitInputFrameState(void)
{
    inv_IPT_ulInputFrameCounter = 1;
    inv_IPT_disabledDeviceMask = 1;
}

void func_00120C98(s32 deviceIndex)
{
    loc_11E218(IPT_g_hInputStructure.devices[IPT_FIRST_DEVICE + deviceIndex]);
}

s32 IPT_fn_ucGetFirstValidJoystick(void)
{
    s32 result = -1;
    s32 i;
    u32 mask;

    for (i = inv_IPT_ulInputFrameCounter - 1; i >= 0; i--) {
        mask = 1 << i;
        if (inv_IPT_fn_bIsPadConnected(i) == 0) {
            inv_IPT_disabledDeviceMask &= ~mask;
            func_00120C98(i);
        }
        if ((inv_IPT_disabledDeviceMask & mask) == 0) {
            result = i;
        }
    }
    return result;
}
