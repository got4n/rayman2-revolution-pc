#include "common.h"

#include "IPT/IPT.h"

#include "MTH/MTH.h"
#include "ps2/PS2.h"

#define IPT_DEVICE_JOYSTICK 4
#define IPT_FIRST_BUTTON 0x1A
#define IPT_BUTTON_COUNT 0x10

typedef s32 (*KeywordTest)(tdu_KeyWordElementUnion_* keyword);

extern IPT_tdstEntryElement_* g_h_ActualEntryInput;
extern tdu_KeyWordElementUnion_* IPT_g_hNextParameter;
extern KeywordTest inv_IPT_keywordDispatchTable[];
extern u32 IPT_g_ulSequenceAnalyseNumber;
extern f32 IPT_g_fNormCorrectionTable[];

s32 inv_IPT_fn_bIsKeyWordValid(tdu_KeyWordElementUnion_* keyword);
f32 atan2f(f32 y, f32 x);
f32 fabsf(f32 value);
f32 sqrtf(f32 value);

s32 IPT_fn_bIsEntryActionValid(IPT_tdstEntryElement_* _hEntryElement)
{
    tdu_KeyWordElementUnion_* keywords = _hEntryElement->d_stKeyWordElementArray;
    s32 valid = 0;

    g_h_ActualEntryInput = _hEntryElement;
    _hEntryElement->xAnalogicValue = 0.0f;
    IPT_g_hNextParameter = keywords;
    if (keywords != NULL) {
        valid = inv_IPT_fn_bIsKeyWordValid(keywords);
    }
    return valid;
}

s32 inv_IPT_fn_bIsKeyWordValid(tdu_KeyWordElementUnion_* keyword)
{
    return inv_IPT_keywordDispatchTable[keyword->type](keyword);
}

u8 IPT_fn_bIsKeyWordValid_And(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 left;
    s32 right;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    left = inv_IPT_fn_bIsKeyWordValid(_hKeyWordElement + 1);
    right = inv_IPT_fn_bIsKeyWordValid(IPT_g_hNextParameter);
    right = right == 1;
    if (left != 1) {
        right = 0;
    }
    return right;
}

u8 IPT_fn_bIsKeyWordValid_Or(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 left;
    s32 right;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    left = inv_IPT_fn_bIsKeyWordValid(_hKeyWordElement + 1);
    right = inv_IPT_fn_bIsKeyWordValid(IPT_g_hNextParameter);
    return (left == 1) | (right == 1);
}

u8 IPT_fn_bIsKeyWordValid_Not(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    IPT_g_hNextParameter = _hKeyWordElement + 1;
    return inv_IPT_fn_bIsKeyWordValid(_hKeyWordElement + 1) == 0;
}

s32 func_0011F408(void)
{
    return 0;
}

u8 IPT_fn_bIsKeyWordValid_JoystickAxeValue(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    stDevice* hPad;
    s32 lMaxValue;
    s32 lMinValue;
    s32 lower;
    s32 upper;
    s32 value;
    u8 ucReturn = 0;
    f32 position = 0.0f;
    f32 percent;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    lMaxValue = _hKeyWordElement[1].value + 2;
    hPad = IPT_g_hInputStructure.devices[lMaxValue];
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    lMinValue = _hKeyWordElement[2].value;
    IPT_g_hNextParameter = &_hKeyWordElement[3];
    lower = _hKeyWordElement[3].value;
    IPT_g_hNextParameter = &_hKeyWordElement[4];
    upper = _hKeyWordElement[4].value;
    if (IPT_g_hInputStructure.deviceState[lMaxValue] == IPT_DEVICE_JOYSTICK) {
        switch (lMinValue) {
        case 4:
            position = hPad->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            position = hPad->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            position = hPad->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            position = hPad->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            position = hPad->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            position = hPad->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        percent = position * 100.0f;
        value = inv_MTH_fn_lToIntegerOrdered(percent);
        ucReturn = value >= lower && value <= upper;
        if (ucReturn == 1) {
            g_h_ActualEntryInput->xAnalogicValue = percent;
        }
    }
    IPT_g_hNextParameter++;
    return ucReturn;
}

u8 IPT_fn_bIsKeyWordValid_JoystickAngularValue(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    stDevice* device;
    s32 joystick;
    s32 axisX;
    s32 axisY;
    s32 lower;
    s32 upper;
    s32 value;
    u8 result = 0;
    f32 fFirstPosition = 0.0f;
    f32 fSecondPosition = 0.0f;
    f32 angle;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    device = IPT_g_hInputStructure.devices[joystick];
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    axisX = _hKeyWordElement[2].value;
    IPT_g_hNextParameter = &_hKeyWordElement[3];
    axisY = _hKeyWordElement[3].value;
    IPT_g_hNextParameter = &_hKeyWordElement[4];
    lower = _hKeyWordElement[4].value;
    IPT_g_hNextParameter = &_hKeyWordElement[5];
    upper = _hKeyWordElement[5].value;
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        switch (axisX) {
        case 4:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        switch (axisY) {
        case 4:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        if (-0.001f < fSecondPosition && fSecondPosition < 0.001f && -0.001f < fFirstPosition && fFirstPosition < 0.001f) {
            angle = 0.0f;
        } else {
            angle = atan2f(fSecondPosition, fFirstPosition) * MTH_RAD_TO_DEG_inv;
        }
        value = inv_MTH_fn_lToIntegerOrdered(angle);
        result = value >= lower && value <= upper;
        if (result == 1) {
            g_h_ActualEntryInput->xAnalogicValue = angle;
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

u8 IPT_fn_bIsKeyWordValid_JoystickTrueNormValue(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    stDevice* device;
    s32 joystick;
    s32 axisX;
    s32 axisY;
    s32 lower;
    s32 upper;
    s32 value;
    u8 result = 0;
    f32 fFirstPosition = 0.0f;
    f32 fSecondPosition = 0.0f;
    f32 norm;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    device = IPT_g_hInputStructure.devices[joystick];
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    axisX = _hKeyWordElement[2].value;
    IPT_g_hNextParameter = &_hKeyWordElement[3];
    axisY = _hKeyWordElement[3].value;
    IPT_g_hNextParameter = &_hKeyWordElement[4];
    lower = _hKeyWordElement[4].value;
    IPT_g_hNextParameter = &_hKeyWordElement[5];
    upper = _hKeyWordElement[5].value;
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        switch (axisX) {
        case 4:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        switch (axisY) {
        case 4:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        norm = sqrtf(fFirstPosition * fFirstPosition + fSecondPosition * fSecondPosition) * 100.0f;
        value = inv_MTH_fn_lToIntegerOrdered(norm);
        result = value >= lower && value <= upper;
        if (result == 1) {
            g_h_ActualEntryInput->xAnalogicValue = norm;
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

u8 IPT_fn_bIsKeyWordValid_JoystickCorrectedNormValue(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    stDevice* device;
    s32 joystick;
    s32 axisX;
    s32 axisY;
    s32 lower;
    s32 upper;
    s32 value;
    u8 result = 0;
    f32 fFirstPosition = 0.0f;
    f32 fSecondPosition = 0.0f;
    f32 norm;
    f32 angle;
    s32 sector;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    device = IPT_g_hInputStructure.devices[joystick];
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    axisX = _hKeyWordElement[2].value;
    IPT_g_hNextParameter = &_hKeyWordElement[3];
    axisY = _hKeyWordElement[3].value;
    IPT_g_hNextParameter = &_hKeyWordElement[4];
    lower = _hKeyWordElement[4].value;
    IPT_g_hNextParameter = &_hKeyWordElement[5];
    upper = _hKeyWordElement[5].value;
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        switch (axisX) {
        case 4:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            fFirstPosition = device->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        switch (axisY) {
        case 4:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[0];
            break;
        case 5:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[1];
            break;
        case 6:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[2];
            break;
        case 7:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[3];
            break;
        case 8:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[4];
            break;
        case 9:
            fSecondPosition = device->m_p_stState->m_a_fAxisValue[5];
            break;
        }
        norm = sqrtf(fFirstPosition * fFirstPosition + fSecondPosition * fSecondPosition) * 100.0f;
        angle = fabsf(atan2f(fabsf(fSecondPosition), fabsf(fFirstPosition)) - MTH_QUARTER_PI_inv);
        sector = inv_MTH_fn_lToIntegerOrdered((MTH_QUARTER_PI_inv - angle) * MTH_RAD_TO_DEG_inv);
        norm = norm / IPT_g_fNormCorrectionTable[sector];
        norm = ((norm < 0.0f ? 0.0f : norm) > 100.0f) ? 100.0f : (norm < 0.0f ? 0.0f : norm);
        value = inv_MTH_fn_lToIntegerOrdered(norm);
        result = value >= lower && value <= upper;
        if (result == 1) {
            g_h_ActualEntryInput->xAnalogicValue = norm;
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

static inline s32 inv_IPT_fn_lJustPressed(stDevice* device, s32 code)
{
    u32 mask = IPT_g_ulPadAndJoyButtonFlag[code - IPT_FIRST_BUTTON];
    s32 pressed = 0;

    if ((device->m_p_stState->m_ulButtons & mask) == mask) {
        u32 previous = INO_fn_ulJoyPreviousButtons(device);
        u32 previousMask =
            IPT_g_ulPadAndJoyButtonFlag[IPT_g_hNextParameter->value - IPT_FIRST_BUTTON];

        previous = (previous & previousMask) ^ previousMask;
        pressed = previous != 0;
    }
    return pressed;
}

s32 IPT_fn_bIsKeyWordValid_JoystickJustPressed(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 result = 0;
    s32 joystick;
    u16 code;
    stDevice* device;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    device = IPT_g_hInputStructure.devices[joystick];
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        code = _hKeyWordElement[2].code;
        if (code >= IPT_FIRST_BUTTON && code < IPT_FIRST_BUTTON + IPT_BUTTON_COUNT) {
            result = inv_IPT_fn_lJustPressed(device, (s16) code);
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

static inline s32 inv_IPT_fn_lJustReleased(stDevice* device, s32 code)
{
    u32 mask = IPT_g_ulPadAndJoyButtonFlag[code - IPT_FIRST_BUTTON];
    s32 released = 0;

    if ((device->m_p_stState->m_ulButtons & mask) != mask) {
        u32 previous = INO_fn_ulJoyPreviousButtons(device);
        u32 previousMask =
            IPT_g_ulPadAndJoyButtonFlag[IPT_g_hNextParameter->value - IPT_FIRST_BUTTON];

        previous = (previous & previousMask) ^ previousMask;
        released = previous == 0;
    }
    return released;
}

s32 IPT_fn_bIsKeyWordValid_JoystickJustReleased(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 result = 0;
    s32 joystick;
    u16 code;
    stDevice* device;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    device = IPT_g_hInputStructure.devices[joystick];
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        code = _hKeyWordElement[2].code;
        if (code >= IPT_FIRST_BUTTON && code < IPT_FIRST_BUTTON + IPT_BUTTON_COUNT) {
            result = inv_IPT_fn_lJustReleased(device, (s16) code);
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

u8 IPT_fn_bIsKeyWordValid_JoystickPressed(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 result = 0;
    s32 joystick;
    u16 code;
    stDevice* device;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    device = IPT_g_hInputStructure.devices[joystick];
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        code = _hKeyWordElement[2].code;
        if (code >= IPT_FIRST_BUTTON && code < IPT_FIRST_BUTTON + IPT_BUTTON_COUNT) {
            s32 button = (s16) code;
            stJoystickState* state = device->m_p_stState;
            u32 buttons;
            u32 mask;
            u32 missing;

            button -= IPT_FIRST_BUTTON;
            buttons = state->m_ulButtons;
            mask = IPT_g_ulPadAndJoyButtonFlag[button];
            missing = buttons & mask;
            missing ^= mask;
            result = missing == 0;
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

u8 IPT_fn_bIsKeyWordValid_JoystickReleased(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 result = 0;
    s32 joystick;
    u16 code;
    stDevice* device;

    IPT_g_hNextParameter = &_hKeyWordElement[1];
    joystick = _hKeyWordElement[1].value + 2;
    IPT_g_hNextParameter = &_hKeyWordElement[2];
    device = IPT_g_hInputStructure.devices[joystick];
    if (IPT_g_hInputStructure.deviceState[joystick] == IPT_DEVICE_JOYSTICK) {
        code = _hKeyWordElement[2].code;
        if (code >= IPT_FIRST_BUTTON && code < IPT_FIRST_BUTTON + IPT_BUTTON_COUNT) {
            s32 button = (s16) code;
            stJoystickState* state = device->m_p_stState;
            u32 buttons;
            u32 mask;
            u32 missing;

            button -= IPT_FIRST_BUTTON;
            buttons = state->m_ulButtons;
            mask = IPT_g_ulPadAndJoyButtonFlag[button];
            missing = buttons & mask;
            missing ^= mask;
            result = missing != 0;
        }
    }
    IPT_g_hNextParameter++;
    return result;
}

s32 func_0011FE80(tdu_KeyWordElementUnion_* keyword)
{
    u8 result = 0;
    s32 joystick;
    s32 button;
    s32 minimum;
    s32 maximum;
    stDevice* device;
    u8 state;
    f32 pressure;
    s32 percent;

    IPT_g_hNextParameter = &keyword[1];
    joystick = keyword[1].value + 2;
    device = IPT_g_hInputStructure.devices[joystick];
    IPT_g_hNextParameter = &keyword[2];
    button = keyword[2].value;
    IPT_g_hNextParameter = &keyword[3];
    minimum = keyword[3].value;
    IPT_g_hNextParameter = &keyword[4];
    maximum = keyword[4].value;
    state = IPT_g_hInputStructure.deviceState[joystick];
    if (state == 3 || state == 4) {
        pressure = device->m_p_stState->m_a_fButtonValue[button] * 100.0f;
        percent = inv_MTH_fn_lToIntegerOrdered(pressure);
        result = percent >= minimum && percent <= maximum;
        if (result == 1)
            g_h_ActualEntryInput->xAnalogicValue = pressure;
    }
    IPT_g_hNextParameter++;
    return result;
}

u8 IPT_fn_bIsKeyWordValid_ActionJustValidated(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 ucReturn;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    ucReturn = IPT_fn_bIsJustValidated(_hKeyWordElement[1].hEntryElement);
    IPT_g_hNextParameter++;
    return ucReturn == 1;
}

u8 IPT_fn_bIsKeyWordValid_ActionValidated(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 ucReturn;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    ucReturn = IPT_fn_bIsValidated(_hKeyWordElement[1].hEntryElement);
    IPT_g_hNextParameter++;
    return ucReturn == 1;
}

u8 IPT_fn_bIsKeyWordValid_ActionJustInvalidated(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 ucReturn;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    ucReturn = IPT_fn_bIsJustInvalidated(_hKeyWordElement[1].hEntryElement);
    IPT_g_hNextParameter++;
    return ucReturn == 1;
}

u8 IPT_fn_bIsKeyWordValid_ActionInvalidated(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    s32 ucReturn;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    ucReturn = IPT_fn_bIsInvalidated(_hKeyWordElement[1].hEntryElement);
    IPT_g_hNextParameter++;
    return ucReturn == 1;
}

u8 IPT_fn_bIsKeyWordValid_Sequence(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    u8 ucReturn;
    u32 i;

    IPT_g_hNextParameter = _hKeyWordElement + 2;
    IPT_g_ulSequenceAnalyseNumber = 0;
    ucReturn = 1;
    for (i = 0; ucReturn && i < (u32) _hKeyWordElement[1].ulNumberOfSequence; i++)
        ucReturn &= inv_IPT_fn_bIsKeyWordValid(IPT_g_hNextParameter) == 1;
    return ucReturn;
}

u8 IPT_fn_bIsKeyWordValid_SequencePad(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    u8 ucReturn = 0;
    u32 ulPadNumber;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    ulPadNumber = IPT_g_hNextParameter->value;
    IPT_g_hNextParameter++;
    if (IPT_g_hInputStructure.historic[IPT_g_ulSequenceAnalyseNumber].bf6DeviceType == ulPadNumber + 2 &&
        IPT_g_hInputStructure.historic[IPT_g_ulSequenceAnalyseNumber].bf2State == 0 &&
        IPT_g_hInputStructure.historic[IPT_g_ulSequenceAnalyseNumber].swDeviceValue == IPT_g_hNextParameter->value)
        ucReturn = 1;
    IPT_g_ulSequenceAnalyseNumber++;
    IPT_g_hNextParameter++;
    return ucReturn;
}

u8 IPT_fn_bIsKeyWordValid_SequencePadEnd(tdu_KeyWordElementUnion_* _hKeyWordElement)
{
    u8 ucReturn = 0;
    u32 ulPadNumber;

    IPT_g_hNextParameter = _hKeyWordElement + 1;
    ulPadNumber = IPT_g_hNextParameter->value;
    IPT_g_hNextParameter++;
    if (IPT_g_hInputStructure.historic[IPT_g_ulSequenceAnalyseNumber].bf6DeviceType == ulPadNumber + 2 &&
        IPT_g_hInputStructure.historic[IPT_g_ulSequenceAnalyseNumber].bf2State == 1 &&
        IPT_g_hInputStructure.historic[IPT_g_ulSequenceAnalyseNumber].swDeviceValue == IPT_g_hNextParameter->value)
        ucReturn = 1;
    IPT_g_ulSequenceAnalyseNumber++;
    IPT_g_hNextParameter++;
    return ucReturn;
}
