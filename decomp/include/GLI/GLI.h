#ifndef REVO_GLI_GLI_H
#define REVO_GLI_GLI_H

#include "ps2/types.h"
#include "GEO/GEO.h"
#include "MTH/MTH.h"
#include "renderware_fwd.h"
#include "ps2/copy_types.h"

typedef struct GLI_tdstLight_ {
    u8 b_lOnOff;
    u8 inv_bShadowCaster;
    u16 lTypeOfLight;
    f32 xFar;
    f32 xNear;
    f32 xLittleAlpha;
    f32 xBigAlpha;
    f32 xLittleTangent;
    f32 xBigTangent;
    POS_tdstMatrix_ stMatrix;
    GEO_tdstColor_ stColor;
    f32 inv_xShadowIntensity;
    u8 bValid;
    u8 ucObjectLighted;
    u8 bIsPainting;
    u8 ucIsAlpha;
    MTH3D_tdstVector_ xInter_Min_Pos;
    MTH3D_tdstVector_ xExter_Min_Pos;
    MTH3D_tdstVector_ xInter_Max_Pos;
    MTH3D_tdstVector_ xExter_Max_Pos;
    MTH3D_tdstVector_ xCenterBox;
    f32 xRadius;
    f32 xIntensity_Min;
    f32 xIntensity_Max;
    MTH4D_tdstVector_ stBackgroundColor;
    void* backendLight;
} GLI_tdstLight_;

#define LEVEL_LIGHT_COUNT 16

typedef struct LevelLightList {
    GLI_tdstLight_* lights[LEVEL_LIGHT_COUNT];
    u8 count;
} LevelLightList;

typedef union GliVertexColor {
    u32 packed;

    struct {
        u8 blue;
        u8 green;
        u8 red;
        u8 alpha;
    } channels;
} GliVertexColor;

typedef struct tdstFogParams_ {
    f32 xDepthStart;
    f32 xDepthEnd;
    f32 xInfinite;
    f32 xBlendStart;
    f32 xBlendEnd;
    union {
        GEO_tdstColor_ stColor;
        PackedU64x2Aligned4 stWords;
    } uColor;
} tdstFogParams_;

#define GLI_DRAW_DEFERRED_inv 0x8000

typedef struct GLI_tdstRenderContext {
    void* object;
    struct GeometricObjectView* geometricObject;
    s32 lHierachDrawMask;
    s32 lCurrentDrawMask;
    s32 inv_lDrawFlags;
    u8 unk0014[0x1F50];
    POS_tdstMatrix_* geometryMatrix;
    POS_tdstMatrix_* currentMatrix;
    u8 unk1F6C[0x54];
    POS_tdstMatrix_ objectMatrix;
    f32 inv_xNear;
    f32 inv_xOoNear;
    f32 clipWindow[4];
    struct GliTexture* currentTexture;
    struct GliMaterialData* currentMaterial;
    s32 resetWord2034;
    struct GLI_tdstCamera_* camera;
    s32 unk203C;
    u8 unk2040[0x10];
    GliVertexColor vertexColor;
    u8 unk2054[0xC];
    MTH4D_tdstMatrix_
        im3dMatrix;
    tdstFogParams_* fogParameters;
    tdstFogParams_* appliedFogParameters;
    s32 fogEnabled;
    s32 unk20AC;
    s32 unk20B0;
    s32 inv_bNoCameraClear;
    s32 inv_bUseSinEffectBase;
    f32 globalAlpha;
} GLI_tdstRenderContext;

extern GLI_tdstRenderContext* inv_GLI_renderContext;

typedef struct GLI_tdst2DVertex_ {
    f32 xX;
    f32 xY;
    f32 xOoZ;
} GLI_tdst2DVertex_;

typedef struct GLI_tdstCamera_ {
    s32 lCameraMode;
    POS_tdstMatrix_ stMatrix;
    f32 xAlphaX;
    f32 xAlphaY;
    f32 xNear;
    f32 xFar;
    f32 xScreen;
    GLI_tdst2DVertex_ stScale;
    GLI_tdst2DVertex_ stTrans;
    f32 xXProjectionRatio;
    f32 xYProjectionRatio;
    MTH4D_tdstVector_ clipPlanes[4];
    f32 xRatio;
} GLI_tdstCamera_;

typedef struct ProjectedVertex {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} ProjectedVertex;

struct LevelLightList;

#define MATERIAL_COLOR_FROM_TEXTURE 0x4000000

typedef struct GliTexture GliTexture;

extern s32 GLI_RW_bInitAPI(void* pDisplay, void* pEngineInitParam, s32 (*pfnCallback)(void));
extern void inv_GLI_RW_vApplyFogParameters(tdstFogParams_* fog);
extern void GLI_RW_vDoTextureSelection(void);
extern void GLI_RW_vFogOff(void);
extern void GLI_RW_vFogOn(void);
extern void inv_GLI_RW_vClearCamera(s32 a0, s32 a1, s32 a2);
extern void inv_GLI_RW_vFlush2DVertexBuffer(void);
extern s32 func_0011BC98(void);
extern s32 inv_GLI_RW_bRender2DVertexBuffer(void);
extern void GLI_RW_vDoMaterialSelection(void);
extern void inv_GLI_RW_vAdd2DRectangle(s32 x0, s32 y0, s32 x1, s32 y1);
extern void inv_GLI_RW_vReset2DVertexBuffer(void);
extern void inv_GLI_RW_vSetCameraFOV(GLI_tdstCamera_* p_stCamera);
extern void inv_GLI_RW_vShowCameraRaster(void);
extern void loc_11C5D0(void);
extern void inv_GLI_RW_vClearCameraIfEnabled(s32 left, s32 right, s32 top, s32 bottom);
extern void GLI_RW_xInitRasterize(s32 a0, s32 a1, s32 a2, s32 a3, s32 depth);

extern void RW3_Light_AddListOfLightsToWorld(struct LevelLightList* list);
extern RpMaterial* inv_RW3_fn_pCreateDefaultRpMaterial(void);
extern void inv_RW3_Texture_PurgeLevelResources(void);
extern s32 inv_fn_bLoadLevelTextureDictionaries(char* name);
extern RwTexture* inv_RW3_fn_p_stFindLevelTexture(char* name);
extern void inv_RW3_fn_vSetRpMaterialColor(RpMaterial* material, GEO_tdstColor_* color);
struct RW3ObjectView;
extern void inv_RW3_fn_vScrollObjectTexCoords(struct RW3ObjectView* rw3Object, GLI_tdstRenderContext* context);

extern void inv_RW3_fn_vLoadVignetteRaster(RwRaster* raster, char* name);

struct SpriteElement;
struct GLI_tdstLight_;
struct GLD_tdstViewportAttributes_;
extern void RW3_Light_UpdateMatrixFromGLILightMatrix(struct GLI_tdstLight_* light);
extern void inv_RW3_fn_vSetSingleLightColor(void* color);
extern void RW3_Light_CreateFromGLILight(GLI_tdstLight_* light);
extern void RW3_Light_SetAngle(GLI_tdstLight_* light);
extern void RW3_Light_SetColor(GLI_tdstLight_* light, GEO_tdstColor_* color);
extern void RW3_Light_SetRadius(GLI_tdstLight_* light);
extern void inv_RW3_fn_vDrawQueuedHalos(struct GLD_tdstViewportAttributes_* viewport);
extern void inv_RW3_fn_vResetEnabledLights(void);
extern void inv_RW3_fn_vDisableSingleLight(void);
extern void inv_RW3_fn_vClearLightList(LevelLightList* list);
extern void inv_RW3_fn_vSetViewportLightMask(s32 mask);
extern void func_0014EA40(void);
extern void inv_RW3_fn_vCopyViewportLights(LevelLightList* list);
extern void inv_GLI_vPopLightsInViewport(void);
extern void inv_RW3_fn_vSendDeformedObject(struct GeometricObjectView* object);
extern void inv_RW3_fn_vQueueHalo(struct GeometricObjectView* source, struct SpriteElement* sprite);

struct HIE_tdstSuperObject_;
extern void inv_GLI_fn_vRenderPrepared2DRectangle(void);
extern void RW3_SHW_fn_vBeginRenderCharacterInShadowView(void);
extern void RW3_SHW_fn_vEndRenderCharacterInShadowView(void);
extern void func_00153970(u8* modeOut);
extern void inv_RW3_fn_vInitScreenQuadVertices(void);
extern void inv_SHW_fn_vDesInitShadow(void);
extern void inv_GLI_fn_vDraw2DTexturedRectWithColor(struct GLD_tdstViewportAttributes_* viewport,
                                                    u32 color, f32 x, f32 y, f32 z, f32 w);
extern s32 RW3_SHW_fn_bInit(s32 a0, s32 a1);
extern void RW3_SHW_fn_vRenderCharacterInShadowView(struct HIE_tdstSuperObject_* superObject,
                                                    POS_tdstMatrix_* matrix,
                                                    struct GliMaterialData* material);
extern void func_001539C0(u8* modeOut, GLI_tdstRenderContext* context);

struct GLD_tdstViewportAttributes_;
extern void inv_RW3_fn_vFlushAllQuadBatches(struct GLD_tdstViewportAttributes_* viewport);
extern void inv_RW3_fn_vInitQuadBatches(void);

extern void GLI_vFogOff(void);
extern void GLI_vFogOn(void);
extern void GLI_vSetFog2(GEO_tdstColor_* color, f32 xBlendStart, f32 xDepthStart, f32 xBlendEnd, f32 xDepthEnd, f32 xInfinite);

extern void GLI_vResetTableOfActiveLightsForChar(s32 value);
extern POS_tdstMatrix_* inv_GLI_p_stGetLightMatrix(GLI_tdstLight_* light);
extern f32 GLI_fn_xFogGetBlendFar(GLI_tdstLight_* p_stLight);
extern f32 GLI_fn_xFogGetBlendNear(GLI_tdstLight_* p_stLight);
extern f32 GLI_fn_xFogGetFar(GLI_tdstLight_* p_stLight);
extern f32 GLI_fn_xFogGetInfinite(GLI_tdstLight_* p_stLight);
extern f32 GLI_fn_xFogGetNear(GLI_tdstLight_* p_stLight);
extern s32 GLI_bDoesLightAffectBV(GLI_tdstLight_* light, MTH3D_tdstVector_* center, f32 radius);
extern void GLI_vGetLightAlphas(GLI_tdstLight_* p_stLight, f32* p_xLittleAlpha, f32* p_xBigAlpha);
extern void GLI_vGetLightNearFar(GLI_tdstLight_* p_stLight, f32* p_xNear, f32* p_xFar);
extern void GLI_vPopLightsInViewport(void* lUnused, s32 lNbLightsToPop);
extern void GLI_vSetLightAlphas(GLI_tdstLight_* p_stLight, f32 xLittleAlpha, f32 xBigAlpha);
extern void GLI_vSetLightType(GLI_tdstLight_* p_stLight, u16 lNewType);
extern void GLI_vSetObjectLighted(GLI_tdstLight_* light, u8 enabled);
extern void GLI_xCreateLight(GLI_tdstLight_** h_stLight, s32 lTypeOfLight);
extern void GLI_xSetLightMatrix(GLI_tdstLight_* p_stLight, POS_tdstMatrix_* p_stMatrix);
extern void inv_GLI_vSetLightSectorSwitch(GLI_tdstLight_* light, u8 enabled);
extern void GLI_vSetLightSwitch(GLI_tdstLight_* light, u8 value);
extern u32 GLI_ucGetObjectLighted(GLI_tdstLight_* p_stLight);
extern void GLI_vCopyLight(GLI_tdstLight_* p_stLightDst, const GLI_tdstLight_* p_stLightSrc);
extern void GLI_xGetLightColor(const GLI_tdstLight_* p_stLight, GEO_tdstColor_* p_stColor);
extern void GLI_xSetViewportLightsOnOff(u8 bOnOff);
extern void GLI_vSetLightNearFar(GLI_tdstLight_* p_stLight, f32 xNear, f32 xFar);
extern s32 gs_CurrentNumberOfLightInViewport;
extern u16 GLI_lGetLightType(GLI_tdstLight_* p_stLight);
extern void GLI_xGetLightBackgroundColor(const GLI_tdstLight_* light, MTH4D_tdstVector_* color);
extern void GLI_xSetLightColor(GLI_tdstLight_* p_stLight, GEO_tdstColor_* p_stColor);
extern void GLI_vClearListOfLightInViewport(struct GLD_tdstViewportAttributes_* _p_stVpt);
extern void GLI_vAddLightToViewport(void* viewport, GLI_tdstLight_* light);
extern void GLI_xGetLightState(const GLI_tdstLight_* p_stLight, s32* pb_lOnOff);

extern f32 GLI_vGetGlobalAlpha(void);
extern void GLI_vSetGlobalAlpha(f32 alpha);
extern void GLI_xSendObjectToViewportWithLights(struct GLD_tdstViewportAttributes_* p_stVpt,
                                                struct GeometricObjectView* p_stObj,
                                                s32 lDrawModeMask);

#define MATERIAL_LAYER_COUNT 1

#define MATERIAL_SCROLL_U 0x2
#define MATERIAL_SCROLL_V 0x4
#define MATERIAL_SCROLL_CONSTANT 0x10
#define MATERIAL_SCROLL_ANY (MATERIAL_SCROLL_U | MATERIAL_SCROLL_V | MATERIAL_SCROLL_CONSTANT)
#define MATERIAL_LAYER_CONTINUES 0x40

struct GliTexture {
    RwTexture* rwTexture;
    u16 flags;
    u8 addressMode;
    u8 unk07;
    u16 lHeight;
    u16 lWidth;
    u16 lRealHeight;
    u16 lRealWidth;
    u8 loaded;
};

typedef struct AnimatedTextureElement {
    GliTexture* texture;
    f32 xDisplayTime;
    struct AnimatedTextureElement* next;
} AnimatedTextureElement;

typedef struct MaterialLayer {
    GliTexture* texture;
    u8 mode;
    u8 unk05;
    u8 unk06;
    u8 flags;
    f32 scrollU;
    f32 scrollV;
    u8 unk10[8];
    f32 constantScrollU;
    f32 constantScrollV;
    RpMaterial* rpMaterial;
} MaterialLayer;

typedef struct GliMaterialData {
    u32 type;
    GEO_tdstColor_ color;
    u32 frame;
    AnimatedTextureElement* animatedFirst;
    AnimatedTextureElement* animatedCurrent;
    f32 time;
    u16 animatedCount;
    u16 animationFlags;
    MaterialLayer layers[MATERIAL_LAYER_COUNT];
} GliMaterialData;

extern GliMaterialData** inv_RW3_p_pLevelMaterials;

typedef struct GeometricElement {
    GliMaterialData* material;
    s16 xNbFaces;
    s16 lightmapSlot;
    GEO_tdstTripledIndex_* d_stListOfFacesTripled;
} GeometricElement;

typedef struct SpriteElement {
    u16 flags;
    s16 vertexIndex;
    f32 halfWidth;
    f32 halfHeight;
    union {
        GliMaterialData* material;
        u8 inv_ucHaloIndex;
    } u;
} SpriteElement;

typedef struct SpriteElementList {
    SpriteElement* sprites;
    s16 count;
} SpriteElementList;

typedef struct RW3ObjectView {
    RwFrame* frame;
    RpAtomic** atomics;
    u16** vertexIndices;
    u8* atomicStates;
    struct TexCoord* scrollOffsets;
    u16 atomicCount;
    u8 mode;
} RW3ObjectView;

typedef struct LightmapSlot {
    RwRaster* r;
    RwRaster* g;
    RwRaster* b;
    void* uvs;
} LightmapSlot;

typedef struct GeometricObjectView {
    s16* elementTypes;
    GeometricElement** elements;
    s32 flags;
    f32 boundingSphereRadius;
    MTH3D_tdstVector_ boundingSphereCenter;
    RW3ObjectView* rw3Object;
    s16 vertexCount;
    s16 elementCount;
    MTH3D_tdstVector_* vertices;
    MTH3D_tdstVector_* normals;
} GeometricObjectView;

typedef struct QuadColor {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} QuadColor;

typedef struct QuadVertex {
    MTH3D_tdstVector_ position;
    QuadColor color;
    MTH3D_tdstVector_ normal;
    f32 u;
    f32 v;
} QuadVertex;

typedef struct Quad {
    QuadVertex v[4];
} Quad;

#define DRAW_KIND_TEXTURED 1
#define DRAW_KIND_ATOMIC 2

#define DRAW_REQUEST_KEEP_FRAME 0x01
#define DRAW_REQUEST_UNLIT 0x10

typedef struct AtomicDrawRequest {
    RpAtomic* atomic;
    s32 comparisonFlags;
    f32 alpha;
    u8 mode;
    u8 flags;
    RwFrame* frame;
    LevelLightList lights;
    tdstFogParams_* fogParameters;
} AtomicDrawRequest;

typedef struct TexturedDrawRequest {
    Quad quad;
    GliTexture* texture;
    tdstFogParams_* fogParameters;
} TexturedDrawRequest;

typedef union DrawQueueObject {
    AtomicDrawRequest* atomic;
    TexturedDrawRequest* textured;
} DrawQueueObject;

typedef struct DrawQueueEntry {
    DrawQueueObject object;
    s32 kind;
} DrawQueueEntry;

typedef struct GLD_tdstDeviceAttributes_ {
    s32 dwHeight;
    s32 dwWidth;
    f32 xPixelDimensionX;
    f32 xPixelDimensionY;
    u32 p_cVirtualScreen;
    s32 lPitch;
    u32 p_sZBuffer;
} GLD_tdstDeviceAttributes_;

typedef struct ScreenRect {
    s32 x;
    s32 y;
    s32 width;
    s32 height;
} ScreenRect;

#define LIGHT_COMMAND_TAG 0xFF000000
#define LIGHT_COMMAND_COUNT 0x00FFFFFF
#define LIGHT_COMMAND_LIGHTS 0x01000000
#define LIGHT_COMMAND_LIGHT_LIST 0x02000000
#define LIGHT_COMMAND_MODULE 0x03000000
#define LIGHT_COMMAND_FOG 0x04000000
#define LIGHT_COMMAND_LAST 0x04FFFFFF
#define LIGHT_COMMAND_STATE_05 0x05000000
#define LIGHT_COMMAND_STATE_06 0x06000000
#define LIGHT_COMMAND_STATE_SKIP 0x07000000

typedef union LightCommandValue {
    u32 word;
    struct HIE_tdstSuperObject_* superObject;
    f32 level;
} LightCommandValue;

void GLI_vDrawWorldQuadOpt(MTH3D_tdstVector_* corner0, MTH3D_tdstVector_* corner1, MTH3D_tdstVector_* corner2,
                   MTH3D_tdstVector_* corner3, GliMaterialData* material);
u8 GLI_vDrawWorldSpriteOpt(MTH3D_tdstVector_* center, GliMaterialData* material, f32 halfWidth, f32 halfHeight,
                 struct GLD_tdstViewportAttributes_* viewport, u32 count);
u8 GLI_vDrawWorldPlanarSprite(MTH3D_tdstVector_* center, GliMaterialData* material, f32 halfWidth, f32 halfHeight,
                 struct GLD_tdstViewportAttributes_* viewport, u32 count);
u8 GLI_vDrawWorldOrthoPlanarSprite(MTH3D_tdstVector_* center, GliMaterialData* material, f32 scale0, f32 scale1,
                 struct GLD_tdstViewportAttributes_* viewport, u32 count, MTH3D_tdstVector_* axis0,
                 MTH3D_tdstVector_* axis1);

extern LightmapSlot* inv_RW3_lightmapSlots;
extern GliMaterialData* inv_GLI_p_stHaloMaterial;

extern RwCamera* inv_GLI_primaryCamera;

extern Qword* inv_GLI_packetCursor;

extern struct LightHaloEntry* inv_RW3_p_stLightHalos;
extern u8* inv_RW3_p_ucQueuedHalos;

extern void GLI_fn_SinusEffect_vSetState(s32 _cState);
extern void GLI_fn_SinusEffect_vSetFlag(s32 _cFlag);
extern void GLI_fn_SinusEffect_vSetAmplitude(const MTH3D_tdstVector_* _p_stAmpVector);
extern s32 GLI_fn_SinusEffect_cIsSecondViewportAffected(void);
extern s32 GLI_fn_SinusEffect_vGetState(void);
extern void GLI_fn_SinusEffect_vSetFreq(s32 frequency0, s32 frequency1, s32 frequency2);
extern void GLI_vInitWaterplaneModule(void);
extern void GLI_vRestoreSinusContext(void);
extern void GLI_vSaveSinusContext(void);
extern void GLI_vSetSinEffetBase(f32 xBase);
extern void GLI_vSetSinusEffectOnRLIAmp(f32 xRed, f32 xGreen, f32 xBlue);
extern void GLI_vSetSinusEffectOnRLIBase(f32 xRed, f32 xGreen, f32 xBlue);
extern void GLI_vInitLensFlare(void);
extern void GLI_vUpdateEffectData(u32 _ulDeltaT);
extern s8 GLI_fn_SinusEffect_vGetFlag(void);
extern void GLI_vSinusEffetOn3DVertex(MTH3D_tdstVector_* p_stVertex3D);

extern void RW3_Utils_DuplicateFrame(RwFrame* destination, RwFrame* source);
extern void inv_RW3_fn_vCopyRasterSize(RwRaster* destination, RwRaster* source);
extern void inv_RW3_fn_vCreateWorldHeap(u32 size);
extern void inv_RW3_fn_vCopyRasterPalette(RwRaster* destination, RwRaster* source);
extern s32 inv_RW3_fn_bWriteRasterTex0(void* texture, void* unused, s32 context);
extern void inv_RW3_fn_vInitDefaultHeap(u32 size);

extern void GLI_vDoMaterialSelection(void);

extern void GLI_DRV_vSetClipWindow(f32 clip0, f32 clip1, f32 clip2, f32 clip3);
extern void GLI_DRV_vSetZClip(GLI_tdstCamera_* camera, f32 value);

extern void RW3_fn_vCreateAllRWObjectLEVEL(void);

extern void RW3_fn_vDestroyAllRWObjectLEVEL(void);

extern void inv_RW3_fn_vRegisterLightmapPlugin(void);
extern void RW3_DestroyRW3Object(RW3ObjectView* object);
extern void inv_RW3_fn_vCreateWorld(RwBBox* bounds);
extern void RW3_CreateRW3ObjectFromGeometric(GeometricObjectView* object);
extern void inv_RW3_fn_vSelectViewportCamera(struct GLD_tdstViewportAttributes_* attributes);
extern void RW3_UpdateFrameWithGeometricObjectMatrix(RwFrame* frame, const POS_tdstMatrix_* source);

extern void func_00150498(void);

extern void func_001516F8(void);
extern void inv_RW3_fn_vDestroyLightmapTemplates(void);
extern void* func_00151338(void);
extern void func_001515C0(void);

extern s32 inv_RW3_fn_bIsLightmapPipeline(void* pipeline);
extern void func_001519D0(void);
extern void func_001519F0(void);
extern void inv_RW3_fn_vSendFogDistances(f32 width, f32 height);
extern void inv_RW3_fn_vSetAtomicPipeline(RpAtomic* atomic, void* pipeline);
extern void* inv_RW3_fn_pGetAtomicPipeline(RpAtomic* atomic);

typedef struct MeshView {
    struct WorldVertex* vertices;
    u32 vertexCount;
    u32 capacity;
    GliMaterialData* material;
} MeshView;
extern void inv_RW3_fn_vEndMesh(MeshView* mesh);
extern void inv_RW3_fn_vDrawMesh(MeshView* mesh, f32 weight, tdstFogParams_* fogParameters);
extern void inv_RW3_fn_vSetRenderObject(void* object, u32 renderFlags);

extern void inv_RW3_fn_vInitMeshIndices(void);
extern s32 inv_RW3_fn_bCreateMesh(MeshView** out, s32 count);
extern void inv_RW3_fn_vBeginMesh(MeshView* mesh, MTH3D_tdstVector_* position);
extern void inv_RW3_fn_vAddMeshVertex(MTH3D_tdstVector_* position, f32* uv, f32 alpha);
extern void fn_vMSSoundSetMechaCharacter(MeshView* mesh, GliMaterialData* material);

extern void inv_RW3_fn_vDrawTexturedQuads(struct GLD_tdstViewportAttributes_* viewport, Quad* quads, GliTexture* texture);

extern s32 inv_RW3_fn_bReserveTexturedRequest(TexturedDrawRequest** result, GliTexture* texture);
extern void inv_RW3_fn_vSetAtomicAlpha(f32 intensity, RpAtomic* object);
extern void inv_RW3_fn_vInitAtomicRequests(void);
extern void inv_RW3_fn_vResetAtomicRequests(void);
extern void inv_RW3_fn_vSetRasterPalette(RwRaster* raster, void* palette);
extern RwRaster* inv_RW3_lightmapPaletteRaster;
extern u32 inv_RW3_lightmapVramR;
extern u32 inv_RW3_lightmapVramG;
extern u32 inv_RW3_lightmapVramB;
extern u8 g_ucTableOfActiveLightsForChar[];
extern f32 gs_fCoefOfMultiplicationForAnimatedTexture;
extern RwV3d inv_RW3_stWorldOrigin;

extern u32 inv_GLI_lightCommandBuffer[];
extern u32* inv_GLI_lightCommandCursor;
extern u32* inv_GLI_openLightCommand;

extern void func_00152978(struct GLD_tdstViewportAttributes_* viewport);
extern void func_00152B10(void);

extern void func_00153560(void);
extern void func_00153800(u8* color);

extern s32 GLD_bGetDeviceAttributes(s16 hDev, GLD_tdstDeviceAttributes_* pstDevAttrib);
extern s32 GLD_bGiveSpecificAttributesToViewport(s32 hDev, s16 hVP, void* p_vSpecAttrib);
extern s32 GLD_bDeleteViewport(s32 _unused, s16 hVP);
extern s32 GLD_bClearViewportWithChoice(s32 hDev, s32 hVP, s16 sChoice);
extern s32 GLD_bWriteToViewportFinished(s32 _unused, s16 hVP);
extern s32 GLD_bCreateDevice(GLD_tdstDeviceAttributes_* pstDevAttrib, s16* phDev);
extern s32 GLD_bSetViewportPosition(s32 _unused, s16 hVP,
                                    struct GLD_tdstViewportAttributes_* pstViewAttrib);
extern s32 GLD_bCreateViewport(s16 hDev, struct GLD_tdstViewportAttributes_* pstViewAttrib,
                               s16* phVP);
extern s32 GLD_bGetViewportAttributes(s16 _unused, s16 hVP,
                                      struct GLD_tdstViewportAttributes_* pstViewAttrib);
extern s32 GLD_bRequestWriteToViewport(s32 _unused, s16 hVP,
                                       struct GLD_tdstViewportAttributes_* pstViewAttrib,
                                       s32* pbCanWrite);
extern s32 GLD_bEndWriteToDevice(s16 device, void* attributes);
extern s32 GLD_bFlipDeviceWithSynchro(s16 device, s32 value);
extern s32 GLD_bBeginWriteToDevice(s16 hDevice);

extern s32 GLI_xIsGliInit(void);
extern void GLI_xDesinit(void);
extern void GLI_xInitGli(void);

extern f32 inv_GLI_xGetLightShadowIntensity(GLI_tdstLight_* state);
extern u8 inv_GLI_bIsLightShadowCaster(GLI_tdstLight_* state);

extern void func_0014B380(void* viewport, u8 skipFiltered);
extern void inv_GLI_vRecordPopLightsInViewport(s32 count);
extern void inv_GLI_vRecordAddLightToViewport(GLI_tdstLight_* light);
extern void _GLI_vResetTableOfActiveLightsForChar(struct HIE_tdstSuperObject_* _hSprObjSector);

extern void GLI_xSerialLinearOp(s32 lNbOfVertex, MTH3D_tdstVector_* p_stSource,
                                ProjectedVertex* p_stDest, POS_tdstMatrix_* p_stMatrix);

extern void GLI_vRefreshAllCyclingTextures(f32 fCoefOfMultiplication);
extern void GLI_xRefreshAnimatedTexture(GliMaterialData* hMaterial);
extern s32 GLI_lGetCurrentAnimatedTextureNumber(GliMaterialData* hMaterial);
extern u16 GLI_lGetMaterialNumberOfAnimatedTexture(GliMaterialData* hMaterial);
extern void GLI_xGetMaterialAmbientCoef(GliMaterialData* hMaterial, void* p_stColor);
extern void GLI_xGetMaterialColor(const GliMaterialData* hMaterial, GEO_tdstColor_* p_stColor);
extern void GLI_xSetMaterialColor(GliMaterialData* hMaterial, GEO_tdstColor_* p_stColor);
extern void GLI_xSetMaterialType(GliMaterialData* hMaterial, u32 xMType);
extern void GLI_xGetMaterialTextureScrollingCoef(GliMaterialData* material, s32 layerIndex, s32* mode, f32* scrollU,
                          f32* scrollV);
extern GliMaterialData* GLI_pGetVisualMaterialForLetter(u32 letter);
extern s32 inv_GLI_bIsMaterialAnimationEnabled(GliMaterialData* material);
extern void GLI_xGetMaterialTexture(GliMaterialData* material, GliTexture** texture);
extern void GLI_vLockAnimatedTexture(GliMaterialData* _hMaterial);
extern void GLI_vSetCurrentAnimatedTextureNumber(GliMaterialData* hMaterial, u32 lTextureNumber);
extern void GLI_vUnLockAnimatedTexture(GliMaterialData* _hMaterial);
extern void GLI_xSetMaterialAmbientCoef(GliMaterialData* hMaterial, GEO_tdstColor_* p_stColor);
extern void GLI_xSetMaterialTextureScrollingCoef(GliMaterialData* hMaterial, s32 lUnused,
                                                 s32 bSwitch, f32 fUSpeed, f32 fVSpeed);
extern void GLI_xSetMaterialTextureScrollingOnOff(GliMaterialData* hMaterial, s32 bSwitch);
extern void _GLI_xSetMaterialTextureScrollingCoef(GliMaterialData* hMaterial, s32 lUnused,
                                                  s32 bSwitch, f32 fUSpeed, f32 fVSpeed);
extern void inv_GLI_vSetMaterialBackfaceFlag(GliMaterialData* material, s32 enabled);
extern void GLI_xCreateMaterial(GliMaterialData** hMaterial);

extern void GLI_xInitMatrixStack(void);

extern void GLI_xSerialProjection(GLI_tdstCamera_* p_stCam, s32 lNbOfVertex,
                                  ProjectedVertex* p_stSource, ProjectedVertex* p_stDest);

extern void inv_GLI_vRestoreRenderFlags(void);
extern void inv_GLI_vSaveRenderFlags(void);

extern void* func_0014DC70(void);

extern s32 inv_RW_fn_bFileSystemPluginAttach(void);

extern void GLI_xGetCameraAspectAndRatio(const GLI_tdstCamera_* p_stCam, f32* p_xAlphaX,
                                         f32* p_xRatio);
extern void GLI_xAdjustCameraToViewport(s16 viewportIndex, s16 cameraIndex,
                                        GLI_tdstCamera_* camera);
extern void GLI_xSetCameraRatio(GLI_tdstCamera_* p_stCam, f32 xRatio);
extern s32 inv_GLI_fn_bSetBackGroundColor(s32 unused, s32 value);
extern void GLI_xCreateCamera(GLI_tdstCamera_** h_stCam, s32 lCameraMode);
extern void GLI_xSetCameraAspectAndRatio(GLI_tdstCamera_* p_stCam, f32 xAlphaX, f32 xRatio);
extern void GLI_xSetCameraMatrix(GLI_tdstCamera_* camera, POS_tdstMatrix_* matrix);
extern void GLI_xSetViewportCamera(s16 hDevice, s16 hViewport, GLI_tdstCamera_* p_stCam);
extern void GLI_xSetCameraNearFar(GLI_tdstCamera_* p_stCam, f32 xNear, f32 xFar);
extern void GLI_xGetViewportCamera(s16 hDevice, s16 hViewport, GLI_tdstCamera_** p_stCam);
extern void GLI_vGet3DVertexFromScreenPos(struct GLD_tdstViewportAttributes_* viewport, MTH3D_tdstVector_* position, f32 ratioX, f32 ratioY, f32 size);

extern void GLI_vSetLensFlareRef(void* reference);

extern s32 inv_ATO_fn_bLoadLevelAtomicClump(char* name);

extern GLI_tdstLight_* gs_aDEF_p_CurrentListOfLight[];
extern u32 gs_lActualRefreshNumberForScolledTextures;
extern s32 g_lNbMatrixInStack;
extern MTH3D_tdstVector_ stSinEffectAmp;
extern f32 GLI_fF2pow24;
extern f32 GLI_fSinEffectBase;
extern s8 g_cSinusEffectState;
extern RwCamera* inv_GLI_currentCamera;
extern s32 g_lVIDEO_WantedDepth;
extern u32 g_lVIDEO_WantedHeight;
extern RwCamera* inv_GLI_secondaryCamera;
extern u8 inv_GLI_textureAddressState;
extern s16 inv_GLI_fogState;
extern u8 D_00393658;
extern RpClump* inv_RW3_levelClump;
extern RpLight* inv_RW3_singleLight;
extern u8 D_0039371C;
extern s32 inv_RW3_ulViewportLightMask;
extern s32 inv_RW3_enabledDirectionalLights;
extern u32 inv_RW3_enabledPointLights;
extern RpClump* inv_RW3_unsortedClump;
extern RpClump* inv_RW3_sortedClump;
extern RpWorld* RW3_g_pWorld;
extern s32 inv_RW3_lLightmapAtomicPluginOffset;
extern s8 D_003936FC;
extern s8 D_0039371D;
extern void* inv_RW3_p_stLightmapPipeline;
extern RwRaster* inv_RW3_lightmapTemplateRasterR;
extern RwRaster* inv_RW3_lightmapTemplateRasterG;
extern RwRaster* inv_RW3_lightmapTemplateRasterB;
extern RwRaster* inv_RW3_lightmapRasterR;
extern RwRaster* inv_RW3_lightmapRasterG;
extern RwRaster* inv_RW3_lightmapRasterB;
extern DrawQueueEntry inv_GLI_drawQueue[];
extern u32 inv_GLI_texturedRequestCount;
extern u32 inv_GLI_atomicRequestCount;
extern u8 g_bOneShotAnim;
extern u8 g_bTakeCareOfAnimationCoordination;
extern f32 g_BaseTimeForParticleAnimation;
extern u8 g_bHasTextureAnimationFinished;

typedef struct GliDisplayStateView {
    u8 unk00[4];
    u32 flags;
    s32 unk08;
    u8 unk0C[8];
    s32 unk14;
    s32 unk18;
    u8 unk1C[4];
    s32 unk20;
    u8 unk24;
    f32 frameRate;
} GliDisplayStateView;

extern GliDisplayStateView inv_GLI_stDisplayState;

extern void inv_GLI_fn_vPrepareLevelFadeOut(void);

extern u8 GLI_g_bSaveFog;

extern POS_tdstMatrix_* g_p_stCurrentMatrix;

extern struct tdstEngineObject_* g_AlphabetCharacterPointer;
extern GliMaterialData* gs_hDefaultMaterial;
extern s8 inv_GLI_cTextGradientMode;

extern GLI_tdstLight_** inv_RW3_p_pLevelLights;
extern u32 inv_RW3_ulLightmapSlotCount;

extern Quad inv_GLI_singleQuad;

#define MATRIX_STACK_SIZE 100
extern POS_tdstMatrix_ g_aDEF_stMatrixStack[MATRIX_STACK_SIZE];

extern f32 a64_fSinTable[64];

extern u32 g_lVIDEO_WantedWidth;

extern u8 inv_GLI_blendModeState;
extern u8 inv_GLI_zWriteState;
extern u8 inv_GLI_zTestState;

#endif
