#include "common.h"

#include "IPT/IPT.h"

#include "ps2/libc.h"
#include "ps2/PS2.h"

void IPT_fn_vActualizeHistoric(void)
{
    IPT_tdstHistoricElement_ record;
    stDevice* device;
    s32 sent;
    s32 i;
    s32 j;
    s32 row;

    sent = 0;
    if (IPT_g_hInputStructure.historicMode == 2) {
        for (i = 2; !sent && i < 0x12; i++) {
            row = i - 2;
            if (IPT_g_hInputStructure.deviceState[i] == 3 ||
                IPT_g_hInputStructure.deviceState[i] == 4) {
                device = IPT_g_hInputStructure.devices[i];
                INO_fn_uwJoyPreviousDirections(device);
                INO_fn_uwJoyPreviousDirections(device);
                for (j = 10; !sent && j < 0x29; j++) {
                    if ((device->m_p_stState->m_ulButtons & IPT_g_ulPadAndJoyButtonFlag[j - 10]) ==
                            IPT_g_ulPadAndJoyButtonFlag[j - 10] &&
                        (INO_fn_ulJoyPreviousButtons(device) &
                         IPT_g_ulPadAndJoyButtonFlag[j - 10]) !=
                            IPT_g_ulPadAndJoyButtonFlag[j - 10]) {
                        sent = 1;
                        record.bf2State = 2;
                        record.bf6DeviceType = i;
                        record.swDeviceValue = inv_IPT_a_wButtonKeyWordValues[j];
                        record.ucCounter = 0;
                        IPT_g_hInputStructure.ucPadAndJoyCounter[row][inv_IPT_a_wButtonKeyWordValues[j]] = 0;
                    } else if ((device->m_p_stState->m_ulButtons &
                                IPT_g_ulPadAndJoyButtonFlag[j - 10]) !=
                                   IPT_g_ulPadAndJoyButtonFlag[j - 10] &&
                               (INO_fn_ulJoyPreviousButtons(device) &
                                IPT_g_ulPadAndJoyButtonFlag[j - 10]) ==
                                   IPT_g_ulPadAndJoyButtonFlag[j - 10]) {
                        IPT_g_hInputStructure.ucPadAndJoyCounter[row][inv_IPT_a_wButtonKeyWordValues[j]] = 0;
                    }
                    {
                        s32 held = IPT_g_hInputStructure.ucPadAndJoyCounter[row][inv_IPT_a_wButtonKeyWordValues[j]] + 1;

                        if (held >= 0x100) {
                            held = 0xFF;
                        }
                        IPT_g_hInputStructure.ucPadAndJoyCounter[row][inv_IPT_a_wButtonKeyWordValues[j]] = held;
                    }
                }
            }
        }
    }
    if (sent) {
        memmove(IPT_g_hInputStructure.historic + 1, IPT_g_hInputStructure.historic,
                (IPT_g_hInputStructure.historicCount - 1) * sizeof(IPT_tdstHistoricElement_));
        IPT_g_hInputStructure.historic[0] = record;
    }
    for (i = 0; i < IPT_g_hInputStructure.historicCount; i++) {
        if (IPT_g_hInputStructure.historic[i].bf6DeviceType >= 2 &&
            IPT_g_hInputStructure.historic[i].bf6DeviceType < 0x12 &&
            IPT_g_hInputStructure.historic[i].bf2State == 2) {
            device = IPT_g_hInputStructure.devices[IPT_g_hInputStructure.historic[i].bf6DeviceType];
            if ((IPT_g_hInputStructure.historic[i].swDeviceValue == 0 &&
                 (device->m_p_stState->m_uwDirections & 4)) ||
                (IPT_g_hInputStructure.historic[i].swDeviceValue == 1 &&
                 (device->m_p_stState->m_uwDirections & 8)) ||
                (IPT_g_hInputStructure.historic[i].swDeviceValue == 2 &&
                 (device->m_p_stState->m_uwDirections & 1)) ||
                (IPT_g_hInputStructure.historic[i].swDeviceValue == 3 &&
                 (device->m_p_stState->m_uwDirections & 2)) ||
                ((u16) IPT_g_hInputStructure.historic[i].swDeviceValue >= 10 &&
                 (u16) IPT_g_hInputStructure.historic[i].swDeviceValue < 26 &&
                 (device->m_p_stState->m_ulButtons &
                  IPT_g_ulPadAndJoyButtonFlag[IPT_g_hInputStructure.historic[i].swDeviceValue -
                                              10]) ==
                     IPT_g_ulPadAndJoyButtonFlag[IPT_g_hInputStructure.historic[i].swDeviceValue -
                                                 10])) {
                s32 duration = IPT_g_hInputStructure.historic[i].ucCounter + 1;

                if (duration >= 0x100) {
                    duration = 0xFF;
                }
                IPT_g_hInputStructure.historic[i].ucCounter = duration;
            } else if (IPT_g_hInputStructure.historic[i].bf2State == 2) {
                IPT_g_hInputStructure.historic[i].bf2State = 1;
            } else if (IPT_g_hInputStructure.historic[i].bf2State == 1) {
                IPT_g_hInputStructure.historic[i].bf2State = 0;
            }
        } else if (IPT_g_hInputStructure.historic[i].bf2State == 2) {
            IPT_g_hInputStructure.historic[i].bf2State = 1;
        } else if (IPT_g_hInputStructure.historic[i].bf2State == 1) {
            IPT_g_hInputStructure.historic[i].bf2State = 0;
        }
    }
}
