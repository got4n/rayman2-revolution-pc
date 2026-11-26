#include "common.h"

#include "IPT/IPT.h"
#include "GAM/GAM.h"

void func_001028F0(void)
{
}

void fn_vEngineReadInput(void)
{
    s32 firstJoystick;

    firstJoystick = IPT_fn_ucGetFirstValidJoystick();
    IPT_fn_vReadInput();
    func_001028F0();
    if (g_stEngineStructure.xDemoMode != 0)
        return;

    if (fn_bIsEngineInPause() != 0) {
        if (firstJoystick == 0) {
            if (inv_IPT_fn_lGetPadWithButton8Released() == 0) {
                inv_IPT_fn_bSwapJoystickDevices(0, 0);
                fn_vResetEngineFromPaused();
            }
        } else if (firstJoystick > 0) {
            s32 joystick = inv_IPT_fn_lGetPadWithButton8Released();

            if (joystick > 0) {
                inv_IPT_fn_bSwapJoystickDevices(firstJoystick, joystick);
                fn_vResetEngineFromPaused();
            }
        }

        if (fn_bIsEngineInPause() == 0) {
            IPT_fn_vReadInput();
            IPT_fn_vReadInput();
        }

        if (firstJoystick > 0 && (u32) inv_IPT_fn_ulGetInputFrameCounter() >= 2 &&
            IPT_fn_bIsJoystickButtonPressed(0, 1) != 0) {
            fn_vResetEngineFromPaused();
            fn_vReinitTheMap();
        }
        return;
    }

    if (firstJoystick == -1 || inv_fn_ulGetEngineTrameNumber() < 0x10)
        return;
    fn_vSetEngineInPaused();
}
