#include "common.h"
#include "snd/snd.h"

typedef struct tdstBACKMngr tdstBACKMngr;

extern tdstBACKMngr inv_SND_backManager;

s32 fn_iInitBACKEnv(void)
{
    return fn_iConstructTdstBACKMngr(&inv_SND_backManager);
}

s32 fn_iDesInitBACKEnv(void)
{
    return fn_iDestructTdstBACKMngr(&inv_SND_backManager);
}

s32 fn_iGetNumFreeVoiceBACKEnv(void)
{
    return fn_iGetNumFreeVoiceTdstBACKMngr(&inv_SND_backManager);
}

s32 fn_u32AllocateVoiceBACKEnv(tdstBACKCall* source)
{
    return fn_u32AllocateVoiceTdstBACKMngr(&inv_SND_backManager, source);
}

s32 fn_u32DesallocateVoiceBACKEnv(s32 voice)
{
    return fn_u32DesallocateVoiceTdstBACKMngr(&inv_SND_backManager, (tdstBACKData*) voice);
}

s32 fn_iOperateCallTdstBACKEnv(s32 voice)
{
    return fn_iOperateCallTdstBACKMngr(&inv_SND_backManager, (tdstBACKData*) voice);
}

s32 fn_iOperateCall(s32* output, s32 outputSize, s32* input, s32 inputSize)
{
    s32 result = -1;

    if (inputSize == 4)
        result = fn_iOperateCallTdstBACKEnv(input[0]);
    return result;
}

extern SndCmdHandler inv_SND_a_pfnBackEnvHandlers[2];

s32 fn_iGetEEAdpcmInterface(SndCmdModule* module)
{
    inv_SND_a_pfnBackEnvHandlers[0] = fn_iOperateCall;
    module->tag = 0x6B636162;
    module->id = 3;
    module->lastCommand = 1;
    module->handlers = inv_SND_a_pfnBackEnvHandlers;
    return 0;
}
