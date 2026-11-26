#ifndef REVO_IPT_IPT_H
#define REVO_IPT_IPT_H

#include "ps2/types.h"
#include "MTH/MTH.h"

typedef struct stJoystickState {
    u8 m_ucStatus;
    u16 m_uwDirections;
    u32 m_ulButtons;
    f32 m_a_fAxisValue[6];
    f32 m_a_fButtonValue[16];
} stJoystickState;

typedef struct stDevice {
    u16 m_uwType;
    s16 m_wPort;
    void* m_p_stCaps;
    stJoystickState* m_p_stState;
    stJoystickState* m_p_stLastState;
    s32 (*m_pfnRead)(struct stDevice* device);
} stDevice;

typedef union tdu_KeyWordElementUnion_ {
    u8 type;
    s16 value;
    u16 code;
    s32 ulNumberOfSequence;
    void* hEntryElement;
} tdu_KeyWordElementUnion_;

typedef struct IPT_tdstEntryElement_ {
    s32 ulNumberOfKeyWordElement;
    tdu_KeyWordElementUnion_* d_stKeyWordElementArray;
    char* p_szActionName;
    s32 lState;
    f32 xAnalogicValue;
    u8 bIsActivate;
} IPT_tdstEntryElement_;

typedef struct IPT_tdstHistoricElement_ {
    u8 bf2State;
    u8 bf6DeviceType;
    u8 ucCounter;
    s16 swDeviceValue;
} IPT_tdstHistoricElement_;

typedef struct gu_IPT_InputStructure {
    stDevice* devices[18];
    u32 entryCount;
    IPT_tdstEntryElement_* entries;
    IPT_tdstHistoricElement_* historic;
    s16 historicCount;
    u8 historicMode;
    u8 deviceState[18];
    u8 ucPadAndJoyCounter[16][42];
} gu_IPT_InputStructure;

extern u8 IPT_fn_bIsJustInvalidated(IPT_tdstEntryElement_* entry);
extern u8 IPT_fn_bIsJustValidated(IPT_tdstEntryElement_* entry);
extern u8 IPT_fn_bIsValidated(IPT_tdstEntryElement_* entry);
extern u8 IPT_fn_bIsInvalidated(IPT_tdstEntryElement_* entry);
extern f32 IPT_fn_xGetAnalogicValue(IPT_tdstEntryElement_* _hEntryElement);
extern void IPT_fn_vReadInput(void);

struct IptObjectView;
extern s32 IPT_fn_bIsJoystickButtonPressed(s32 deviceIndex, s32 buttonMask);
extern s32 IPT_fn_ucGetFirstValidJoystick(void);
extern s32 inv_IPT_fn_bIsPadConnected(s32 deviceIndex);
extern s32 inv_IPT_fn_lGetPadWithButton8Pressed(void);
extern void inv_IPT_fn_bSwapJoystickDevices(s32 device0, s32 device1);
extern s32 inv_IPT_fn_lGetPadWithButton8Released(void);
extern void IPT_fn_vActivateAllEntryElements(void);
extern void IPT_fn_vActiveJoystickControl(void);
extern void IPT_fn_vDesactiveJoystickAndPadControl(void);
extern void IPT_fn_vResetInputEntry(void);
extern void inv_IPT_fn_vEnableAllPads(void);
extern void inv_IPT_fn_vIncrementInputFrameCounter(void);
extern char* IPT_fn_szGetStringOf(IPT_tdstEntryElement_* _hElement);
extern s32 IPT_fn_bIsJoystickButtonJustPressed(u32 deviceIndex, u32 buttonMask);
extern s32 IPT_fn_vDesactiveJoystickControl(s32 deviceIndex);
extern s32 inv_IPT_fn_bActivateJoystick(s32 deviceIndex);
extern u8 IPT_fn_bChangeIPTValue(IPT_tdstEntryElement_* _hElement, u8 ucWhichTest);
extern u8 IPT_fn_bModifyIPTAction(IPT_tdstEntryElement_* entry,
                                  IPT_tdstEntryElement_* _hReplacingElement);
extern void IPT_fn_vActivateEntryElement(IPT_tdstEntryElement_* _hEntryElement);
extern void IPT_fn_vDesactivateEntryElement(IPT_tdstEntryElement_* _hEntryElement);
extern u8 IPT_fn_bAButtonPadJustPressed(void);
extern void inv_IPT_fn_vInitInputFrameState(void);
extern void IPT_fn_vInitInput(s16 _sHistoricSize);
extern u32 inv_IPT_fn_ulGetInputFrameCounter(void);
extern IPT_tdstEntryElement_* inv_IPT_fn_hGetJoystickAxeEntry(s16 entry, s16 field);
extern void func_001203D0(struct IptObjectView* object, void* value);

extern gu_IPT_InputStructure IPT_g_hInputStructure;

extern void IPT_fn_vActualizeHistoric(void);

extern s32 IPT_fn_bIsEntryActionValid(IPT_tdstEntryElement_* _hEntryElement);

typedef struct IPT_tdstPadReadingOutput_ {
    MTH3D_tdstVector_ stGlobalVector;
    s16 wHorizontalAxis;
    s16 wVerticalAxis;
    f32 xAnalogForce;
    f32 xTrueAnalogForce;
    f32 xRotationAngle;
    s32 lSector;
} IPT_tdstPadReadingOutput_;

typedef struct JoystickPads {
    IPT_tdstPadReadingOutput_ left;
    IPT_tdstPadReadingOutput_ right;
} JoystickPads;

extern IPT_tdstEntryElement_* inv_IPT_a_pEntryElementRecords[][4];

extern u32 IPT_g_ulPadAndJoyButtonFlag[];
extern s16 inv_IPT_a_wButtonKeyWordValues[];

#endif
