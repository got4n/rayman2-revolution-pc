#include "common.h"

#include "GAM/GAM.h"
#include "MTH/MTH.h"
#include "GLI/GLI.h"
#include "TMR/TMR.h"
#include "clib/clib.h"

#define CAMERA_TYPE 0x67

typedef enum ViewportAttribute {
    VIEWPORT_SIZE = 1,
    VIEWPORT_CLIP = 2,
    VIEWPORT_ORIGIN = 3,
    VIEWPORT_ORIGIN_PER_MILLE = 4
} ViewportAttribute;

extern char inv_GAM_formattedLevelName[];
extern u32 GAM_gs_tdeLoadingMode;

s32 strcasecmp(const char* a, const char* b);
void fn_vSetGLIDisplayFlag(u32* flags, s32 type, u8 value);
void fn_vCreateFixViewPort(s16 device);
void fn_vCreateFixCamera(void);
void fn_vSetFirstLevelName(const char* name);
void inv_fn_vSetLevelNumber(const char* number);
void loc_112C40(s16 device);
void fn_vCreateGameViewPortCamera(void);

void fn_vChangeEngineMode(u8 eNewMode)
{
    g_stEngineStructure.eEngineMode = eNewMode;
}

void fn_vSetLevelName(const char* _szName)
{
    char p_cDelimiter[0x100];
    char* p_cEntry;

    strcpy(p_cDelimiter, _szName);

    p_cEntry = inv_My_strchr(p_cDelimiter, '#');
    if (p_cEntry != NULL) {
        strcpy(g_stEngineStructure.szFirstLevelName, p_cEntry + 1);
        *p_cEntry = '\0';
    } else {
        strcpy(g_stEngineStructure.szFirstLevelName, "Default");
    }

    p_cEntry = inv_My_strchr(p_cDelimiter, '$');
    if (p_cEntry != NULL) {
        strcpy(g_stEngineStructure.levelNumber, p_cEntry + 1);
        *p_cEntry = '\0';
    } else {
        strcpy(g_stEngineStructure.levelNumber, "Default");
    }

    strcpy(g_stEngineStructure.szLevelName, p_cDelimiter);
}

void* fn_lGetSubMapNumber(void)
{
    return NULL;
}

char* fn_p_szGetLevelName(void)
{
    return g_stEngineStructure.szLevelName;
}

char* inv_fn_p_szGetFullLevelName(void)
{
    strcpy(inv_GAM_formattedLevelName, g_stEngineStructure.szLevelName);
    if (strcasecmp(g_stEngineStructure.levelNumber, "default") != 0) {
        Strcat(inv_GAM_formattedLevelName, "$");
        Strcat(inv_GAM_formattedLevelName, g_stEngineStructure.levelNumber);
    }
    if (strcasecmp(g_stEngineStructure.szFirstLevelName, "default") != 0) {
        Strcat(inv_GAM_formattedLevelName, "#");
        Strcat(inv_GAM_formattedLevelName, g_stEngineStructure.szFirstLevelName);
    }
    return inv_GAM_formattedLevelName;
}

void inv_fn_vGetRandomLevelName(char* name)
{
    s32 index = func_0014BA90() % g_stEngineStructure.ucNumberOfLevels;

    strcpy(name, g_stEngineStructure.a_szLevelName[index]);
}

char* inv_fn_p_szGetLevelNumber(void)
{
    return g_stEngineStructure.levelNumber;
}

char* fn_p_szGetFirstLevelName(void)
{
    return g_stEngineStructure.szFirstLevelName;
}

void fn_vSetNextLevelName(const char* name)
{
    strcpy(g_stEngineStructure.szNextLevelName, name);
}

void inv_fn_vSetLevelNumber(const char* number)
{
    if (number != NULL) {
        strcpy(g_stEngineStructure.levelNumber, number);
        return;
    }
    strcpy(g_stEngineStructure.levelNumber, "Default");
}

void fn_vSetFirstLevelName(const char* name)
{
    if (name != NULL) {
        strcpy(g_stEngineStructure.szFirstLevelName, name);
        return;
    }
    strcpy(g_stEngineStructure.szFirstLevelName, "Default");
}

char* fn_p_szGetNextLevelName(void)
{
    return g_stEngineStructure.szNextLevelName;
}

void fn_vFreeNextLevelName(void)
{
    g_stEngineStructure.szNextLevelName[0] = '\0';
}

s32 fn_bIsNextLevelNameValid(void)
{
    return g_stEngineStructure.szNextLevelName[0] != '\0';
}

void GAM_fn_bIsSlotValid(s32 type, u8 value)
{
    fn_vSetGLIDisplayFlag(&g_stEngineStructure.ulDisplayMode, type, value);
}

void fn_vSetGLIDisplayFlag(u32* p_DisplayMode, s32 ulFlag, u8 ucValue)
{
    if (ulFlag == 0) {
        if (ucValue == 1) {
            *p_DisplayMode = 0xFFFFFFFF;
        } else {
            *p_DisplayMode = 0;
        }
    } else if (ulFlag == 2) {
        if (ucValue == 1) {
            *p_DisplayMode |= 1;
        } else {
            *p_DisplayMode &= ~1;
        }
    } else if (ulFlag == 1) {
        if (ucValue == 1) {
            *p_DisplayMode |= 2;
        } else {
            *p_DisplayMode &= ~2;
        }
    } else if (ulFlag == 3) {
        if (ucValue == 0) {
            *p_DisplayMode |= 0x8000;
        } else {
            *p_DisplayMode &= ~0x8000;
        }
    } else if (ulFlag == 5) {
        if (ucValue == 0) {
            *p_DisplayMode |= 4;
        } else {
            *p_DisplayMode &= ~4;
        }
    } else if (ulFlag == 6) {
        if (ucValue == 0) {
            *p_DisplayMode |= 0x400000;
        } else {
            *p_DisplayMode &= ~0x400000;
        }
    }
}

void fn_vChangeViewPortPercent(s32 gameViewport, s32 attribute, s32 widthPercent, s32 heightPercent,
                               s32 clipTop, s32 clipLeft, s32 clipBottom, s32 clipRight,
                               s32 originX, s32 originY)
{
    GLD_tdstViewportAttributes_ attributes;
    struct GLI_tdstCamera_* camera;

    if (gameViewport == 1) {
        GLI_xGetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport, &camera);
    } else {
        GLI_xGetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, &camera);
    }
    if (camera == NULL) {
        return;
    }
    GLD_bGetViewportAttributes(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                                 &attributes);
    if (attribute == VIEWPORT_SIZE) {
        attributes.values.dwWidthInPercent = widthPercent;
        attributes.values.dwHeightInPercent = heightPercent;
    } else if (attribute == VIEWPORT_CLIP) {
        attributes.values.dwClipTopInPerMille = clipTop;
        attributes.values.dwClipLeftInPerMille = clipLeft;
        attributes.values.dwClipBottomInPerMille = clipBottom;
        attributes.values.dwClipRightInPerMille = clipRight;
    } else if (attribute == VIEWPORT_ORIGIN) {
        attributes.values.iOffsetPosX = originX;
        attributes.values.iOffsetPosY = originY;
    } else if (attribute == VIEWPORT_ORIGIN_PER_MILLE) {
        attributes.values.iOffsetPosX = originX * (s32) attributes.values.dwWidth / 1000;
        attributes.values.iOffsetPosY = originY * (s32) attributes.values.dwHeight / 1000;
    }
    if (gameViewport == 1) {
        GLD_bSetViewportPosition(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                                 &attributes);
        GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                                    camera);
    } else {
        GLD_bSetViewportPosition(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport,
                                 &attributes);
        GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, camera);
    }
}

void fn_vChangeViewportRatio(s32 gameViewport, f32 ratio)
{
    struct GLI_tdstCamera_* p_stCam;

    if (gameViewport == 1) {
        GLI_xGetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport, &p_stCam);
        if (p_stCam != NULL) {
            GLI_xSetCameraRatio(p_stCam, ratio);
            GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                                        p_stCam);
        }
    } else {
        GLI_xGetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, &p_stCam);
        if (p_stCam != NULL) {
            GLI_xSetCameraRatio(p_stCam, ratio);
            GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport,
                                        p_stCam);
        }
    }
}

void GAM_fn_vActualizeAllGameViewportSize(void)
{
    struct GLI_tdstCamera_* camera;

    GLI_xGetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport, &camera);
    if (camera != NULL) {
        GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                                    camera);
    }
    GLI_xGetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, &camera);
    if (camera != NULL) {
        GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, camera);
    }
}

void GAM_fn_vCreateGameDeviceAndViewPorts(void)
{
    g_stEngineStructure.p_stFixCamera = 0;
    fn_vCreateFixViewPort(g_stEngineStructure.hGLDDevice);
    fn_vCreateGameViewPortCamera();
}

void fn_vDestroyFixViewPort(void)
{
    loc_112C40(g_stEngineStructure.hGLDDevice);
}

void fn_vCreateFixViewPort(s16 device)
{
    tdstEngineStructure_* engine = &g_stEngineStructure;

    engine->stFixViewportAttr.values.dwWidthInPercent = 100;
    engine->stFixViewportAttr.values.dwHeightInPercent = 100;
    engine->stFixViewportAttr.values.dwClipTopInPerMille = 0;
    engine->stFixViewportAttr.values.dwClipBottomInPerMille = 1000;
    engine->stFixViewportAttr.values.dwClipLeftInPerMille = 0;
    engine->stFixViewportAttr.values.dwClipRightInPerMille = 1000;
    engine->stFixViewportAttr.values.iOffsetPosX = 0;
    engine->stFixViewportAttr.values.iOffsetPosY = 0;
    GLD_bCreateViewport(device, &engine->stFixViewportAttr, &engine->hGLDFixViewport);
    GLD_bGiveSpecificAttributesToViewport(device, engine->hGLDFixViewport, &engine->stFixAttributes3D);
    GLD_bGetViewportAttributes(device, engine->hGLDFixViewport, &engine->stFixViewportAttr);
    if (engine->p_stFixCamera == 0) {
        fn_vCreateFixCamera();
    }
    GLI_xAdjustCameraToViewport(device, engine->hGLDFixViewport, engine->p_stFixCamera);
    GLI_xSetViewportCamera(device, engine->hGLDFixViewport, engine->p_stFixCamera);
}

void loc_112C40(s16 device)
{
    GLD_bDeleteViewport(device, g_stEngineStructure.hGLDFixViewport);
}

void fn_vCreateFixCamera(void)
{
    POS_tdstMatrix_ matrix;
    POS_tdstMatrix_ rotation;
    MTH3D_tdstVector_ translation;

    GLI_xCreateCamera(&g_stEngineStructure.p_stFixCamera, CAMERA_TYPE);
    GLI_xSetCameraAspectAndRatio(g_stEngineStructure.p_stFixCamera, 1.5f, 0.75f);
    GLI_xSetCameraNearFar(g_stEngineStructure.p_stFixCamera, 0.1f, 1000.0f);
    POS_fn_vRotateX(&matrix, MTH_HALF_PI_inv);
    POS_fn_vRotateZ(&rotation, MTH_PI_inv);
    _POS_fn_vMulMatrixMatrix(&matrix, &rotation, &matrix);

    MTH3D_M_vSetXYZ_inv(&translation, 0.0f, 0.0f, 5.0f);
    if (matrix.ucType == MAT_eTransform_Identity) {
        matrix.ucType = MAT_eTransform_Translation;
    }
    MTH4D_fn_vCopy3DTo4DVector(&matrix.stMatrix.stCol_3, &translation);
    GLI_xSetCameraMatrix(g_stEngineStructure.p_stFixCamera, &matrix);
}

void fn_vReputGameViewportCamera(void)
{
    GLI_xSetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                           g_stEngineStructure.p_stGameViewportCamera);
}

void fn_vCreateGameViewPortCamera(void)
{
    GLI_xCreateCamera(&g_stEngineStructure.p_stGameViewportCamera, CAMERA_TYPE);
    GLI_xSetCameraAspectAndRatio(g_stEngineStructure.p_stGameViewportCamera, 1.5f, 0.75f);
    GLI_xSetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                           g_stEngineStructure.p_stGameViewportCamera);
}

void GAM_fn_vAskToChangeLevel(const char* szNewLevelName)
{
    char p_cDelimiter[0x50];
    char* p_cEntry;
    char* firstLevelSeparator;

    strcpy(p_cDelimiter, szNewLevelName);
    p_cEntry = inv_My_strchr(p_cDelimiter, '$');
    firstLevelSeparator = inv_My_strchr(p_cDelimiter, '#');
    fn_vSetNextLevelName(szNewLevelName);
    fn_vSetFirstLevelName(firstLevelSeparator);
    if (firstLevelSeparator != NULL) {
        *firstLevelSeparator = '\0';
    }
    inv_fn_vSetLevelNumber(p_cEntry);
    fn_vChangeEngineMode(6);
}

s32 func_00112E38(void)
{
    return GAM_gs_tdeLoadingMode == 0;
}

void GAM_fn_vSetEngineInFixMode(void)
{
    GAM_gs_tdeLoadingMode = 0;
}

void GAM_fn_vSetEngineInLevelMode(void)
{
    GAM_gs_tdeLoadingMode = 1;
}

void func_00112E60(void)
{
}

u8 fn_bIsEngineInPause(void)
{
    return g_stEngineStructure.bEngineIsInPaused;
}

void fn_vSetEngineInPaused(void)
{
    g_stEngineStructure.bEngineIsInPaused = 1;
    if (g_stEngineStructure.bEngineFrozen == 0) {
        fn_vSaveEngineClock();
    }
}

void fn_vResetEngineFromPaused(void)
{
    g_stEngineStructure.bEngineIsInPaused = 0;
    if (g_stEngineStructure.bEngineFrozen == 0) {
        fn_vLoadEngineClock();
    }
}
