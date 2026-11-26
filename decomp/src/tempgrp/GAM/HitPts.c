#include "common.h"

#include "GAM/GAM.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

u8 fn_ucGetHitPoints(tdstEngineObject_* _p_stObject)
{
    return _p_stObject->h_StandardGame->ucHitPoints;
}

void fn_vSetHitPointsInit(tdstEngineObject_* _p_stObject, u8 _ucValue)
{
    _p_stObject->h_StandardGame->ucSaveHitPoints = _ucValue;
}

void fn_vSetHitPoints(tdstEngineObject_* _p_stObject, u8 _ucValue)
{
    _p_stObject->h_StandardGame->ucHitPoints = _ucValue;
}

void fn_vSetHitPointsToMaxValue(tdstEngineObject_* _p_stObject)
{
    gu_tdstStandardGame* standardGame = _p_stObject->h_StandardGame;

    standardGame->ucHitPoints = standardGame->ucHitPointsMax;
}

static inline void inv_GAM_fn_vHitPointsAddClamped(tdstEngineObject_* object, s32 amount, s32 maximumLevel)
{
    gu_tdstStandardGame* standardGame = object->h_StandardGame;
    s32 increment = amount & 0xFF;
    s32 current;
    s32 maximum;
    s32 exceedsMaximum;

    current = maximumLevel ? standardGame->ucHitPointsMax : standardGame->ucHitPoints;
    maximum = maximumLevel ? standardGame->ucHitPointsMaxMax : standardGame->ucHitPointsMax;
    exceedsMaximum = current + increment;
    exceedsMaximum = maximum < exceedsMaximum;
    increment = *&increment;
    if (!exceedsMaximum) {
        if (maximumLevel)
            standardGame->ucHitPointsMax = (u8) (increment + current);
        else
            standardGame->ucHitPoints = (u8) (increment + current);
        return;
    }
    if (maximumLevel)
        standardGame->ucHitPointsMax = (u8) maximum;
    else
        standardGame->ucHitPoints = (u8) maximum;
}

void fn_vAddHitPoints(tdstEngineObject_* _p_stObject, s32 _ucValue)
{
    inv_GAM_fn_vHitPointsAddClamped(_p_stObject, _ucValue, 0);
}

u8 fn_ucSubHitPoints(tdstEngineObject_* _p_stObject, u8 _ucValue)
{
    gu_tdstStandardGame* standardGame = _p_stObject->h_StandardGame;
    u8 hitPoints = standardGame->ucHitPoints;

#ifdef PC_PORT
    if (pc_mod_no_damage((PcAddress) _p_stObject)) {
        return hitPoints;
    }
#endif
    if (hitPoints - _ucValue > 0)
        standardGame->ucHitPoints = -_ucValue + hitPoints;
    else
        standardGame->ucHitPoints = 0;
    return _p_stObject->h_StandardGame->ucHitPoints;
}

u8 fn_ucGetHitPointsMax(tdstEngineObject_* _p_stObject)
{
    return _p_stObject->h_StandardGame->ucHitPointsMax;
}

void fn_vSetHitPointsMax(tdstEngineObject_* _p_stObject, u8 _ucValue)
{
    _p_stObject->h_StandardGame->ucHitPointsMax = _ucValue;
}

void fn_vSetHitPointsMaxToMaxValue(tdstEngineObject_* _p_stObject)
{
    gu_tdstStandardGame* standardGame = _p_stObject->h_StandardGame;

    standardGame->ucHitPointsMax = standardGame->ucHitPointsMaxMax;
}

void fn_vAddHitPointsMax(tdstEngineObject_* _p_stObject, s32 _ucValue)
{
    inv_GAM_fn_vHitPointsAddClamped(_p_stObject, _ucValue, 1);
}

u8 fn_ucSubHitPointsMax(tdstEngineObject_* _p_stObject, u8 _ucValue)
{
    u8 hitPointsMax = _p_stObject->h_StandardGame->ucHitPointsMax;

    if (hitPointsMax - _ucValue > 0)
        _p_stObject->h_StandardGame->ucHitPointsMax = -_ucValue + hitPointsMax;
    else
        _p_stObject->h_StandardGame->ucHitPointsMax = 0;
    fn_vAddHitPoints(_p_stObject, 0);
    return _p_stObject->h_StandardGame->ucHitPointsMax;
}
