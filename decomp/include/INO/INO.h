#ifndef REVO_INO_INO_H
#define REVO_INO_INO_H

#include "ps2/types.h"
#include "IPT/IPT.h"

typedef struct PS2_tdstControllerData_ {
    u8 ucStatus;
    u8 ucControllerIDExt;
    u16 uwDigitalButtons;
    u8 a2_ucAnalogStickRight[2];
    u8 a2_ucAnalogStickLeft[2];
    u8 a12_ucAnalogButtons[12];
    union {
        u8 a12_ucReserved[12];
        struct {
            u16 inv_uwReadStatus;
            u16 inv_uwStepCount;
        } stDemo_inv;
    } u_inv;
} PS2_tdstControllerData_;

extern s32 INO_fn_wReadAllDevices(void);
extern s32 func_0011DC30(void);
extern s32 INO_fn_wReadJoystick(u16 mask, s16 mode);
extern s32 INO_fn_wActivateDevice(stDevice* _hDev);
extern s16 INO_fn_wDeactivateDevice(stDevice* _hDev);
extern stDevice* func_0011DC60(u16 kind, s16 id, s16 mode);
extern void loc_11E218(stDevice* device);

#endif
