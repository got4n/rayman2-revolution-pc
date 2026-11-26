#ifndef REVO_RWPS2_H
#define REVO_RWPS2_H

#include "renderware.h"

typedef s32 RwBool;

struct RwRaster {
    void* parent;
    u8* cpPixels;
    void* palette;
    s32 width;
    s32 height;
    s32 depth;
    s32 stride;
    s16 nOffsetX;
    s16 nOffsetY;
    u8 cType;
    u8 cFlags;
    u8 privateFlags;
    u8 cFormat;
};

typedef struct RxPipelineCluster RxPipelineCluster;
typedef struct RxPipelineNode RxPipelineNode;
typedef struct RxPipeline RxPipeline;

typedef struct RxCluster {
    u16 flags;
    u16 stride;
    void* data;
    void* currentData;
    u32 numAlloced;
    u32 numUsed;
    RxPipelineCluster* clusterRef;
    u32 attributes;
} RxCluster;

typedef struct RxPacket {
    u16 flags;
    u16 numClusters;
    RxPipeline* pipeline;
    u32* inputToClusterSlot;
    u32* slotsContinue;
    RxPipelineCluster** slotClusterRefs;
    RxCluster clusters[1];
} RxPacket;

typedef enum rxEmbeddedPacketState {
    rxPKST_PACKETLESS = 0,
    rxPKST_UNUSED = 1,
    rxPKST_INUSE = 2,
    rxPKST_PENDING = 3
} rxEmbeddedPacketState;

typedef struct RxPipelineNodeParam {
    void* dataParam;
    struct RxHeap* heap;
} RxPipelineNodeParam;

typedef struct RxNodeMethods {
    s32 (*nodeBody)(RxPipelineNode* self, const RxPipelineNodeParam* params);
    void* nodeInit;
    void* nodeTerm;
    void* pipelineNodeInit;
    void* pipelineNodeTerm;
    void* pipelineNodeConfig;
    void* configMsgHandler;
} RxNodeMethods;

typedef struct RxNodeDefinition {
    char* name;
    RxNodeMethods nodeMethods;
    u32 io[5];
    u32 pipelineNodePrivateDataSize;
    s32 editable;
    s32 InputPipesCnt;
} RxNodeDefinition;

struct RxPipelineNode {
    RxNodeDefinition* nodeDef;
    u32 numOutputs;
    u32* outputs;
    RxPipelineCluster** slotClusterRefs;
    u32* slotsContinue;
    void* privateData;
    u32* inputToClusterSlot;
    void* topSortData;
};

struct RxPipeline {
    s32 locked;
    u32 numNodes;
    RxPipelineNode* nodes;
    void* nodesBlock;
    u32 packetNumClusterSlots;
    rxEmbeddedPacketState embeddedPacketState;
    RxPacket* embeddedPacket;
    u32 numInputRequirements;
    void* inputRequirements;
    u32* superBlock;
    u32 entryPoint;
};

typedef struct RxExecutionContext {
    RxPipeline* pipeline;
    RxPipelineNode* currentNode;
    s32 exitCode;
    u32 pad;
    RxPipelineNodeParam params;
} RxExecutionContext;

typedef struct RwResEntry RwResEntry;

struct RwResEntry {
    RwLLLink link;
    u32 size;
    void* owner;
    RwResEntry** ownerRef;
    void (*destroyNotify)(RwResEntry* entry);
};

typedef struct rwPS2FieldRec {
    s32 numVerts;
    s32 dataoffset;
    s16 skip;
    s16 reverse;
    u8 vuoffset;
} rwPS2FieldRec;

typedef struct rwPS2ClusterInstanceInfo {
    s32 attrib;
    s32 stride;
} rwPS2ClusterInstanceInfo;

typedef struct rwPS2BreakOut {
    RxCluster* cluster;
    u32 unk04;
    u32 dataOffset;
    u32 stride;
} rwPS2BreakOut;

typedef struct rwPS2MatInstancePvtData {
    s32 numBreakOuts;
    s32 numVerts;
    rwPS2BreakOut breakOut[8];
    rwPS2ClusterInstanceInfo clusterInfo[8];
    s32 unkC8;
    s32 unkCC;
    s32 unkD0;
    s32 unkD4;
    u8 totallyOpaque;
    u8 numStripes;
    u8 sizeOnVU;
    u8 unkDB;
    s32 stripBatchSize;
    s32 stripBatchesPerTag;
    rwPS2FieldRec stripFieldRec[8];
    s32 listBatchSize;
    s32 listBatchesPerTag;
    rwPS2FieldRec listFieldRec[8];
} rwPS2MatInstancePvtData;

typedef struct rwPS2ResEntryHeader {
    u32 refCnt;
    u32 clrCnt;
    Qword* data;
    u32 serialNum;
    u16 overloadFlags;
    u16 pad0;
    u32 numVerts;
    u32 meshType;
    u32 batchSize;
    u32 batchesPerTag;
    rwPS2FieldRec fieldRec[8];
} rwPS2ResEntryHeader;

typedef struct RpMesh {
    u16* indices;
    u32 numIndices;
    RpMaterial* material;
} RpMesh;

typedef struct RpMeshHeader {
    u32 flags;
    u16 numMeshes;
    u16 serialNum;
} RpMeshHeader;

typedef struct RxPS2Mesh {
    RpMesh* mesh;
    RwResEntry** cacheEntryRef;
} RxPS2Mesh;

typedef struct RxPS2DMASessionRecord {
    u32 objType;
    void* sourceObject;
    u32 inFrustum;
    u16 serialNum;
    u16 unk0E;
} RxPS2DMASessionRecord;

struct RpWorld {
    RwObject object;
    u8 pad08[0x10];
    RpPlaneSector* rootSector;
    u8 pad1C[0x2C];
    RwBBox boundingBox;
};

struct RpWorldSector {
    u8 unk00[0x50];
    RpMeshHeader* meshHeader;
};

typedef struct RpMaterialList {
    RpMaterial** materials;
    s32 numMaterials;
    s32 space;
} RpMaterialList;

struct RpGeometry {
    RwObject object;
    u16 lockedSinceLastInst;
    s16 refCount;
    RpMaterialList matList;
    s32 numTriangles;
    s32 numVertices;
    s32 numMorphTargets;
    RpTriangle* triangles;
    RwRGBA* preLitLum;
    RwTexCoords* texCoords;
    RwSurfaceProperties ignoredSurfaceProps;
    RpMeshHeader* mesh;
    RpMorphTarget* morphTarget;
};

typedef struct GsTex0Bits {
    u64 tbp0 : 14;
    u64 tbw : 6;
    u64 psm : 6;
    u64 tw : 4;
    u64 th : 4;
    u64 tcc : 1;
    u64 tfx : 2;
    u64 cbp : 14;
    u64 cpsm : 4;
    u64 csm : 1;
    u64 csa : 5;
    u64 cld : 3;
} GsTex0Bits;

typedef union GsTex0Reg {
    u64 value;
    GsTex0Bits bits;
    u32 words[2];
} GsTex0Reg;

typedef struct _SkyRasterExt {
    s32 dmaRefCount;
    s32 dmaClrCount;
    GsTex0Reg tex0;
    u32 palOffset;
    u16 mipmapKL;
    u8 maxMipLevel;
    u8 bLocked;
    u32 miptbp1Lsb;
    u32 miptbp1Msb;
    u32 miptbp2Lsb;
    u32 miptbp2Msb;
    u32 sysMemSize;
    u32 sysMemPalSize;
    u32 nTexCacheSize;
    u8 cachePkts;
    u8 lockedMipLevel;
    u8 flags;
    u8 pad[1];
    void* palUploadPkt;
    void* mipUploadPkts[7];
    void* mpCacheEntry;
} _SkyRasterExt;

extern s32 skyRasterExt;

extern u8 D_00393C10;

#endif
