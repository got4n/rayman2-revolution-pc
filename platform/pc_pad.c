#include <string.h>

#include "PC.h"
#include "pc_host.h"

int32_t pc_pad_read(int32_t port, PcPad *pad)
{
    uint8_t sticks[4];

    memset(pad, 0, sizeof *pad);
    pad->leftX = pad->leftY = pad->rightX = pad->rightY = PC_PAD_AXIS_CENTER;
    if (port != 0) {
        return 0;
    }

    pad->buttons = (uint16_t) ~revo_pad_buttons();
    revo_pad_sticks(sticks);
    pad->rightX = sticks[0];
    pad->rightY = sticks[1];
    pad->leftX = sticks[2];
    pad->leftY = sticks[3];
    return 1;
}
