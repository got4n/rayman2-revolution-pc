#include "common.h"

#include "MTH/MTH.h"
#include "snd/snd.h"

#define SND_NO_VALUE 0x7FFFFFFE

extern s32 g_lSoundSpeed;
extern s32 g_rDistSatur;
extern s32 g_rDistFade;
extern s32 g_rDistBackGround;
extern u8 ucVolumeBackGround;

void SND_fn_vSetDistanceForVolume(s32 nearDistance, s32 farDistance);

void SND_fn_vSetDistanceForVolume(s32 nearDistance, s32 farDistance)
{
    g_rDistSatur = nearDistance;
    g_rDistFade = 8 * nearDistance;
    g_rDistBackGround = farDistance;
    if (farDistance < nearDistance)
        g_rDistBackGround = 0x7FFFFFFF;
    if (g_rDistBackGround >= g_rDistFade)
        ucVolumeBackGround = 0;
    else
        ucVolumeBackGround = SND_fn_rMulRealRealSnd(
            0x7F, SND_fn_rDivRealRealSnd(g_rDistFade - g_rDistBackGround, g_rDistFade - g_rDistSatur));
}

void SND_fn_vSetDefaultRollOff(SndRollOff* rollOff)
{
    SndRollOff distances;

    if (rollOff->nearDistance != SND_NO_VALUE) {
        distances.nearDistance = rollOff->nearDistance;
    } else {
        distances.nearDistance = 0x90000;
    }
    if (rollOff->farDistance != SND_NO_VALUE) {
        distances.farDistance = rollOff->farDistance;
    } else {
        distances.farDistance = 0x480000;
    }
    SND_fn_vSetDistanceForVolume(distances.nearDistance, distances.farDistance);
}

void SND_fn_vGetDefaultRollOff(SndRollOff* rollOff)
{
    rollOff->nearDistance = g_rDistSatur;
    rollOff->farDistance = g_rDistBackGround;
}

u8 SND_fn_ucDistanceToVolume_0(s32 distance)
{
    if (distance < g_rDistSatur) {
        return 0x7F;
    }
    if (g_rDistBackGround < distance) {
        return ucVolumeBackGround;
    }
    if (g_rDistFade < distance) {
        return 0;
    }
    return SND_fn_rMulRealRealSnd(
        0x7F, SND_fn_rDivRealRealSnd(g_rDistFade - distance, g_rDistFade - g_rDistSatur));
}

u8 SND_fn_ucDistanceToVolumeEx(s32 distance, SndRollOff* rollOff)
{
    if (rollOff != NULL) {
        if (rollOff->farDistance < rollOff->nearDistance)
            rollOff->farDistance = 0x7FFFFFFF;
        if (distance < rollOff->nearDistance)
            return 0x7F;
        if (rollOff->farDistance < distance) {
            if (8 * rollOff->nearDistance < rollOff->farDistance)
                return 0;
            return SND_fn_rMulRealRealSnd(
                0x7F, SND_fn_rDivRealRealSnd(8 * rollOff->nearDistance - rollOff->farDistance,
                                             8 * rollOff->nearDistance - rollOff->nearDistance));
        }
        if (8 * rollOff->nearDistance < distance)
            return 0;
        return SND_fn_rMulRealRealSnd(
            0x7F, SND_fn_rDivRealRealSnd(8 * rollOff->nearDistance - distance,
                                         8 * rollOff->nearDistance - rollOff->nearDistance));
    }
    return SND_fn_ucDistanceToVolume_0(distance);
}

s32 SND_fn_rDopplerPitch(const SndFixedVector3* sourcePosition, const SndFixedVector3* sourceSpeed,
                         const SndFixedVector3* listenerPosition,
                         const SndFixedVector3* listenerSpeed)
{
    SndFixedVector3 delta;
    s32 norm;
    s32 base;

    if (g_lSoundSpeed != 0) {
        delta.x = (sourcePosition->x - listenerPosition->x) >> 8;
        delta.y = (sourcePosition->y - listenerPosition->y) >> 8;
        delta.z = (sourcePosition->z - listenerPosition->z) >> 8;

        norm = SND_fn_rPseudoNormeVectorSnd(&delta);
        if (norm == 0) {
            return 0x10000;
        }
        if (norm <= 0x400000) {
            base = norm * g_lSoundSpeed;
            return SND_fn_rDivRealRealSnd(base - SND_fn_rScalaireVectorSnd(sourceSpeed, &delta),
                                          base - SND_fn_rScalaireVectorSnd(listenerSpeed, &delta));
        }
    }
    return 0x10000;
}

void SND_fn_vSetDopplerFactor(s32 factor)
{
    if (factor != 0) {
        g_lSoundSpeed = SND_fn_rDivRealRealSnd(0x12C0000, factor) >> 16;
    } else {
        g_lSoundSpeed = 0;
    }
}

s32 SND_fn_rGetDopplerFactor(void)
{
    s32 factor = g_lSoundSpeed;

    if (factor != 0) {
        return SND_fn_rDivRealRealSnd(0x12C0000, factor << 16);
    }
    return 0;
}

u8 SND_fn_ucPositionToPan(const SndFixedVector3* position, const SndFixedVector3* reference,
                          const SndFixedVector3* unused, const SndFixedVector3* direction)
{
    SndFixedVector3 delta;
    s32 norm;
    s32 dot;
    s32 divisor;
    s32 pan;

    MTH3D_M_vSubLowerXYZ_inv(&delta, position, reference);
    norm = SND_fn_rPseudoNormeVectorSnd(&delta);
    dot = SND_fn_rPseudoScalaireNormeVectorSnd(&delta, direction);
    divisor = norm / 128;
    if (divisor == 0) {
        return 0x40;
    }
    if (dot <= -norm) {
        return 0;
    }
    if (dot >= norm) {
        return 0x7F;
    }
    pan = dot / 256 * 127 / divisor + 64;
    if (pan < 0) {
        pan = 0;
    }
    if (pan >= 128) {
        pan = 127;
    }
    return pan;
}

u8 SND_fn_ucPositionToDolby(const SndFixedVector3* position, const SndFixedVector3* reference,
                            const SndFixedVector3* direction)
{
    SndFixedVector3 delta;
    s32 norm;
    s32 dot;
    s32 divisor;
    s32 dolby;

    MTH3D_M_vSubLowerXYZ_inv(&delta, position, reference);
    norm = SND_fn_rPseudoNormeVectorSnd(&delta);
    dot = SND_fn_rPseudoScalaireNormeVectorSnd(&delta, direction);
    divisor = norm / 128;
    if (divisor == 0) {
        return 0x3F;
    }
    if (dot <= -norm) {
        return 0x7F;
    }
    if (dot >= norm) {
        return 0;
    }
    dolby = dot / 256 * -127 / divisor + 63;
    if (dolby < 0) {
        dolby = 0;
    }
    if (dolby >= 128) {
        dolby = 127;
    }
    return dolby;
}
