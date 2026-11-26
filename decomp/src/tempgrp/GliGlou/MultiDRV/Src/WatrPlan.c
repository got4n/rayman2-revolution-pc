#include "common.h"

#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"

extern f32 GLI_gsWaterPlaneRefraction;
extern f32 GLI_gsWaterPlaneDepthSizeOf2DMove;
extern f32 GLI_gsWaterPlaneDepthStartOf2DMove;
extern f32 GLI_gsfWaterPlaneFogStart;
extern u8 inv_GLI_bBuildCosinusTable;
extern s32 gs_lXBase;
extern s32 gs_lYBase;
extern f32 GLI_gsfWaterPlaneFogColor[3];
f32 cosf(f32 angle);

extern s8 g_cSinusEffectFlag;
extern u32 bWaterPlaneState;
extern f32 fWaterPlaneDistance;
extern f32 xSinusEffectDepthStart;
extern f32 xSinusEffectDepthStartSaved;
extern s8 g_cSinusEffectStateSaved;
extern s8 g_cSinusEffectFlagSaved;
extern f32 gs_xFactor;
extern MTH3D_tdstVector_ stWaterPlaneNormale;
extern MTH3D_tdstVector_ stSinEffectVector;
extern MTH3D_tdstVector_ stSinEffectAmpSaved;
extern MTH3D_tdstVector_ stSinEffectVectorSaved;
extern f32 a3_xSinEffectBaseMesh[3];
extern s32 a3_lSinEffectFreqMesh[3];
extern f32 a3_xSinEffectBaseMeshSaved[3];
extern s32 a3_lSinEffectFreqMeshSaved[3];
extern f32 a3_lSinRLIAmp[3];
extern f32 a3_lSinRLIBase[3];

void GLI_vSetWaterplane(void* unused, MTH3D_tdstVector_* vector, f32 value)
{
    stWaterPlaneNormale = *vector;
    fWaterPlaneDistance = value;
}

void GLI_vSetWaterplaneOn(void)
{
    bWaterPlaneState |= 1;
}

void GLI_vSetWaterplaneOff(void)
{
    bWaterPlaneState &= ~1;
}

void GLI_vWaterplaneDisable2DSinus(void)
{
    bWaterPlaneState |= 0x20;
}

void GLI_vInitWaterplaneModule(void)
{
    s32 i;
    f32* entry;

    GLI_fF2pow24 = 25165824.0f;
    if (inv_GLI_bBuildCosinusTable != 0) {
        entry = a64_fSinTable;
        i = 0;
        do {
            *entry = 2.0f * cosf((f32) i * 0.09817477f);
            i++;
            entry++;
        } while (i < 64);
    }
    inv_GLI_bBuildCosinusTable = 0;
    bWaterPlaneState = 0;
    gs_lXBase = 0;
    gs_lYBase = 0x20;
    GLI_gsfWaterPlaneFogColor[0] = -0.6f;
    GLI_gsfWaterPlaneFogColor[1] = -0.3f;
    GLI_gsfWaterPlaneFogColor[2] = -0.4f;
    stWaterPlaneNormale.xZ = 0.0f;
    fWaterPlaneDistance = 0.0f;
    stSinEffectAmp.xY = 0.0f;
    GLI_gsWaterPlaneRefraction = 0.7f;
    GLI_gsWaterPlaneDepthSizeOf2DMove = 1.4f;
    GLI_gsWaterPlaneDepthStartOf2DMove = -5.0f;
    GLI_gsfWaterPlaneFogStart = 7.0f;
    stWaterPlaneNormale.xX = 0.0f;
    stWaterPlaneNormale.xY = 1.0f;
    stSinEffectAmp.xX = 1.0f;
    stSinEffectAmp.xZ = 0.0f;
    a3_lSinEffectFreqMesh[0] = 1;
    a3_lSinEffectFreqMesh[1] = 0;
    a3_lSinEffectFreqMesh[2] = 0;
    a3_xSinEffectBaseMesh[0] = 0.0f;
    a3_xSinEffectBaseMesh[1] = 0.0f;
    a3_xSinEffectBaseMesh[2] = 0.0f;
    xSinusEffectDepthStart = 10.0f;
    g_cSinusEffectState = 0;
    g_cSinusEffectFlag = 0;
}

void GLI_fn_SinusEffect_vSetState(s32 _cState)
{
    g_cSinusEffectState = _cState;
}

s32 GLI_fn_SinusEffect_vGetState(void)
{
    return g_cSinusEffectState;
}

void GLI_fn_SinusEffect_vSetFlag(s32 _cFlag)
{
    g_cSinusEffectFlag = _cFlag;
}

s8 GLI_fn_SinusEffect_vGetFlag(void)
{
    return g_cSinusEffectFlag;
}

s32 GLI_fn_SinusEffect_cIsSecondViewportAffected(void)
{
    return g_cSinusEffectFlag & 1;
}

void GLI_fn_SinusEffect_vSetFreq(s32 frequency0, s32 frequency1, s32 frequency2)
{
    a3_lSinEffectFreqMesh[0] = frequency0;
    a3_lSinEffectFreqMesh[1] = frequency1;
    a3_lSinEffectFreqMesh[2] = frequency2;
}

void GLI_fn_SinusEffect_vSetAmplitude(const MTH3D_tdstVector_* _p_stAmpVector)
{
    stSinEffectAmp = *_p_stAmpVector;
}

void GLI_vUpdateEffectData(u32 _ulDeltaT)
{
    f32 step = (f32) _ulDeltaT * 0.0128f;

    gs_xFactor = step;
    a3_xSinEffectBaseMesh[0] = a3_xSinEffectBaseMesh[0] + a3_lSinEffectFreqMesh[0] * step;
    a3_xSinEffectBaseMesh[1] = a3_xSinEffectBaseMesh[1] + a3_lSinEffectFreqMesh[1] * step;
    a3_xSinEffectBaseMesh[2] = a3_xSinEffectBaseMesh[2] + a3_lSinEffectFreqMesh[2] * step;
}

void GLI_vSinusEffetOn3DVertex(MTH3D_tdstVector_* p_stVertex3D)
{
    f32 bias;
    f32 x;
    f32 y;
    f32 z;
    f32 offsetX;
    f32 offsetY;
    f32 wave;
    f32 amplitudeZ;

    union {
        f32 f;
        s32 i;
    } phase;

    if (inv_GLI_renderContext->inv_bUseSinEffectBase != 0) {
        bias = GLI_fSinEffectBase;
    } else {
        bias = GLI_fF2pow24;
    }

    y = p_stVertex3D->xY;
    z = p_stVertex3D->xZ;
    x = p_stVertex3D->xX;

    phase.f = a3_xSinEffectBaseMesh[0] + y + z + bias;
    offsetX = a64_fSinTable[phase.i & 0x3F] * stSinEffectAmp.xX;
    phase.f = a3_xSinEffectBaseMesh[1] + x + z + bias;
    offsetY = a64_fSinTable[phase.i & 0x3F] * stSinEffectAmp.xY;
    phase.f = a3_xSinEffectBaseMesh[2] + y + x + bias;
    wave = a64_fSinTable[phase.i & 0x3F];
    amplitudeZ = stSinEffectAmp.xZ;

    MTH3D_M_vSetXYZ_inv(p_stVertex3D, x + offsetX, y + offsetY, z + wave * amplitudeZ);
}

void GLI_vSetSinusEffectOnRLIAmp(f32 xRed, f32 xGreen, f32 xBlue)
{
    a3_lSinRLIAmp[0] = xRed;
    a3_lSinRLIAmp[1] = xGreen;
    a3_lSinRLIAmp[2] = xBlue;
}

void GLI_vSetSinusEffectOnRLIBase(f32 xRed, f32 xGreen, f32 xBlue)
{
    a3_lSinRLIBase[0] = xRed;
    a3_lSinRLIBase[1] = xGreen;
    a3_lSinRLIBase[2] = xBlue;
}

void GLI_vSetSinEffetBase(f32 xBase)
{
    GLI_fSinEffectBase = xBase;
}

void GLI_vSaveSinusContext(void)
{
    f32 speed = xSinusEffectDepthStart;
    u8 mipMappingMode = g_cSinusEffectState;
    u8 sinusFlags = g_cSinusEffectFlag;
    s32 i;

    for (i = 0; i < 3; i++) {
        a3_xSinEffectBaseMeshSaved[i] = a3_xSinEffectBaseMesh[i];
        a3_lSinEffectFreqMeshSaved[i] = a3_lSinEffectFreqMesh[i];
    }
    stSinEffectAmpSaved = stSinEffectAmp;
    xSinusEffectDepthStartSaved = speed;
    g_cSinusEffectStateSaved = mipMappingMode;
    g_cSinusEffectFlagSaved = sinusFlags;
    stSinEffectVectorSaved = stSinEffectVector;
}

void GLI_vRestoreSinusContext(void)
{
    f32 speed = xSinusEffectDepthStartSaved;
    u8 mipMappingMode = g_cSinusEffectStateSaved;
    u8 sinusFlags = g_cSinusEffectFlagSaved;
    s32 i;

    for (i = 0; i < 3; i++) {
        a3_xSinEffectBaseMesh[i] = a3_xSinEffectBaseMeshSaved[i];
        a3_lSinEffectFreqMesh[i] = a3_lSinEffectFreqMeshSaved[i];
    }
    stSinEffectAmp = stSinEffectAmpSaved;
    xSinusEffectDepthStart = speed;
    g_cSinusEffectState = mipMappingMode;
    g_cSinusEffectFlag = sinusFlags;
    stSinEffectVector = stSinEffectVectorSaved;
}
