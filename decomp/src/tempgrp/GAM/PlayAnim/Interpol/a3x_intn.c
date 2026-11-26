#include "common.h"

#include "GAM/GAM.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#ifdef PC_PORT
f32 fabsf(f32 value);
#endif
#include "AI/AI.h"

#define NO_INDEX 0xFFFF
#define ORIENTATION_MATRIX_SIZE 0x24

typedef struct PackedQuaternion {
    s16 component[4];
} PackedQuaternion;

typedef struct Quaternion {
    f32 component[4];
} Quaternion;

typedef struct AnimationKey {
    MTH3D_tdstVector_ stPos;
    f32 xScale;
    u16 uwFrame;
    u16 uwMask;
    u16 uwNumOfQuatOri;
    u16 uwNumOfQuatSca;
    u16 uwNumOfScale;
    u16 uwNumOfPosition;
    s16 swOriParam;
    s16 swScaParam;
    s16 swCurve;
    s16 swEase;
} AnimationKey;

typedef struct AnimationChannel {
    u16 inv_uwKeyCount;
    u16 bone;
    u16 uwNumOfLocalPivotPos;
    u16 firstRemap;
    u16 firstKeyIndex;
    u16 inv_uwFirstKey;
} AnimationChannel;

typedef struct AnimationFrame {
    u16 orientation;
    u16 vector;
    u16 linkCount;
    u16 firstLink;
    u16 remapBase;
} AnimationFrame;

typedef struct AnimationRecord {
    u8 weight;
    u8 unk01;
    u16 key;
    u8 value;
    u8 unk05;
} AnimationRecord;

typedef struct AnimationChannelOutput {
    u16 key : 15;
    u16 active : 1;
    u8 type;
    u8 weight;
    u8 value;
    u8 unk05;
} AnimationChannelOutput;

typedef union ScaleTransform {
    POS_tdstMatrix_ matrix;
    MTH3D_tdstVector_ translation;
} ScaleTransform;

void fn_v_QuatToMatrix(MTH3D_tdstMatrix_* matrix, f32* quaternion);
void inv_POS_fn_vScale(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* scale);

void fn_v_InterpolBetweenKeys(AnimationData* data, AnimationKey* k1, AnimationKey* k2,
                              AnimationChannel* channel, POS_tdstMatrix_* res, u16 mode, f32 xT)
{
    MTH3D_tdstVector_ stTranslation;
    MTH3D_tdstVector_ stTranslation2;
    Quaternion q;
    Quaternion q1;
    Quaternion q2;
    MTH3D_tdstVector_ sca;
    ScaleTransform v;
    MTH3D_tdstVector_* p_stPosition1;
    f32 a4_xQuatS1;
    f32 a4_xQuatS2;

    if (xT == 0.0f || (k1->uwMask & 1)) {
        if (!(k1->uwMask & 0x10)) {
            fn_v_ExpandQuat(q.component, data->quaternions[k1->uwNumOfQuatOri].component);
            POS_fn_vInitTransformationWithQuaternion(res, q.component);
        } else {
            gu_MTH4D_M_vSetIdentityMatrix(res);
        }
    } else {
        fn_v_ExpandQuat(q1.component, data->quaternions[k1->uwNumOfQuatOri].component);
        fn_v_ExpandQuat(q2.component, data->quaternions[k2->uwNumOfQuatOri].component);
        fn_v_InterpolQuatWithOmega(q.component, q1.component, q2.component, k1->swOriParam, xT);
        POS_fn_vInitTransformationWithQuaternion(res, q.component);
    }

    if ((k1->uwMask & 0x44) != 0x44 && (xT != 0.0f || !(k1->uwMask & 0x40))) {
        MTH3D_M_vSetXYZ_inv(
            &sca,
            data->vectors[k1->uwNumOfScale].xX +
                xT * (data->vectors[k2->uwNumOfScale].xX - data->vectors[k1->uwNumOfScale].xX),
            data->vectors[k1->uwNumOfScale].xY +
                xT * (data->vectors[k2->uwNumOfScale].xY - data->vectors[k1->uwNumOfScale].xY),
            data->vectors[k1->uwNumOfScale].xZ +
                xT * (data->vectors[k2->uwNumOfScale].xZ - data->vectors[k1->uwNumOfScale].xZ));
        if (xT == 0.0f || (k1->uwMask & 2)) {
            if (k1->uwMask & 0x20) {
                inv_POS_fn_vScale(res, &sca);
            } else {
                fn_v_ExpandQuat(q.component, data->quaternions[k1->uwNumOfQuatSca].component);
                a4_xQuatS2 = sca.xX;
                if (fabsf(a4_xQuatS2 - sca.xY) < 0.01f && fabsf(a4_xQuatS2 - sca.xZ) < 0.01f) {
                    inv_POS_fn_vZoom(res, a4_xQuatS2);
                } else {
                    POS_fn_vInitTransformationWithNonDiagonalScale(&v.matrix, &sca, q.component);
                    _POS_fn_vMulMatrixMatrix(res, res, &v.matrix);
                }
            }
        } else {
            fn_v_ExpandQuat(q1.component, data->quaternions[k1->uwNumOfQuatSca].component);
            fn_v_ExpandQuat(q2.component, data->quaternions[k2->uwNumOfQuatSca].component);
            fn_v_InterpolQuatWithOmega(q.component, q1.component, q2.component, k1->swScaParam, xT);
            a4_xQuatS2 = sca.xX;
            if (fabsf(a4_xQuatS2 - sca.xY) < 0.01f && fabsf(a4_xQuatS2 - sca.xZ) < 0.01f) {
                inv_POS_fn_vZoom(res, a4_xQuatS2);
            } else {
                POS_fn_vInitTransformationWithNonDiagonalScale(&v.matrix, &sca, q.component);
                _POS_fn_vMulMatrixMatrix(res, res, &v.matrix);
            }
        }
    }

    if (xT == 0.0f || (k1->uwMask & 8)) {
        f32 xs;

        p_stPosition1 = &data->vectors[k1->uwNumOfPosition];
        xs = k1->xScale;
        inv_MTH3D_fn_vKTimesXYZ(&stTranslation, xs, p_stPosition1);
        inv_POS_fn_vSetTranslation(res, &stTranslation);
    } else if (mode == 0) {
        POS_fn_vApplyMatrixToVector(&sca, res, &data->vectors[channel->uwNumOfLocalPivotPos]);
        MTH3D_M_vSetXYZ_inv(&stTranslation, k1->stPos.xX + xT * (k2->stPos.xX - k1->stPos.xX),
                            k1->stPos.xY + xT * (k2->stPos.xY - k1->stPos.xY),
                            k1->stPos.xZ + xT * (k2->stPos.xZ - k1->stPos.xZ));
        inv_MTH3D_fn_vSubXYZ(&v.translation, &stTranslation, &sca);
        inv_POS_fn_vSetTranslation(res, &v.translation);
    } else {
        f32 xScale1;
        f32 xScale2;

        xScale1 = k1->xScale;

        if (1e-07f < xScale1 || 1e-07f < k2->xScale) {
            xScale2 = k2->xScale;
            p_stPosition1 = &data->vectors[k1->uwNumOfPosition];
            if (k1->uwMask & 0x100) {
                a4_xQuatS1 = (xScale2 - xScale1) * xT + xScale1;
                inv_MTH3D_fn_vKTimesXYZ(&stTranslation, a4_xQuatS1, p_stPosition1);
                inv_POS_fn_vSetTranslation(res, &stTranslation);
            } else {
                MTH3D_tdstVector_* p_stPosition2 = &data->vectors[k2->uwNumOfPosition];
                s16 n = k1->swCurve;
                f32 a;

                if (ax_SinTab[n] != 0) {
                    s32 i;
                    s16 j;

                    a4_xQuatS2 = 1.0f / (f32) ax_SinTab[n];
                    i = inv_MTH_fn_lToIntegerOrdered(xT * (f32) n);
                    j = i;
                    a = ax_SinTab[j] * a4_xQuatS2;
                    a4_xQuatS1 = ax_SinTab[n - j] * a4_xQuatS2;
                } else {
                    a4_xQuatS1 = 1.0f - xT;
                    a = xT;
                }
                a4_xQuatS2 = (xScale2 - xScale1) * xT + xScale1;
                a4_xQuatS1 *= a4_xQuatS2;
                a *= a4_xQuatS2;
                inv_MTH3D_fn_vKTimesXYZ(&stTranslation2, a4_xQuatS1, p_stPosition1);
                inv_MTH3D_fn_vKTimesXYZ(&stTranslation, a, p_stPosition2);
                inv_MTH3D_fn_vAddXYZ(&stTranslation, &stTranslation, &stTranslation2);
                inv_POS_fn_vSetTranslation(res, &stTranslation);
            }
        }
    }
}

tdstEventInTable_* fn_p_GetEventInTable(gu_tdst3dData_* owner, tdstEvent_* key)
{
    u32 index;
    tdstObjectsTablesList_* table;
    tdstEventInTable_* event;

    index = key->usEventNumberInTBL;
    table = owner->h_CurrentObjectsTable;
    event = (tdstEventInTable_*) table->d_stObjectsTable[index].stElement.h_Target;
    if (event != NULL && index < table->wNumberOfElement) {
        return event;
    }
    return (tdstEventInTable_*) owner->h_Family->h_DefaultObjectsTable->d_stObjectsTable[index].stElement.h_Target;
}

void func_001103C8(tdstAnim3d_* animation)
{
    AnimationData* data;

    data = animation->p_stData;
    animation->uwNumberOfFrames = data->frameCount;
    animation->ucFrameRate = data->inv_ucFrameRate;
    animation->ucMaxNumberOfElements = data->channelCount;
    animation->ucNumberOfEvents = data->eventCount;

    if (!MTH3D_M_bIsZeroXYZ_inv(&data->vectors[data->offsetVectorIndex])) {
        animation->stOffsetMatrix = data->vectors[data->offsetVectorIndex];
    } else {
        memset(&animation->stOffsetMatrix, 0, sizeof(animation->stOffsetMatrix));
    }

    if (animation->ucNumberOfEvents == 0) {
        animation->d_stAnimEvent = NULL;
    } else {
        animation->d_stAnimEvent = data->events;
    }

    if (data->morphCount == 0) {
        animation->d_stMorphDataArray = NULL;
    } else {
        animation->d_stMorphDataArray = data->morphData;
    }
}

void fn_vUpdateFrame(void* _p_stSuperObject, tdstFrame3d_* _p_stFrame, tdstAnim3d_* _p_stAnim3d,
                     u16 _uwFrame)
{
    AnimationData* data;
    AnimationFrame* entry;
    AnimationChannel* channel;
    AnimationChannelOutput* output;
    u16* remap;
    u16 remapBase;
    Quaternion quaternion;
    s32 i;

    if (_uwFrame >= _p_stAnim3d->uwNumberOfFrames) {
        return;
    }
    if (_p_stFrame->p_stAnim == _p_stAnim3d && _p_stFrame->uwFrameNumber == _uwFrame) {
        return;
    }
    _p_stFrame->p_stAnim = _p_stAnim3d;
    _p_stFrame->uwFrameNumber = _uwFrame;
    _p_stFrame->ucMatrixUpdated = 0;
    data = _p_stAnim3d->p_stData;
    channel = data->channels;
    entry = &data->frames[_uwFrame];
    remapBase = entry->remapBase;
    if (entry->orientation == NO_INDEX) {
        if (_p_stFrame->p_stAngularSpeedMatrix != NULL) {
            inv_MEM_fn_vFreeInHeap(gp_6F74, _p_stFrame->p_stAngularSpeedMatrix);
        }
        _p_stFrame->p_stAngularSpeedMatrix = NULL;
    } else {
        if (_p_stFrame->p_stAngularSpeedMatrix == NULL) {
            _p_stFrame->p_stAngularSpeedMatrix = AI_fn_p_vTrueAlloc(gp_6F74, ORIENTATION_MATRIX_SIZE);
        }
        fn_v_ExpandQuat(quaternion.component, data->quaternions[entry->orientation].component);
        fn_v_QuatToMatrix(_p_stFrame->p_stAngularSpeedMatrix, quaternion.component);
    }
    if (entry->vector == NO_INDEX) {
        _p_stFrame->p_stLinearSpeedVector = NULL;
    } else {
        _p_stFrame->p_stLinearSpeedVector = &data->vectors[entry->vector];
    }
    if (entry->linkCount != 0) {
        _p_stFrame->stHierarchy.d_stCouples = &data->links[entry->firstLink];
    } else {
        _p_stFrame->stHierarchy.d_stCouples = NULL;
    }
    _p_stFrame->stHierarchy.ulNbOfCouples = entry->linkCount;
    output = (AnimationChannelOutput*) _p_stFrame->p_stArrayOfElts3d;
    for (i = 0; i < data->channelCount; i++, channel++, output++) {
        remap = &data->remap[channel->firstRemap + remapBase];
        if (*remap != NO_INDEX) {
            output->key = data->records[*remap].key;
            output->type = channel->bone;
            output->active = 1;
            output->weight = data->records[*remap].weight;
            output->value = data->records[*remap].value;
        } else {
            output->key = 0x7FFF;
            output->type = channel->bone;
            output->active = 1;
            output->weight = 2;
            output->value = 0;
        }
    }
}

void fn_vUpdateFrameMatrix(void* superObject, tdstFrame3d_* cache,
                           tdstAnim3d_* animation, u16 frame, u8 force, tdstAChannel_* bones)
{
    AnimationData* data;
    AnimationChannel* channel;
    AnimationKey* keyA;
    AnimationKey* keyB;
    POS_tdstMatrix_* matrix;
    u16* remap;
    u16 remapBase;
    u16 bone;
    f32 ratio;
    f32 ease;
    s32 i;
    AnimationKey* kA;
    AnimationKey* kB;
    u16* keyIndex;
    f32 unused[8];

    fn_vUpdateFrame(superObject, cache, animation, frame);
    if (cache->ucMatrixUpdated != 0 && force == 0) {
        return;
    }
    cache->ucMatrixUpdated = 1;
    data = animation->p_stData;
    channel = data->channels;
    remapBase = data->frames[frame].remapBase;
    for (i = 0; i < data->channelCount; i++, channel++) {
        ratio = 0.0f;
        if (channel->bone != NO_INDEX && bones[bone = channel->bone].bControlledChannel == 0) {
            keyIndex = &data->keyIndices[channel->firstKeyIndex + frame];
            keyA = &data->keys[*keyIndex];
            remap = &data->remap[channel->firstRemap + remapBase];
            if (keyA->uwMask & 0x80) {
                if (keyA->uwMask & 0x200) {
                    keyB = &data->keys[data->keyIndices[channel->firstKeyIndex]];
                    ratio = (f32) (frame - keyA->uwFrame) /
                            (f32) (data->frameCount + keyB->uwFrame - keyA->uwFrame);
                } else {
                    keyB = keyA;
                }
            } else {
                keyB = keyA + 1;
                if (keyB->uwFrame != keyA->uwFrame) {
                    ratio = (f32) (frame - keyA->uwFrame) / (f32) (keyB->uwFrame - keyA->uwFrame);
                }
            }
            if (keyA->swEase != 0 && ratio != 0.0f) {
                ease = (f32) keyB->swEase * (1.0f / 8192.0f);
                ratio = ratio * ((ratio * ease + 1.0f) - ease);
            }
            matrix = bones[channel->bone].hSupObject->gu_hLocalMatrix;
            if (matrix == NULL) {
                ease = ratio;
            }
            if (channel == NULL) {
                kA = keyA;
            } else {
                kA = keyA;
            }
            if (channel == NULL) {
                kB = keyB;
            } else {
                kB = keyB;
            }
            if (matrix == NULL) {
                if (remap == NULL) {
                    kB = keyB;
                } else {
                    kB = keyB;
                }
            }
            fn_v_InterpolBetweenKeys(data, kA, kB, channel, matrix, 0, ratio);
        }
    }
}

MTH3D_tdstVector_* fn_p_stGetLinearSpeedVector(tdstAnim3d_* _p_stAnim3d, u16 _uwFrame)
{
    AnimationData* data;
    AnimationFrame* entry;

    if (_uwFrame >= _p_stAnim3d->uwNumberOfFrames) {
        return NULL;
    }
    data = _p_stAnim3d->p_stData;
    entry = &data->frames[_uwFrame];
    if (entry->vector == NO_INDEX) {
        return NULL;
    }
    return &data->vectors[entry->vector];
}

MTH3D_tdstMatrix_* fn_p_stGetAngularSpeedMatrix(tdstAnim3d_* _p_stAnim3d, u16 _uwFrame,
                                 MTH3D_tdstMatrix_* p_stMatrix)
{
    Quaternion quaternion;
    AnimationData* data;
    AnimationFrame* entry;

    if (_uwFrame >= _p_stAnim3d->uwNumberOfFrames) {
        return NULL;
    }
    data = _p_stAnim3d->p_stData;
    entry = &data->frames[_uwFrame];
    if (entry->orientation == NO_INDEX) {
        return NULL;
    }
    fn_v_ExpandQuat(quaternion.component, data->quaternions[entry->orientation].component);
    fn_v_QuatToMatrix(p_stMatrix, quaternion.component);
    return p_stMatrix;
}
