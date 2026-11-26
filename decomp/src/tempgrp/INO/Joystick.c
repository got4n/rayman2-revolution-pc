#include "common.h"

#include "IPT/IPT.h"
#include "INO/INO.h"

#include "ps2/libc.h"
#include "GAM/GAM.h"

#define PAD_PACKET_SIZE 32
#define PAD_PRESSURE_COUNT 12
#define PAD_KIND_DIGITAL 4
#define PAD_KIND_ANALOG 7
#define PAD_MODE_PRESSURE 0x79
#define PAD_AXIS_CENTER 128
#define PAD_AXIS_SCALE (1.0f / 128.0f)
#define PAD_DEMO_AXIS_MIN 64
#define PAD_DEMO_AXIS_LIMIT 193

#define AXIS_LEFT_X 0
#define AXIS_LEFT_Y 1
#define AXIS_RIGHT_X 3
#define AXIS_RIGHT_Y 4

typedef struct stAxisCaps {
    s32 m_lRange;
} stAxisCaps;

typedef struct stButtonCaps {
    s32 m_lRange;
} stButtonCaps;

typedef struct stJoystickCaps {
    stAxisCaps m_a_stAxisCaps[6];
    stButtonCaps m_a_stButtonCaps[16];
} stJoystickCaps;

extern s16 INO_gs_xNbDevice;
extern s16 inv_INO_wPolledDevice;
extern stDevice* INO_gs_a_tdhDevice[2];
extern f32 D_00392808;
extern f32 D_0039280C;
extern f32 D_00392810;
extern volatile f32 D_00392814;
extern f32 D_00392818;
extern volatile f32 D_0039281C;
extern f32 D_00392820;
extern volatile f32 D_00392824;

#ifdef PC_PORT
#include "PC/PC.h"

#define PAD_MODE_DUALSHOCK 0x73

static s32 inv_INO_fn_wReadPad(s32 port, s32 slot, PS2_tdstControllerData_* packet)
{
    PcPad pad;

    memset(packet, 0, sizeof(PS2_tdstControllerData_));
    if (!pc_pad_read(port, &pad)) {
        return 0;
    }
    packet->ucControllerIDExt = PAD_MODE_DUALSHOCK;
    packet->uwDigitalButtons = ~pad.buttons;
    packet->a2_ucAnalogStickLeft[0] = pad.leftX;
    packet->a2_ucAnalogStickLeft[1] = pad.leftY;
    packet->a2_ucAnalogStickRight[0] = pad.rightX;
    packet->a2_ucAnalogStickRight[1] = pad.rightY;
    return PAD_PACKET_SIZE;
}

#define scePadRead inv_INO_fn_wReadPad
#else
s32 scePadGetState(s32 port, s32 slot);
s32 scePadInit(s32 mode);
s32 scePadRead(s32 port, s32 slot, PS2_tdstControllerData_* packet);
s32 scePadSetMainMode(s32 port, s32 slot, s32 offset, s32 lock);
#endif
void freemem(void* block);
void inv_INO_fn_vClearJoystickStates(stDevice* device);

s32 INO_fn_wInit(void);
s32 inv_INO_fn_wOpenPadPort(s16 id);
s32 inv_INO_fn_bSetPadMainMode(s16 id);
s32 inv_INO_fn_wInitJoystickCaps(s16 id, stJoystickCaps* calibration);
s32 INO_fn_wIsConnected(s16 id);
stDevice* INO_fn_hCreateDevice(s16 id, s16 mode);
s32 func_0011E398(stDevice* device);
s32 inv_INO_fn_wUpdateDeviceActivation(stDevice* device);
s32 inv_INO_fn_bIsPrimaryPadConnected(stDevice* device);
s32 inv_INO_fn_wInitializeDeviceInputState(stDevice* device);
s32 inv_INO_fn_wDeviceReadCallback(stDevice* device);

s32 func_0011DC30(void)
{
    memset(INO_gs_a_tdhDevice, 0, sizeof(INO_gs_a_tdhDevice));
    INO_fn_wInit();
    return 1;
}

stDevice* func_0011DC60(u16 kind, s16 id, s16 mode)
{
    stDevice** slot;
    stDevice* device;
    s16 i;

    i = INO_gs_xNbDevice - 1;
    for (slot = &INO_gs_a_tdhDevice[i]; i >= 0; i--) {
        device = *slot;
        if (device->m_uwType == kind && device->m_wPort == id) {
            goto found;
        }
        slot--;
    }
    device = NULL;
    if (kind == 1) {
        device = INO_fn_hCreateDevice(id, mode);
    }
    goto store;
found:
    return device;
store:
    INO_gs_a_tdhDevice[INO_gs_xNbDevice++] = device;
    return device;
}

s32 func_0011DD30(stDevice* device)
{
    stDevice** slot;
    s16 i;

    if (device != NULL) {
        i = INO_gs_xNbDevice - 1;
        for (slot = &INO_gs_a_tdhDevice[i]; i >= 0; i--) {
            if (*slot == device) {
                return 1;
            }
            slot--;
        }
    }
    return 0;
}

s32 INO_fn_wActivateDevice(stDevice* _hDev)
{
    s32 result = 0;

    if (func_0011DD30(_hDev) != 0) {
        if (_hDev->m_p_stState->m_ucStatus & 2) {
            return 1;
        }
        if (_hDev->m_uwType == 1) {
            result = func_0011E398(_hDev);
        }
    }
    if (result == 1) {
        _hDev->m_p_stState->m_ucStatus |= 2;
    }
    return result;
}

s16 INO_fn_wDeactivateDevice(stDevice* _hDev)
{
    if (func_0011DD30(_hDev) != 0) {
        _hDev->m_p_stState->m_ucStatus &= ~2;
        return 1;
    }
    return 0;
}

s32 INO_fn_wReadJoystick(u16 mask, s16 mode)
{
    stDevice** slot;
    s16 result = 1;
    s32 value;
    s16 i;

    if (inv_DEMO_bPlaying != 0) {
        i = INO_gs_xNbDevice - 1;
        for (slot = &INO_gs_a_tdhDevice[i]; i >= 0; i--) {
            if (((*slot)->m_uwType & mask) != 0 && (*slot)->m_pfnRead != NULL) {
                (*slot)->m_pfnRead(*slot);
            }
            slot--;
        }
    } else {
        stDevice* device;

        i = INO_gs_xNbDevice - 1;
        for (slot = &INO_gs_a_tdhDevice[i]; i >= 0; i--) {
            device = *slot;
            if ((device->m_uwType & mask) != 0 && (device->m_p_stState->m_ucStatus & 2) != 0) {
                if (mode == 1) {
                    inv_INO_fn_wUpdateDeviceActivation(device);
                }
                if ((device->m_p_stState->m_ucStatus & 1) != 0) {
                    if (device->m_pfnRead != NULL) {
                        value = device->m_pfnRead(device);
                    } else {
                        value = -1;
                    }
                    if (result != -1) {
                        result = value == -1 ? -1 : result & value;
                    }
                }
            }
            slot--;
        }
    }
    return result;
}

s32 inv_INO_fn_wUpdateDeviceActivation(stDevice* device)
{
    s32 result;

    if (device == NULL) {
        return -1;
    }
    if (device->m_uwType == 1) {
        result = inv_INO_fn_bIsPrimaryPadConnected(device);
    } else {
        result = -1;
    }
    if (result == 1) {
        if ((device->m_p_stState->m_ucStatus & 1) == 0) {
            device->m_p_stState->m_ucStatus |= 1;
            INO_fn_wActivateDevice(device);
            inv_INO_fn_wInitializeDeviceInputState(device);
        }
    } else if (result == 0) {
        if ((device->m_p_stState->m_ucStatus & 1) != 0) {
            device->m_p_stState->m_ucStatus &= ~1;
            INO_fn_wDeactivateDevice(device);
        }
    }
    return result;
}

s32 inv_INO_fn_wInitializeDeviceInputState(stDevice* device)
{
    if (device->m_uwType == 1) {
        return inv_INO_fn_wInitJoystickCaps(device->m_wPort, device->m_p_stCaps);
    }
    return -1;
}

s32 INO_fn_wReadAllDevices(void)
{
    stDevice* device;
    s32 result;

    if (inv_INO_wPolledDevice >= INO_gs_xNbDevice) {
        inv_INO_wPolledDevice++;
        if (inv_INO_wPolledDevice >= 8) {
            inv_INO_wPolledDevice = 0;
        }
        return 1;
    }
    device = INO_gs_a_tdhDevice[inv_INO_wPolledDevice];
    result = inv_INO_fn_wUpdateDeviceActivation(device);
    if (result != -1) {
        if ((device->m_p_stState->m_ucStatus & 4) != 0) {
            device->m_p_stState->m_ucStatus &= ~4;
            INO_fn_wActivateDevice(device);
            printf("---> Device %d back in normal mode\n", inv_INO_wPolledDevice);
        }
        inv_INO_wPolledDevice++;
        if (inv_INO_wPolledDevice >= 8) {
            inv_INO_wPolledDevice = 0;
        }
    } else {
        INO_fn_wDeactivateDevice(device);
        device->m_p_stState->m_ucStatus |= 4;
        printf("---> WeirdStatus set on Device %d\n", inv_INO_wPolledDevice);
    }
    return result;
}

void loc_11E218(stDevice* device)
{
    if (device->m_uwType == 1) {
        inv_INO_fn_vClearJoystickStates(device);
    }
}

stDevice* INO_fn_hCreateDevice(s16 _wPort, s16 _wHistoricSize)
{
    stDevice* device;
    s32 state = inv_INO_fn_wOpenPadPort(_wPort);

    if (state != 1) {
        return NULL;
    }
    device = malloc(sizeof(stDevice));
    if (device == NULL) {
        return NULL;
    }
    device->m_wPort = _wPort;
    device->m_uwType = state;
    device->m_p_stCaps = malloc(sizeof(stJoystickCaps));
    if (device->m_p_stCaps != NULL) {
        memset(device->m_p_stCaps, 0, sizeof(stJoystickCaps));
        inv_INO_fn_wInitJoystickCaps(device->m_wPort, device->m_p_stCaps);
        device->m_p_stState = malloc(sizeof(stJoystickState));
        if (device->m_p_stState != NULL) {
            memset(device->m_p_stState, 0, sizeof(stJoystickState));
            device->m_p_stLastState = malloc(sizeof(stJoystickState));
            if (device->m_p_stLastState != NULL) {
                memset(device->m_p_stLastState, 0, sizeof(stJoystickState));
                if (INO_fn_wIsConnected(device->m_wPort) == state) {
                    device->m_p_stState->m_ucStatus |= 1;
                }
                device->m_pfnRead = inv_INO_fn_wDeviceReadCallback;
                return device;
            }
            freemem(device->m_p_stState);
        }
        freemem(device->m_p_stCaps);
    }
    freemem(device);
    return NULL;
}

s32 inv_INO_fn_bIsPrimaryPadConnected(stDevice* device)
{
    if (device->m_uwType == 1) {
        return INO_fn_wIsConnected(device->m_wPort);
    }
    return -1;
}

s32 func_0011E398(stDevice* device)
{
    if (device->m_uwType == 1) {
        return inv_INO_fn_bSetPadMainMode(device->m_wPort);
    }
    return -1;
}

void inv_INO_fn_vJoyUpdateDirection(s16 id, stJoystickState* state,
                                    stJoystickCaps* calibration)
{
    f32 axis;
    f32 threshold;
    u16 flags;

    axis = state->m_a_fAxisValue[0];
    state->m_uwDirections = 0;
    if (axis < D_00392808) {
        state->m_uwDirections = 1;
    } else if (D_0039280C < axis) {
        state->m_uwDirections = 2;
    }

    axis = state->m_a_fAxisValue[1];
    if (D_00392810 < axis) {
        flags = state->m_uwDirections | 8;
        goto storeVertical;
    }
    threshold = D_00392814;
    if (axis < threshold) {
        flags = state->m_uwDirections | 4;
    storeVertical:
        state->m_uwDirections = flags;
        axis = state->m_a_fAxisValue[3];
        threshold = D_00392818;
    }

    if (state->m_a_fAxisValue[3] < threshold) {
        flags = state->m_uwDirections | 0x10;
        goto storeRightHorizontal;
    }
    threshold = D_0039281C;
    if (threshold < state->m_a_fAxisValue[3]) {
        flags = state->m_uwDirections | 0x20;
    storeRightHorizontal:
        state->m_uwDirections = flags;
        axis = state->m_a_fAxisValue[4];
        threshold = D_00392820;
    }

    if (threshold < state->m_a_fAxisValue[4]) {
        state->m_uwDirections |= 0x80;
    } else if (state->m_a_fAxisValue[4] < D_00392824) {
        state->m_uwDirections |= 0x40;
    }
}

s16 INO_fn_wGetJoystickCurrentPosition(s16 padId, stJoystickState* padState,
                                       stJoystickCaps* calibration)
{
    PS2_tdstControllerData_ packet;
    s16 status;
    s32 abortDemo;
    s32 kind;
    s32 demoKind;
    u32 bits;
    s32 leftXOffset;
    s32 leftYOffset;
    s32 rightXOffset;
    s32 rightYOffset;
    s32 signedAxis;
    f32 normalizedLeftX;
    f32 normalizedLeftY;
    f32 normalizedRightX;
    f32 normalizedRightY;
    s32 leftXCalibrationOffset;
    s32 leftYCalibrationOffset;
    s32 rightXCalibrationOffset;
    s32 rightYCalibrationOffset;
    s32 leftXValue;
    s32 leftYValue;
    s32 rightXValue;
    s32 rightYValue;
    s32 maxAxisValue;
    s32 value;
    s32 maxValue;
    s32 countFixed;
    s32 remaining;
    u8* rawPressure;
    f32* pressure;
    stButtonCaps* buttonCaps;
    u16 rawButtons;

    bits = (u32) (u16) padId << 16;
    padId = (s32) bits >> 16;
    status = 1;
    if (inv_DEMO_bPlaying != 0) {
        abortDemo = 0;
        if (scePadRead(padId & 1, (s32) bits >> 17, &packet) == PAD_PACKET_SIZE) {
            abortDemo = (u16) ~packet.uwDigitalButtons != 0;
            if (!abortDemo) {
                demoKind = (packet.ucControllerIDExt >> 4) & 7;
                if (demoKind == PAD_KIND_ANALOG || demoKind == 0x53) {
                    if (packet.a2_ucAnalogStickLeft[0] < PAD_DEMO_AXIS_MIN) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickLeft[0] >= PAD_DEMO_AXIS_LIMIT) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickLeft[1] < PAD_DEMO_AXIS_MIN) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickLeft[1] >= PAD_DEMO_AXIS_LIMIT) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickRight[0] < PAD_DEMO_AXIS_MIN) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickRight[0] >= PAD_DEMO_AXIS_LIMIT) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickRight[1] < PAD_DEMO_AXIS_MIN) {
                        goto outOfRange;
                    }
                    if (packet.a2_ucAnalogStickRight[1] >= PAD_DEMO_AXIS_LIMIT) {
                    outOfRange:
                        abortDemo = 1;
                    }
                }
            }
        }
        if (abortDemo) {
            inv_DEMO_fn_vEndDemoPlayback();
            status = 0;
        } else if (padId == 0) {
            inv_DEMO_fn_vReadNextDemoRecord(&packet, &status);
        }
    } else if (scePadRead(padId & 1, (s32) bits >> 17, &packet) != PAD_PACKET_SIZE) {
        status = -1;
    }

    if (status != 1) {
        return status;
    }
    if (packet.ucStatus != 0) {
        return 0;
    }

    rawButtons = packet.uwDigitalButtons;
    bits = packet.ucControllerIDExt;
    kind = (bits >> 4) & 7;
    padState->m_ulButtons = ~rawButtons;
    if (kind == PAD_KIND_ANALOG) {
        leftXOffset = packet.a2_ucAnalogStickLeft[0] - PAD_AXIS_CENTER;
        signedAxis = (s8) leftXOffset;
        normalizedLeftX = signedAxis * PAD_AXIS_SCALE;
        leftYOffset = packet.a2_ucAnalogStickLeft[1] - PAD_AXIS_CENTER;
        signedAxis = (s8) leftYOffset;
        normalizedLeftY = signedAxis * PAD_AXIS_SCALE;
        rightXOffset = packet.a2_ucAnalogStickRight[0] - PAD_AXIS_CENTER;
        signedAxis = (s8) rightXOffset;
        normalizedRightX = signedAxis * PAD_AXIS_SCALE;
        rightYOffset = packet.a2_ucAnalogStickRight[1] - PAD_AXIS_CENTER;
        signedAxis = (s8) rightYOffset;
        normalizedRightY = signedAxis * PAD_AXIS_SCALE;
        padState->m_a_fAxisValue[0] = normalizedLeftX;
        padState->m_a_fAxisValue[1] = normalizedLeftY;
        padState->m_a_fAxisValue[4] = normalizedRightY;
        padState->m_a_fAxisValue[3] = normalizedRightX;
    } else if (kind != PAD_KIND_DIGITAL) {
        maxAxisValue = calibration->m_a_stAxisCaps[AXIS_LEFT_X].m_lRange;
        leftXCalibrationOffset = packet.a2_ucAnalogStickLeft[0] - PAD_AXIS_CENTER;
        signedAxis = (s8) leftXCalibrationOffset;
        leftXValue = signedAxis;
        if (maxAxisValue < leftXValue) {
            calibration->m_a_stAxisCaps[AXIS_LEFT_X].m_lRange = leftXValue;
            padState->m_a_fAxisValue[0] = 1.0f;
        } else if (maxAxisValue < -leftXValue) {
            calibration->m_a_stAxisCaps[AXIS_LEFT_X].m_lRange = -leftXValue;
            padState->m_a_fAxisValue[0] = -1.0f;
        } else {
            padState->m_a_fAxisValue[0] = (f32) leftXValue / (f32) maxAxisValue;
        }
        maxAxisValue = calibration->m_a_stAxisCaps[AXIS_LEFT_Y].m_lRange;

        leftYCalibrationOffset = packet.a2_ucAnalogStickLeft[1] - PAD_AXIS_CENTER;
        signedAxis = (s8) leftYCalibrationOffset;
        leftYValue = signedAxis;
        if (maxAxisValue < leftYValue) {
            calibration->m_a_stAxisCaps[AXIS_LEFT_Y].m_lRange = leftYValue;
            padState->m_a_fAxisValue[1] = 1.0f;
        } else if (maxAxisValue < -leftYValue) {
            calibration->m_a_stAxisCaps[AXIS_LEFT_Y].m_lRange = -leftYValue;
            padState->m_a_fAxisValue[1] = -1.0f;
        } else {
            padState->m_a_fAxisValue[1] = (f32) leftYValue / (f32) maxAxisValue;
        }
        maxAxisValue = calibration->m_a_stAxisCaps[AXIS_RIGHT_X].m_lRange;

        rightXCalibrationOffset = packet.a2_ucAnalogStickRight[0] - PAD_AXIS_CENTER;
        signedAxis = (s8) rightXCalibrationOffset;
        rightXValue = signedAxis;
        if (maxAxisValue < rightXValue) {
            calibration->m_a_stAxisCaps[AXIS_RIGHT_X].m_lRange = rightXValue;
            padState->m_a_fAxisValue[3] = 1.0f;
        } else if (maxAxisValue < -rightXValue) {
            calibration->m_a_stAxisCaps[AXIS_RIGHT_X].m_lRange = -rightXValue;
            padState->m_a_fAxisValue[3] = -1.0f;
        } else {
            padState->m_a_fAxisValue[3] = (f32) rightXValue / (f32) maxAxisValue;
        }
        maxAxisValue = calibration->m_a_stAxisCaps[AXIS_RIGHT_Y].m_lRange;

        rightYCalibrationOffset = packet.a2_ucAnalogStickRight[1] - PAD_AXIS_CENTER;
        rightYValue = signedAxis = (s8) rightYCalibrationOffset;
        if (maxAxisValue < rightYValue) {
            calibration->m_a_stAxisCaps[AXIS_RIGHT_Y].m_lRange = rightYValue;
            padState->m_a_fAxisValue[4] = 1.0f;
        } else if (maxAxisValue < -rightYValue) {
            calibration->m_a_stAxisCaps[AXIS_RIGHT_Y].m_lRange = -rightYValue;
            padState->m_a_fAxisValue[4] = -1.0f;
        } else {
            padState->m_a_fAxisValue[4] = (f32) rightYValue / (f32) maxAxisValue;
        }
    }

    if ((bits & 0xFF) == PAD_MODE_PRESSURE) {
        buttonCaps = calibration->m_a_stButtonCaps;
        pressure = padState->m_a_fButtonValue;
        rawPressure = packet.a12_ucAnalogButtons;
        countFixed = (PAD_PRESSURE_COUNT - 1) << 16;
        do {
            value = *rawPressure;
            maxValue = buttonCaps->m_lRange;
            if (maxValue < value) {
                buttonCaps->m_lRange = value;
                *pressure = 1.0f;
            } else {
                *pressure = (f32) value / (f32) maxValue;
            }
            remaining = countFixed;
            rawPressure++;
            countFixed -= 0x10000;
            remaining >>= 16;
            buttonCaps++;
            pressure++;
        } while ((s16) remaining > 0);
    }
    return 1;
}

#ifdef PC_PORT
s32 INO_fn_wInit(void)
{
    return 1;
}

s32 inv_INO_fn_wOpenPadPort(s16 id)
{
    return 1;
}

s32 inv_INO_fn_bSetPadMainMode(s16 id)
{
    return 1;
}
#else
s32 INO_fn_wInit(void)
{
    return scePadInit(0) == 1 ? 1 : -1;
}

typedef u8 PadDmaBuffer[0x100];

extern PadDmaBuffer inv_INO_a_stPadDmaBuffers[];

s32 scePadPortOpen(s32 port, s32 slot, void* buffer);
void fn_vTimerWait(s32 milliseconds);

s32 inv_INO_fn_wOpenPadPort(s16 id)
{
    s32 port = id;
    s32 slot = id >> 1;
    s16 attempts = 0;
    s16 result;

    do {
        result = scePadPortOpen((s16) (port & 1), (s16) slot, inv_INO_a_stPadDmaBuffers[port]) == 1
                     ? 1
                     : -1;
        fn_vTimerWait(300);
        if (++attempts >= 3)
            break;
    } while (result == -1);
    return result;
}

s32 inv_INO_fn_bSetPadMainMode(s16 id)
{
    scePadSetMainMode(id & 1, id >> 1, 1, 3);
    return 1;
}
#endif

s32 inv_INO_fn_wInitJoystickCaps(s16 id, stJoystickCaps* calibration)
{
    calibration->m_a_stAxisCaps[AXIS_LEFT_X].m_lRange = 0x40;
    calibration->m_a_stAxisCaps[AXIS_LEFT_Y].m_lRange = 0x40;
    calibration->m_a_stAxisCaps[AXIS_RIGHT_X].m_lRange = 0x40;
    calibration->m_a_stAxisCaps[AXIS_RIGHT_Y].m_lRange = 0x40;
    return 1;
}

#ifdef PC_PORT
s32 INO_fn_wIsConnected(s16 id)
{
    PcPad pad;

    return pc_pad_read(id & 1, &pad);
}
#else
s32 INO_fn_wIsConnected(s16 id)
{
    s32 state = scePadGetState(id & 1, id >> 1);

    if (id == 0) {
        if (state == 2) {
            return 1;
        }
        if (state >= 3) {
            if (state != 6) {
                if (state != 7) {
                    goto print;
                }
                printf("---> JoyCheckConnection ERROR -> ignoring state\n");
            }
        } else if (state != 0) {
        print:
            printf("---> JoyCheckConnection = %d\n", state);
        }
    }
    if (state == 2 || state == 6) {
        return 1;
    }
    if (state == 0) {
        return 0;
    }
    return -1;
}
#endif

s32 inv_INO_fn_wDeviceReadCallback(stDevice* device)
{
    stJoystickState* current = device->m_p_stState;
    stJoystickState* previous = device->m_p_stLastState;
    stJoystickCaps* calibration = device->m_p_stCaps;
    s16 id = device->m_wPort;
    s32 result;

    memcpy(previous, current, sizeof(stJoystickState));
    result = INO_fn_wGetJoystickCurrentPosition(id, current, calibration);
    if (result != 1) {
        inv_INO_fn_vClearJoystickStates(device);
    }
    inv_INO_fn_vJoyUpdateDirection(id, current, calibration);
    return result;
}

static inline void inv_INO_fn_vClearJoystickState(stJoystickState* state)
{
    u8 ucStatus = state->m_ucStatus;

    memset(state, 0, sizeof(stJoystickState));
    state->m_ucStatus = ucStatus;
}

void inv_INO_fn_vClearJoystickStates(stDevice* device)
{
    stJoystickState* current = device->m_p_stState;
    stJoystickState* previous = device->m_p_stLastState;

    inv_INO_fn_vClearJoystickState(current);
    inv_INO_fn_vClearJoystickState(previous);
}
