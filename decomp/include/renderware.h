#ifndef REVO_RENDERWARE_H
#define REVO_RENDERWARE_H

#include "ps2/types.h"
#include "renderware_fwd.h"

typedef RpAtomic* (*RpAtomicCallBackRender)(RpAtomic* atomic);
typedef void (*RwObjectHasFrameSyncFunction)(void* object);

struct RwRGBA {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
};

struct RwLLLink {
    RwLLLink* next;
    RwLLLink* prev;
};

typedef struct RwLinkList {
    RwLLLink link;
} RwLinkList;

typedef struct RwObject {
    u8 type;
    u8 subType;
    u8 flags;
    u8 privateFlags;
    void* parent;
} RwObject;

typedef struct RwObjectHasFrame {
    RwObject object;
    RwLLLink lFrame;
    RwObjectHasFrameSyncFunction sync;
} RwObjectHasFrame;

typedef struct RwV2d {
    f32 x;
    f32 y;
} RwV2d;

typedef struct RwTexCoords {
    f32 u;
    f32 v;
} RwTexCoords;

typedef struct RwRGBAReal {
    f32 red;
    f32 green;
    f32 blue;
    f32 alpha;
} RwRGBAReal;

struct RwV3d {
    f32 x;
    f32 y;
    f32 z;
};

typedef struct RwSky2DVertexFields {
    RwV3d scrVertex;
    f32 camVertex_z;
    f32 u;
    f32 v;
    f32 recipZ;
    f32 pad1;
    RwRGBAReal color;
    RwV3d objNormal;
    f32 pad2;
} __attribute__((aligned(8))) RwSky2DVertexFields;

struct RwBBox {
    RwV3d sup;
    RwV3d inf;
};

struct RwSphere {
    RwV3d center;
    f32 radius;
};

struct RpPlaneSector {
    s32 type;
    f32 value;
    RpPlaneSector* leftSubTree;
    RpPlaneSector* rightSubTree;
    f32 leftValue;
    f32 rightValue;
};

struct RpWorldBspStackEntry {
    RpPlaneSector* node;
    RwBBox bbox;
};

struct RwSurfaceProperties {
    f32 ambient;
    f32 specular;
    f32 diffuse;
};

struct RwImage {
    u32 flags;
    s32 width;
    s32 height;
    s32 depth;
    s32 stride;
    u8* cpPixels;
    u8* palette;
};

typedef struct ImageResampleSpan {
    f32 unk0;
    f32 unk4;
    f32 unk8;
    f32 unkC;
} ImageResampleSpan;

typedef struct RwBBoxSectorLocal {
    RwV3d inf;
    RwV3d sup;
} RwBBoxSectorLocal;

typedef struct RpLightChunkInfo {
    f32 radius;
    f32 red;
    f32 green;
    f32 blue;
    f32 minusCosAngle;
    u32 typeAndFlags;
} RpLightChunkInfo;

struct RwPluginRegEntry {
    s32 offset;
    s32 size;
    s32 pluginID;
    void* readCB;
    void* writeCB;
    void* getSizeCB;
    void* (*constructCB)(void*, void*, s32);
    void* (*destructCB)(void*, void*, s32);
    void* copyCB;
    void* errStrCB;
    RwPluginRegEntry* nextRegEntry;
    RwPluginRegEntry* prevRegEntry;
    RwPluginRegistry* parentRegistry;
};

struct RpMorphTarget {
    RpGeometry* parentGeom;
};

struct RpTriangle {
    u16 vertIndex[3];
    s16 matIndex;
};

struct RpGeometrySortVert {
    s32 faceIndex;
    s16 matIndex;
    u16 origIndex;
};

struct RwChunkHeader {
    u32 type;
    u32 length;
    u32 version;
};

struct RwPluginRegistry {
    s32 sizeOfStruct;
    s32 origSizeOfStruct;
    s32 maxSizeOfStruct;
    RwPluginRegEntry* firstRegEntry;
    RwPluginRegEntry* lastRegEntry;
};

typedef struct RpWorldChunkInfoSector {
    s32 matListWindowBase;
    s32 numPolygons;
    s32 numVertices;
    RwV3d inf;
    RwV3d sup;
    s32 collSectorPresent;
    s32 unused;
} RpWorldChunkInfoSector;

typedef struct WorldSectorDiskTriangle {
    u16 vertIndex[3];
    u16 matIndex;
} WorldSectorDiskTriangle;

typedef struct WorldSector {
    s32 type;
    WorldSectorDiskTriangle* triangles;
    f32* vertices;
    u32* preLitLum;
    f32* texCoords;
    u32* normals;
    void* mesh;
    RwLLLink atomics;
    RwLLLink lights;
    RwLLLink collision;
    RwBBoxSectorLocal boundingBox;
    void* colSector;
    void* repEntry;
    void* world;
    s16 matListWindowBase;
    u16 numVertices;
    u16 numTriangles;
    u16 pad5E;
} WorldSector;

typedef void* (*RwMallocFn)(u32 size);
typedef void (*RwFreeFn)(void* ptr);

typedef struct RwMemoryFunctions {
    RwMallocFn rwmalloc;
    RwFreeFn rwfree;
    void* rwrealloc;
    void* rwcalloc;
} RwMemoryFunctions;

typedef struct RwGlobals_inv {
    u8 unk000[0x12C];
    RwMemoryFunctions memoryFunctions_inv;
    void* (*freeListAlloc_inv)(void* freeList);
    void* (*freeListFree_inv)(void* freeList, void* entry);
} RwGlobals_inv;

extern RwGlobals_inv ourGlobals;

struct RpMaterial {
    void* texture;
    RwRGBA color;
    void* pipeline;
    RwSurfaceProperties surfaceProps;
    s16 refCount;
    s16 pad;
};

struct RwMatrixTag {
    RwV3d right;
    u32 flags;
    RwV3d up;
    u32 pad1;
    RwV3d at;
    u32 pad2;
    RwV3d pos;
    u32 pad3;
};

typedef struct RpInterpolator {
    s32 flags;
    s16 startMorphTarget;
    s16 endMorphTarget;
    f32 time;
    f32 recipTime;
    f32 position;
} RpInterpolator;

struct RpAtomic {
    RwObjectHasFrame object;
    void* repEntry;
    RpGeometry* geometry;
    RwSphere boundingSphere;
    RwSphere worldBoundingSphere;
    RpClump* clump;
    RwLLLink inClumpLink;
    RpAtomicCallBackRender renderCallBack;
    RpInterpolator interpolator;
    u16 renderFrame;
    u16 collisionFrame;
    RwLinkList llWorldSectorsInAtomic;
    void* pipeline;
};

struct RpClump {
    RwObject object;
    RwLinkList atomicList;
};

struct RwTexture {
    RwRaster* raster;
};

struct RpLight {
    RwObjectHasFrame object;
    f32 radius;
    RwRGBAReal color;
    f32 minusCosAngle;
};

struct RwCamera {
    RwObjectHasFrame object;
    s32 projectionType;
    void* beginUpdate;
    void* endUpdate;
    RwMatrix viewMatrix;
    void* frameBuffer;
    void* zBuffer;
    RwV2d viewWindow;
    RwV2d recipViewWindow;
    RwV2d viewOffset;
    f32 nearPlane;
    f32 farPlane;
    f32 fogPlane;
    f32 zScale;
    f32 zShift;
};

struct RwStream {
    s32 type;
    s32 accessType;
    s32 position;
    void* handle;
    u32 unk10;
    u32 length;
    u32 unk18;
    void* userData;
    s32 owned;
};

struct RpBuildMesh {
    u32 triangleBufferSize;
    u32 numTriangles;
    void* meshTriangles;
};

struct RwMemory {
    u8* start;
    u32 length;
};

struct RwError {
    s32 pluginID;
    s32 errorCode;
};

typedef void* (*RwFreeListCallback)(void*, void*);
typedef void* (*RwPluginCallback)(void*, s32, s32);
typedef RpWorldSector* (*RpWorldSectorCallback)(RpWorldSector*, void*);
typedef void* (*RpWorldSectorIntersectCallback)(void* data, RpPlaneSector* node, void* moreData);

typedef enum RwRenderState {
    rwRENDERSTATENARENDERSTATE = 0,
    rwRENDERSTATETEXTURERASTER = 1,
    rwRENDERSTATETEXTUREADDRESS = 2,
    rwRENDERSTATETEXTUREADDRESSU = 3,
    rwRENDERSTATETEXTUREADDRESSV = 4,
    rwRENDERSTATETEXTUREPERSPECTIVE = 5,
    rwRENDERSTATEZTESTENABLE = 6,
    rwRENDERSTATESHADEMODE = 7,
    rwRENDERSTATEZWRITEENABLE = 8,
    rwRENDERSTATETEXTUREFILTER = 9,
    rwRENDERSTATESRCBLEND = 10,
    rwRENDERSTATEDESTBLEND = 11,
    rwRENDERSTATEVERTEXALPHAENABLE = 12,
    rwRENDERSTATEBORDERCOLOR = 13,
    rwRENDERSTATEFOGENABLE = 14,
    rwRENDERSTATEFOGCOLOR = 15,
    rwRENDERSTATEFOGTYPE = 16,
    rwRENDERSTATEFOGDENSITY = 17,
    rwRENDERSTATEFOGTABLE = 18,
    rwRENDERSTATEALPHAPRIMITIVEBUFFER = 19
} RwRenderState;

typedef enum RwTextureAddressMode {
    rwTEXTUREADDRESSNATEXTUREADDRESS = 0,
    rwTEXTUREADDRESSWRAP = 1,
    rwTEXTUREADDRESSMIRROR = 2,
    rwTEXTUREADDRESSCLAMP = 3,
    rwTEXTUREADDRESSBORDER = 4
} RwTextureAddressMode;

typedef enum RwTextureFilterMode {
    rwFILTERNAFILTERMODE = 0,
    rwFILTERNEAREST = 1,
    rwFILTERLINEAR = 2,
    rwFILTERMIPNEAREST = 3,
    rwFILTERMIPLINEAR = 4,
    rwFILTERLINEARMIPNEAREST = 5,
    rwFILTERLINEARMIPLINEAR = 6
} RwTextureFilterMode;

typedef enum RwBlendFunction {
    rwBLENDNABLEND = 0,
    rwBLENDZERO = 1,
    rwBLENDONE = 2,
    rwBLENDSRCCOLOR = 3,
    rwBLENDINVSRCCOLOR = 4,
    rwBLENDSRCALPHA = 5,
    rwBLENDINVSRCALPHA = 6,
    rwBLENDDESTALPHA = 7,
    rwBLENDINVDESTALPHA = 8,
    rwBLENDDESTCOLOR = 9,
    rwBLENDINVDESTCOLOR = 10,
    rwBLENDSRCALPHASAT = 11
} RwBlendFunction;

typedef enum RwShadeMode {
    rwSHADEMODENASHADEMODE = 0,
    rwSHADEMODEFLAT = 1,
    rwSHADEMODEGOURAUD = 2
} RwShadeMode;

typedef enum RwFogType {
    rwFOGTYPENAFOGTYPE = 0,
    rwFOGTYPELINEAR = 1,
    rwFOGTYPEEXPONENTIAL = 2,
    rwFOGTYPEEXPONENTIAL2 = 3
} RwFogType;

typedef enum RwVideoModeFlag {
    rwVIDEOMODEEXCLUSIVE = 1,
    rwVIDEOMODEINTERLACE = 2,
    rwVIDEOMODEFFINTERLACE = 4,
    rwVIDEOMODEFSAA0 = 8,
    rwVIDEOMODEFSAA1 = 16
} RwVideoModeFlag;

typedef struct RwVideoMode {
    s32 width;
    s32 height;
    s32 depth;
    s32 flags;
} RwVideoMode;

extern void RwEngineStop(void);
extern void RwEngineClose(void);
extern void RwEngineTerm(void);

extern s32 RwTextureDestroy(void*);
extern s32 RwRasterDestroy(void*);
extern RwFrame* RwCameraGetFrame(RwCamera* camera);
extern void RwFrameDestroy(void*);
extern void RwCameraDestroy(void*);
extern RwRaster* RwCameraGetRaster(RwCamera* camera);
extern RwCamera* RwCameraClear(RwCamera* camera, const RwRGBA* color, s32 clearMode);
extern RwCamera* RwCameraBeginUpdate(RwCamera* camera);
extern RwCamera* RwCameraEndUpdate(RwCamera* camera);
extern s32 RwRenderStateSet(s32 state, void* value);

extern void* RwFreeListForAllUsed(void* list, RwFreeListCallback callback, void* data);
extern void* RwFreeListDestroy(void* list);
extern RwFreeList* RwFreeListCreate(s32 entry_size, s32 entries_per_block, s32 alignment);

extern RwRaster* RwRasterCreate(s32 width, s32 height, s32 depth, s32 flags);
extern u8* RwRasterLock(RwRaster* raster, s32 level, s32 flags);
extern s32 RwRasterGetHeight(RwRaster* raster);
extern u32* RwRasterLockPalette(RwRaster* raster, s32 flags);
extern RwRaster* RwRasterUnlock(RwRaster* raster);
extern RwRaster* RwRasterUnlockPalette(RwRaster* raster);

extern RwFrame* RpLightGetFrame(RpLight* light);
extern RwMatrix* RwFrameGetLTM(RwFrame* frame);
extern RwV3d* RwV3dTransformPoints(RwV3d* out, const RwV3d* in, s32 count, const RwMatrix* matrix);
extern RwStream* RwStreamWrite(RwStream* stream, const void* data, u32 size);

extern s32 _rwerror(s32 code, ...);
extern RwError* RwErrorSet(RwError* error);

extern void j_RpGeometryDestroy(void*, void*);
extern s32 RwEngineRegisterPlugin(s32 size, u32 plugin_id, RwPluginCallback open_callback,
                                  RwPluginCallback close_callback);
extern s32 _rpWorldObjRegisterExtensions(void);
extern void* _rpMaterialOpen(void*, s32, s32);
extern void* _rpMaterialClose(void*, s32, s32);
extern void* _rpMeshOpen(void*, s32, s32);
extern void* _rpMeshClose(void*, s32, s32);
extern void* _rpLightOpen(void*, s32, s32);
extern void* _rpGeometryOpen(void*, s32, s32);
extern void* _rpGeometryClose(void*);
extern RpMaterial* RpGeometryGetMaterial(RpGeometry* geometry, s32 index);
extern RpAtomic* RpAtomicSetFrame(RpAtomic* atomic, RwFrame* frame);
extern void* RpMaterialGetTexture(RpMaterial* material);
extern RpMaterial* RpMaterialSetTexture(RpMaterial* material, void* texture);
extern RpMaterial* RpMaterialSetColor(RpMaterial* material, const RwRGBA* color);

extern RwCamera* RwCameraForAllSectorsInFrustum(RwCamera* camera, RpWorldSectorCallback callback,
                                                void* data);
extern RpWorld* ForAllSphereWorldSectorIntersections(RpWorld* world, RwSphere* sphere,
                                                     RpWorldSectorIntersectCallback callback,
                                                     void* data, void* t0);

extern RwCamera* RwCameraShowRaster(RwCamera* camera, void* dev, u32 flags);
extern s32 RwEngineGetCurrentVideoMode(void);
extern char* RwImageSetPath(char* path);
extern s32 RwRenderStateGet(s32 state, void* value);
extern RpClump* RpClumpCreate(void);
extern RpClump* RpClumpStreamRead(RwStream* stream);
extern RpMorphTarget* RpGeometryGetMorphTarget(RpGeometry* geometry, s32 morphTargetIndex);
extern RpGeometry* RpGeometryLock(RpGeometry* geometry, s32 lockMode);
extern RpGeometry* RpGeometryUnlock(RpGeometry* geometry);
extern s32 RpLightDestroy(RpLight* light);
extern s32 RpLightGetType(RpLight* light);
extern RpMaterial* RpMaterialCreate(void);
extern s32 RpMaterialDestroy(RpMaterial* material);
extern RpWorld* RpWorldAddAtomic(RpWorld* world, RpAtomic* atomic);
extern RpWorld* RpWorldAddClump(RpWorld* world, RpClump* clump);
extern RpWorld* RpWorldAddLight(RpWorld* world, RpLight* light);
extern s32 RpWorldDestroy(RpWorld* world);
extern s32 RpWorldPluginAttach(void);
extern RpWorld* RpWorldSetFlags(RpWorld* world, u32 flags);
extern RpWorld* RpWorldSetRenderOrder(RpWorld* world, s32 renderOrder);
extern f32 RwCameraGetFarClipPlane(RwCamera* camera);
extern s32 RwEngineGetNumVideoModes(void);
extern s32 RwEngineSetVideoMode(s32 modeIndex);
extern s32 RwEngineStart(void);
extern RwFrame* RwFrameAddChild(RwFrame* parent, RwFrame* child);
extern s32 RwFrameDestroyHierarchy(RwFrame* frame);
extern RwFrame* RwFrameRemoveChild(RwFrame* child);
extern RwFrame* RwFrameTranslate(RwFrame* frame, RwV3d* translation, s32 combineOp);
extern f32 RwIm2DGetFarScreenZ(void);
extern f32 RwIm2DGetNearScreenZ(void);
extern RwMatrix* RwMatrixTranslate(RwMatrix* matrix, RwV3d* translation, s32 combineOp);
extern s32 RwResourcesEmptyArena(void);
extern s32 RwResourcesSetArenaSize(s32 size);
extern s32 RwStreamClose(RwStream* stream, void* pData);
extern s32 RwStreamFindChunk(RwStream* stream, u32 type, u32* lengthOut, u32* versionOut);
extern RwStream* RwStreamOpen(s32 type, s32 accessType, void* pData);
extern RwTexture* RwTextureRead(char* name, char* maskName);
extern RwTexture* RwTextureSetAddressingV(RwTexture* texture, s32 addressing);
extern RwTexture* RwTextureSetFilterMode(RwTexture* texture, s32 filtering);
extern RwTexture* RwTextureSetRaster(RwTexture* texture, RwRaster* raster);
extern void skyPrepareUploadRaster(RwRaster* raster);

#endif
