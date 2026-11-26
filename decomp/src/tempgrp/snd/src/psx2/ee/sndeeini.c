#include "common.h"

#include "ps2/libc.h"
#include "snd/snd.h"

extern SndCmdModule inv_SND_a_stCmdModules[8];

s32 fn_iInitEECmdModule(void)
{
    SndCmdModule* modules = inv_SND_a_stCmdModules;
    s32 result = -1;

    memset(modules, 0, sizeof(inv_SND_a_stCmdModules));
    if (fn_iGetEEAdpcmInterface(modules) >= 0 && fn_iInitServerFunc(modules, &modules[8]) >= 0)
        result = 0;
    return result;
}
