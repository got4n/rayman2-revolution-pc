#include "common.h"

#include "MTH/MTH.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"

#include "ps2/libc.h"
#include "AI/AI.h"

#define SET_BRAIN_FREQUENCY(data, frequency)                                                       \
    do {                                                                                           \
        (data)->ucBrainComputationFrequency = (frequency);                                           \
    } while (0)

gu_tdst3dData_* fn_h_3dDataRealAlloc(tdstEngineObject_* object);
void fn_v3dDataInitValueSAI(gu_tdst3dData_* data);

static inline s16 inv_GAM_fn_wRandomBrainDelay(void)
{
    return (s32) (rand() * (17.0f / RAND_RANGE_inv)) + 1;
}

void fn_v3dDataAlloc(tdstEngineObject_* p_stObject)
{
    p_stObject->h_3dData = fn_h_3dDataRealAlloc(p_stObject);
}

gu_tdst3dData_* fn_h_3dDataRealAlloc(tdstEngineObject_* object)
{
    gu_tdst3dData_* data = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(gu_tdst3dData_));
    u16 allBits;

    gu_MTH4D_M_vSetIdentityMatrix(&data->stGLIObjectMatrix);
    data->ucFlagEndState = 0;
    data->ucFlagEndOfAnim = 0;
    data->ucFlagModifState = 0;
    data->sForcedFrame = 0;
    data->stFrame3d.p_stArrayOfElts3d = NULL;
    data->wSizeOfArrayOfElts3d = 0;
    data->hMorphChannelList = NULL;
    data->h_StateInLastFrame = 0;
    data->h_WantedState = 0;
    data->h_CurrentState = NULL;
    data->h_CurrentObjectsTable = NULL;
    data->h_InitialObjectsTable = 0;
    data->ulStartTime = inv_fn_ulGetEngineClock();
    data->bSkipCurrentFrame = 0;
    data->sLastFrame = -1;
    data->ulTimeDelay = 0;
    data->bStateJustModified = 0;
    data->ucFrameRate = 0;
    allBits = 0xFFFF;
    data->uwNbEngineFrameSinceLastMechEvent = allBits;
    data->lDrawMask = -1;
    data->lDrawMaskInit = -1;
    memset(&data->shadow, 0, sizeof(data->shadow));
    fn_v3dDataSetBrainComputationFrequency(data, 1);
    data->cBrainCounter = -1;
    data->uwBrainMainCounter = inv_GAM_fn_wRandomBrainDelay();
    data->ucTransparency = 0xFF;
    return data;
}

void fn_v3dDataInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    s32 mode = eObjectInit;
    POS_tdstMatrix_* initialMatrix;
    HIE_tdstSuperObject_* superObject;
    tdstObjectsTablesList_* table;

    if (mode == 3 || mode == 4) {
        initialMatrix = SAI_fn_p_vGetTableValuePointerOf(&p_stObject->h_3dData->stGLIObjectMatrix, 0x32);
        p_stObject->h_3dData->stGLIObjectMatrix = *initialMatrix;
        superObject = inv_fn_hGetOwnerSuperObject(p_stObject);
        if (superObject != NULL && superObject->hFatherSta != NULL) {
            _POS_fn_vMulMatrixMatrix(superObject->gu_hGlobalMatrix,
                                     superObject->hFatherSta->gu_hGlobalMatrix,
                                     &p_stObject->h_3dData->stGLIObjectMatrix);
        }
    }

    if (inv_fn_hGetOwnerSuperObject(p_stObject) != NULL) {
        inv_fn_hGetOwnerSuperObject(p_stObject)->gu_fTransparenceLevel = p_stObject->h_3dData->ucTransparency;
    }

    switch (mode) {
    case 1:
        fn_v3dDataInitValueSAI(p_stObject->h_3dData);
        p_stObject->h_3dData->hArrayOfChannels = NULL;
        p_stObject->h_3dData->ulNumberOfChannels = p_stObject->h_3dData->h_Family->ulNumberOfChannels;
    case 2:
    case 3:
    case 4:
        p_stObject->h_3dData->bAnimMatrixChanged = 1;
        fn_vTakeControlOfChannel(p_stObject, 0xFFFF, 0);
        p_stObject->h_3dData->ulStartTime = 0;
        p_stObject->h_3dData->ulTimeDelay = 0;
        p_stObject->h_3dData->ulTimePreviousFrame = 0;
        PLA_fn_vDesInitAllChildOfCharacter(inv_fn_hGetOwnerSuperObject(p_stObject), p_stObject);
        memset(&p_stObject->h_3dData->stFrame3d, 0, sizeof(tdstFrame3d_));
        p_stObject->h_3dData->p_stCurrentHieCouples = 0;
        p_stObject->h_3dData->wCurrentHieNbCouples = 0;
        p_stObject->h_3dData->wSizeOfArrayOfElts3d = 0;
        p_stObject->h_3dData->lLastComputeFrame = 0;
        p_stObject->h_3dData->bStateJustModified = 0;
        fn_v3dDataSetFrameRate(p_stObject->h_3dData, 0);
        p_stObject->h_3dData->h_CurrentState = NULL;
        p_stObject->h_3dData->stFrame3d.p_stArrayOfElts3d = NULL;
        p_stObject->h_3dData->stFrame3d.p_stAnim = 0;
        p_stObject->h_3dData->lDrawMask = p_stObject->h_3dData->lDrawMaskInit;
        p_stObject->h_3dData->cBrainCounter = -1;
        p_stObject->h_3dData->bSkipCurrentFrame = 0;
    case 5:
        fn_vInitMorphList(&p_stObject->h_3dData->hMorphChannelList);
        table = p_stObject->h_3dData->h_InitialObjectsTable;
        if (table != 0 && fn_d_hGetInitObjectsTable(table) != 0) {
            fn_vReInitObjectTable(table);
        }
        p_stObject->h_3dData->uwBrainMainCounter = inv_GAM_fn_wRandomBrainDelay();
        fn_v3dDataSetBrainComputationFrequency(p_stObject->h_3dData, 1);
        break;
    case 0:
        break;
    }

    if (inv_fn_hGetOwnerSuperObject(p_stObject) != NULL) {
        PLA_fn_vInitNewState(inv_fn_hGetOwnerSuperObject(p_stObject), p_stObject->h_3dData->h_InitialState);
        inv_fn_hGetOwnerSuperObject(p_stObject)->gu_lDrawModeMask = p_stObject->h_3dData->lDrawMask;
    }
}

void fn_v3dDataDesinit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    s32 mode = eObjectInit;

    if (mode == 2) {
        return;
    }
    if (mode >= 3) {
        return;
    }
    if (mode != 1) {
        return;
    }
    PLA_fn_vDesInitAllChildOfCharacter(inv_fn_hGetOwnerSuperObject(p_stObject), p_stObject);
    fn_vFreeMorphList(&p_stObject->h_3dData->hMorphChannelList);
}

void fn_v3dDataInitValueSAI(gu_tdst3dData_* h_3dData)
{
}

void fn_v3dDataCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    gu_tdst3dData_* destination = _p_stObjectDest->h_3dData;
    gu_tdst3dData_* source = _p_stObjectSrc->h_3dData;
    tdstAChannel_* csoRecords = destination->hArrayOfChannels;
    tdstElement3d_* elements = destination->stFrame3d.p_stArrayOfElts3d;
    tdstAChannel_* activeCsoHead = destination->hFirstActiveChannel;
    LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* morphList = destination->hMorphChannelList;
    u16 elementCapacity = destination->wSizeOfArrayOfElts3d;

    *destination = *source;
    _p_stObjectDest->h_3dData->hArrayOfChannels = csoRecords;
    _p_stObjectDest->h_3dData->stFrame3d.p_stArrayOfElts3d = elements;
    _p_stObjectDest->h_3dData->hFirstActiveChannel = activeCsoHead;
    _p_stObjectDest->h_3dData->hMorphChannelList = morphList;
    _p_stObjectDest->h_3dData->wSizeOfArrayOfElts3d = elementCapacity;
}

void fn_v3dDataSetFrameRate(gu_tdst3dData_* h_3dData, s32 ucFrameRate)
{
    u32 frameRate = ucFrameRate & 0xFF;
    u32 elapsed;
    u32 now;

    if (frameRate >= 2) {
        elapsed = (inv_fn_ulGetEngineClock() - h_3dData->ulStartTime) * h_3dData->ucFrameRate;
        now = inv_fn_ulGetEngineClock();
        h_3dData->ulStartTime = now - elapsed / frameRate;
    }
    h_3dData->ucFrameRate = frameRate;
}

void fn_v3dDataFactorFrameRate(gu_tdst3dData_* _h3dData, s32 _ulFactor100)
{
    u8 frameRate = (u32) (_h3dData->h_CurrentState->scSpeedAnim * _ulFactor100) / 100;
    u32 elapsed;
    u32 now;

    if (frameRate >= 2) {
        elapsed = (inv_fn_ulGetEngineClock() - _h3dData->ulStartTime) * _h3dData->ucFrameRate;
        now = inv_fn_ulGetEngineClock();
        elapsed /= frameRate;
        _h3dData->ucFrameRate = frameRate;
        _h3dData->ulStartTime = now - elapsed;
    }
}

void fn_v3dDataSetBrainComputationFrequency(gu_tdst3dData_* h_3dData, s32 _ucFrequency)
{
    SET_BRAIN_FREQUENCY(h_3dData, _ucFrequency);
    if (h_3dData->cBrainCounter >= (_ucFrequency & 0xFF)) {
        h_3dData->cBrainCounter = 0;
        SET_BRAIN_FREQUENCY(h_3dData, _ucFrequency);
    }
}

void fn_v3dDataUpdateBrainCounter(HIE_tdstSuperObject_* _hCharacter)
{
    tdstEngineObject_* object;
    gu_tdst3dData_* data;

    if (_hCharacter == NULL) {
        return;
    }
    object = _hCharacter->hLinkedObject;
    data = object->h_3dData;
    if (data == NULL) {
        return;
    }
    data->cBrainCounter++;
    if (data->cBrainCounter < data->ucBrainComputationFrequency) {
        return;
    }
    data->cBrainCounter = 0;
}

s32 fn_b3dDataCanComputeBrain(HIE_tdstSuperObject_* _hCharacter)
{
    tdstEngineObject_* object;
    gu_tdst3dData_* data;

    if (_hCharacter != NULL) {
        object = _hCharacter->hLinkedObject;
        data = object->h_3dData;
        if (data != NULL) {
            return data->cBrainCounter == 0;
        }
    }
    return 1;
}

s32 fn_b3dDataGetUserEventFlag(gu_tdst3dData_* h_3dData, s32 _ucFlagIndex, MTH3D_tdstVector_* _p_stPosition)
{
    u8 ucMask = 1 << _ucFlagIndex;

    if (_p_stPosition != NULL) {
        *_p_stPosition = h_3dData->stLastEventGlobalPosition;
    }
    return h_3dData->ucUserEventFlags & ucMask;
}

void fn_v3dDataSetUserEventFlag(gu_tdst3dData_* h_3dData, u32 _ucFlagIndex, s32 _bSet,
                                MTH3D_tdstVector_* _p_stPosition)
{
    u8 ucMask = 1 << _ucFlagIndex;

    if (_bSet) {
        h_3dData->ucUserEventFlags |= ucMask;
    } else {
        h_3dData->ucUserEventFlags &= ~ucMask;
    }
    if (_p_stPosition != NULL) {
        h_3dData->stLastEventGlobalPosition = *_p_stPosition;
    }
}

void fn_vAllocArrayOfElts3d(gu_tdst3dData_* h_3dData, u32 _wNumberOfElts3d)
{
    u16 capacity = _wNumberOfElts3d;

    if ((u16) h_3dData->wSizeOfArrayOfElts3d < capacity) {
        if (h_3dData->stFrame3d.p_stArrayOfElts3d != NULL) {
            inv_MEM_fn_vFreeInHeap(gp_6F74, h_3dData->stFrame3d.p_stArrayOfElts3d);
            h_3dData->stFrame3d.p_stArrayOfElts3d = NULL;
        }
        h_3dData->stFrame3d.p_stArrayOfElts3d = AI_fn_p_vTrueAlloc(gp_6F74, capacity * sizeof(tdstElement3d_));
        h_3dData->wSizeOfArrayOfElts3d = capacity;
    }
}

void fn_vFreeArrayOfElts3d(gu_tdst3dData_* h_3dData)
{
    if (h_3dData->stFrame3d.p_stArrayOfElts3d != NULL) {
        inv_MEM_fn_vFreeInHeap(gp_6F74, h_3dData->stFrame3d.p_stArrayOfElts3d);
        h_3dData->wSizeOfArrayOfElts3d = 0;
    } else {
        h_3dData->wSizeOfArrayOfElts3d = 0;
    }
    h_3dData->stFrame3d.p_stArrayOfElts3d = NULL;
}
