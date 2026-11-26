#include "common.h"

#include "GAM/GAM.h"

void fn_vSetCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities)
{
    _p_stObject->h_StandardGame->ubf32Capabilities = _ubf32Capabilities;
}

u32 fn_ulGetCapabilities(tdstEngineObject_* _p_stObject)
{
    return _p_stObject->h_StandardGame->ubf32Capabilities;
}

void fn_vAddCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities)
{
    _p_stObject->h_StandardGame->ubf32Capabilities |= _ubf32Capabilities;
}

void fn_vSubCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities)
{
    gu_tdstStandardGame* p_stStandardGame = _p_stObject->h_StandardGame;

    p_stStandardGame->ubf32Capabilities -= p_stStandardGame->ubf32Capabilities & _ubf32Capabilities;
}
