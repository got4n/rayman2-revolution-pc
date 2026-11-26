#include "common.h"

#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "rwps2_macros.h"
#include "renderware.h"
#include "rwps2.h"

#define LIGHTMAP_VU_CODE_COUNT 32
#define LIGHTMAP_PALETTE_SIZE 0x40

typedef struct PipelineNodeDefinition {
    char* name;
} PipelineNodeDefinition;

extern u8 D_003936E9;
extern u8 dword_235B70[];
extern u8 dword_239080[];
extern u8 dword_23CC30[];
extern u8 dword_23D5F0[];
extern u8 dword_23E250[];
extern void* inv_RW3_lightmapVuCode[LIGHTMAP_VU_CODE_COUNT];
extern u8 off_372EA8[];
extern u8 off_372EB8[];
extern u8 off_372EC8[];
extern u8 off_372EE8[];

s32 RwRasterGetWidth(void* raster);
s32 func_0017DA18(void* node, s32 a1, s32 a2, s32 a3);
s32 func_0017DA38(void* node, void* cluster, s32 index);
s32 RxPipelineNodePS2MatBridgeSetVU1CodeArray(void* node, void* vuCode);
void* inv_RW3_PIPE_fn_pAddFragments(void* pipeline, s32 a1, PipelineNodeDefinition* first,
                                    PipelineNodeDefinition* second, void* a4, void* a5, s32 a6);
#ifdef PC_PORT
u32 fptoui(f32 value);
#endif
u32 inv_RW3_fn_lGetRasterTbp0(void* raster);
PipelineNodeDefinition* func_0017E5F8(void);
void* RxPipelineNodePS2MatBridgeSetVIFOffset(void* node, s32 a1);
PipelineNodeDefinition* RxNodeDefinitionGetPS2ObjAllInOne(s32 a0);
void* func_001826F8(void* node, s32 a1);
void* inv_RW3_PIPE_fn_lUnlockPipeline(void* pipeline);
void* inv_RW3_PIPE_fn_pCreatePipeline(void);
void* inv_RW3_PIPE_fn_pFindNodeByIndex(void* pipeline, char* name, void* start, s32* index);
void* inv_RW3_PIPE_fn_pLockPipeline(void* pipeline);
PipelineNodeDefinition* inv_RW3_SKYWORLD_fn_pGetStaticNodeDef_181830(void);

void func_00150498(void)
{
    s32 i;
    void* pipeline;
    void* lockedPipeline;
    PipelineNodeDefinition* nodeDef;
    PipelineNodeDefinition* staticNodeDef;
    void* fragment0;
    void* fragment1;
    void* node;

    for (i = LIGHTMAP_VU_CODE_COUNT - 1; i >= 0; i--) {
        inv_RW3_lightmapVuCode[i] = dword_23E250;
    }
    inv_RW3_lightmapVuCode[0] = dword_23CC30;
    inv_RW3_lightmapVuCode[1] = dword_23D5F0;
    inv_RW3_lightmapVuCode[2] = dword_235B70;
    inv_RW3_lightmapVuCode[3] = dword_239080;
    pipeline = inv_RW3_PIPE_fn_pCreatePipeline();
    if (pipeline == NULL) {
        return;
    }

    lockedPipeline = inv_RW3_PIPE_fn_pLockPipeline(pipeline);
    if (lockedPipeline != NULL) {
        nodeDef = RxNodeDefinitionGetPS2ObjAllInOne(1);
        staticNodeDef = inv_RW3_SKYWORLD_fn_pGetStaticNodeDef_181830();
        fragment0 = func_0017E5F8();
        fragment1 = func_00151338();
        lockedPipeline = inv_RW3_PIPE_fn_pAddFragments(lockedPipeline, 0, nodeDef, staticNodeDef,
                                                       fragment1, fragment0, 0);
        node = inv_RW3_PIPE_fn_pFindNodeByIndex(lockedPipeline, staticNodeDef->name, 0, 0);
        if (func_0017DA38(node, off_372EA8, 0) == 0) {
            printf("%s(%d): ERROR --> Cannot generate Instance for Cluster : %s!!\n",
                   "rw3_pp_lightmap.c", 0x21A, "&RxClPS2xyz");
            while (1) {
            }
        }
        D_003936E9++;
        if (func_0017DA38(node, off_372EB8, 1) == 0) {
            printf("%s(%d): ERROR --> Cannot generate Instance for Cluster : %s!!\n",
                   "rw3_pp_lightmap.c", 0x21B, "&RxClPS2uv");
            while (1) {
            }
        }
        D_003936E9++;
        if (func_0017DA38(node, off_372EC8, 2) == 0) {
            printf("%s(%d): ERROR --> Cannot generate Instance for Cluster : %s!!\n",
                   "rw3_pp_lightmap.c", 0x21C, "&RxClPS2rgba");
            while (1) {
            }
        }
        D_003936E9++;
        if (func_0017DA38(node, off_372EE8, 4) == 0) {
            printf("%s(%d): ERROR --> Cannot generate Instance for Cluster : %s!!\n",
                   "rw3_pp_lightmap.c", 0x21F, "&RxClPS2user1");
            while (1) {
            }
        }
        D_003936E9++;
        func_0017DA18(node, 4, 0x41, 0x15);
        lockedPipeline = inv_RW3_PIPE_fn_lUnlockPipeline(lockedPipeline);
        RxPipelineNodePS2MatBridgeSetVIFOffset(
            inv_RW3_PIPE_fn_pFindNodeByIndex(lockedPipeline, func_0017E5F8()->name, 0, 0), 0x1DA);
        func_001826F8(inv_RW3_PIPE_fn_pFindNodeByIndex(lockedPipeline, nodeDef->name, 0, 0), 1);
        if (lockedPipeline == NULL) {
            printf("%s(%d): Error building pipe\n", "rw3_pp_lightmap.c", 0x24D);
        }
    }

    if (RxPipelineNodePS2MatBridgeSetVU1CodeArray(
            inv_RW3_PIPE_fn_pFindNodeByIndex(lockedPipeline, func_0017E5F8()->name, 0, 0),
            inv_RW3_lightmapVuCode) == 0) {
        printf("%s(%d): Failed to set vucode array\n", "rw3_pp_lightmap.c", 0x25D);
    }
    inv_RW3_p_stLightmapPipeline = lockedPipeline;
}

#define OUTSIDE_PALETTE(address, palette)                                                          \
    ((address) < (palette) || (address) >= (palette) + LIGHTMAP_PALETTE_SIZE)

void func_00150808(void* raster, s32 slot)
{
    u32 address = 0;
    u32 baseAddress = inv_RW3_fn_lGetRasterTbp0(inv_RW3_lightmapPaletteRaster);
    f32 addressValue;
    f32 baseValue;
    s32 width;
    s32 height;
    u32 size;

    if (raster != NULL) {
        address = inv_RW3_fn_lGetRasterTbp0(raster);
    }
    if (address == 0) {
        return;
    }
    if (OUTSIDE_PALETTE(address, inv_RW3_lightmapVramR) &&
        OUTSIDE_PALETTE(address, inv_RW3_lightmapVramG) &&
        OUTSIDE_PALETTE(address, inv_RW3_lightmapVramB) && baseAddress != address) {
        if (address < baseAddress) {
            return;
        }
        addressValue = address;
        width = RwRasterGetWidth(inv_RW3_lightmapPaletteRaster);
        height = RwRasterGetHeight(inv_RW3_lightmapPaletteRaster);
        baseValue = baseAddress;
#ifdef PC_PORT
        size = fptoui(width * 0.25f * height) + 0x100;
#else
        size = (u32) (width * 0.25f * height) + 0x100;
#endif
        if (!(addressValue < baseValue + size * (1.0f / 64.0f))) {
            return;
        }
    }
    printf("ERROR --> LM raster overwritten in VRAM\n");
    printf("      --> Orig  = 0x%X\n", address);
    printf("      --> RED   = 0x%X - 0x%X \n", inv_RW3_lightmapVramR,
           inv_RW3_lightmapVramR + LIGHTMAP_PALETTE_SIZE);
    printf("      --> GREEN = 0x%X - 0x%X \n", inv_RW3_lightmapVramG,
           inv_RW3_lightmapVramG + LIGHTMAP_PALETTE_SIZE);
    printf("      --> BLUE  = 0x%X - 0x%X \n", inv_RW3_lightmapVramB,
           inv_RW3_lightmapVramB + LIGHTMAP_PALETTE_SIZE);
    printf("      --> PAL   = 0x%X\n", baseAddress);
}

typedef struct LmUV {
    f32 u;
    f32 v;
} LmUV;

typedef struct LmVert {
    LmUV uv;
    u8 pad[8];
} LmVert;

static inline RxCluster* inv_GLI_fn_p_stLmGetNode(RxPacket* obj, s32 i)
{
    RxCluster* n;

    if ((s32) obj->inputToClusterSlot[i] != -1) {
        n = (RxCluster*) ((u8*) obj + obj->inputToClusterSlot[i] * 28 + 0x14);
        n->currentData = n->data;
        return &obj->clusters[obj->inputToClusterSlot[i]];
    }
    return NULL;
}

extern RxExecutionContext _rxExecCtxGlobal;

static inline RxPacket* inv_GLI_fn_p_stLmNextObject(void)
{
    if (_rxExecCtxGlobal.pipeline->embeddedPacketState == rxPKST_PENDING) {
        _rxExecCtxGlobal.pipeline->embeddedPacketState = rxPKST_INUSE;
        return _rxExecCtxGlobal.pipeline->embeddedPacket;
    }
    return NULL;
}

typedef union LmWords {
    u64 d[2];
    u32 w[4];
} LmWords;

extern u64 D_00393C18;
extern u32 qword_378830[];
extern u32 dword_3789A0[];
extern u32 qword_378840[];

void func_001A4140(RxCluster* node);
void skyTexCacheRasterLock(void* raster, s32 lock);
void inv_RW3_fn_vUploadRaster(void* a0, u32 a1);
void sweFinaliseOpenLocalPkt(u32 a0, s32 a1);
RxPipelineNode* inv_RW3_IM3D_fn_lClearUnusedClusterEntries(RxPipeline* set, s32 a1, s32 a2);
void inv_RW3_IM3D_fn_vClearAllClusterEntries(RxPacket* obj);

static inline RxPS2Mesh* inv_GLI_fn_p_stMeshInstanceOf(RxCluster* node)
{
    return node->currentData;
}

static inline s32* inv_GLI_fn_p_lLightmapSlotOf(void* atomic)
{
    void* data = (u8*) atomic + inv_RW3_lLightmapAtomicPluginOffset;

    return data;
}

static inline RwTexture* inv_GLI_fn_p_stTextureOf(RpMaterial* material)
{
    return RpMaterialGetTexture(material);
}

#define LM_FLUSH_QUEUE(arg)                                                                        \
    do {                                                                                           \
        set = _rxExecCtxGlobal.pipeline;                                                           \
        if (_rxExecCtxGlobal.exitCode != 0) {                                                      \
            h = inv_RW3_IM3D_fn_lClearUnusedClusterEntries(set, (arg), 0);                         \
            if (h != NULL) {                                                                       \
                if (h->nodeDef->nodeMethods.nodeBody(h, &_rxExecCtxGlobal.params) == 0) {          \
                    _rxExecCtxGlobal.exitCode = 0;                                                 \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
        if (set->embeddedPacketState >= rxPKST_INUSE) {                                            \
            set->embeddedPacketState = rxPKST_INUSE;                                               \
            inv_RW3_IM3D_fn_vClearAllClusterEntries(set->embeddedPacket);                          \
        }                                                                                          \
    } while (0)

s32 func_00150A40(s32 arg0)
{
    volatile LmWords t;
    volatile u128 q;
    u32 color;
    RxPacket* obj;
    RxCluster* n0;
    RxCluster* n1;
    RxCluster* n2;
    RxPS2DMASessionRecord* info;
    RpMesh* mesh;
    LmUV* uvs;
    void* raster;
    void* baseRaster;
    s32 slot;
    u32 prim;
    u32 tag;
    u64 primWord;
    RxPipelineNode* h;
    RxPipeline* set;

    color = 0;
    RwRenderStateGet(rwRENDERSTATEFOGCOLOR, &color);
    while ((obj = inv_GLI_fn_p_stLmNextObject()) != NULL) {
        n0 = NULL;
        n1 = NULL;
        raster = NULL;
        baseRaster = NULL;
        n0 = inv_GLI_fn_p_stLmGetNode(obj, 0);
        n1 = inv_GLI_fn_p_stLmGetNode(obj, 1);
        n2 = inv_GLI_fn_p_stLmGetNode(obj, 2);

        info = n0->currentData;
        mesh = inv_GLI_fn_p_stMeshInstanceOf(n1)->mesh;
        slot = *inv_GLI_fn_p_lLightmapSlotOf(info->sourceObject);
        uvs = inv_RW3_lightmapSlots[slot].uvs;
        if (RpMaterialGetTexture(mesh->material) != NULL) {
            baseRaster = inv_GLI_fn_p_stTextureOf(mesh->material)->raster;
        }

        if ((n2->flags & 1) &&
            info->serialNum !=
                ((rwPS2ResEntryHeader*) (*inv_GLI_fn_p_stMeshInstanceOf(n1)->cacheEntryRef + 1))
                    ->serialNum) {
            s32 n = mesh->numIndices;
            u16* idx = mesh->indices;
            LmVert* dst = n2->currentData;

            while (n > 0) {
                dst->uv = uvs[*idx];
                idx++;
                n--;
                memset(dst->pad, 0, 8);
                dst++;
            }
        }

        func_001A4140(n2);
        {
            RpMaterial* material;
            RwTexture* texture;

            t.d[0] = 0;
            material = mesh->material;
            t.d[1] = 0;
            q = 0;
            texture = RpMaterialGetTexture(material);
            if (texture != NULL) {
                raster = texture->raster;
            }
        }
        if (raster != NULL) {
            skyTexCacheRasterLock(raster, 1);
        }

        if (!D_003936FC) {
            inv_RW3_fn_vUploadRaster(inv_RW3_lightmapSlots[slot].r, inv_RW3_lightmapVramR);
            inv_RW3_fn_vUploadRaster(inv_RW3_lightmapSlots[slot].g, inv_RW3_lightmapVramG);
            inv_RW3_fn_vUploadRaster(inv_RW3_lightmapSlots[slot].b, inv_RW3_lightmapVramB);
        }

        sweFinaliseOpenLocalPkt(0x88000501, -0x16);
        if (inv_GLI_packetCursor == NULL) {
            printf("%s(%d): DMA memory failure\n", "rw3_pp_lightmap.c", 0x380);
        } else {
            t.d[0] = 0x10000014;
            t.d[1] = 0x6C0D03B501000404ULL;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            tag = 0x30004000;
            t.d[0] = 0x312E400000000000ULL;
            t.d[1] = 0x412;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            if (D_0039371D != 2 && baseRaster != NULL) {
                prim = 0x5C;
            } else {
                prim = 0x4C;
            }
            primWord = (prim << 15) | tag;
            t.d[0] = primWord << 32;
            t.d[1] = 0x412;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            if (baseRaster != NULL) {
                inv_RW3_fn_bWriteRasterTex0(baseRaster, 0, 0);
            } else {
                t.d[0] = t.d[1] = 0;
                RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
                *(u128*) inv_GLI_packetCursor++ = q;
            }

            t.d[0] = 0x3126400000000000ULL;
            t.d[1] = 0x412;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0x44;
            t.d[1] = 0x43;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            {
                u32 r = (color >> 16) & 0xFF, g = (color >> 8) & 0xFF, b = color & 0xFF,
                    a = color >> 24;
                t.w[3] = a;
                ((LmWords*) &t)->w[2] = b;
                ((LmWords*) &t)->w[0] = r;
                ((LmWords*) &t)->w[1] = g;
            }
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0x1000000000008001ULL;
            t.d[1] = 0xE;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            if (D_00393C10) {
                t.w[0] = qword_378830[0];
            } else {
                t.w[0] = dword_3789A0[0];
            }
            t.d[1] = 0x4D;
            t.w[1] = 0xFFFFFF00;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            ((LmWords*) &t)->w[1] = 0xFFFF00FF;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            ((LmWords*) &t)->w[1] = 0xFF00FFFF;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            if (D_003936FC) {
                inv_RW3_fn_vCopyRasterSize(inv_RW3_lightmapRasterR, inv_RW3_lightmapTemplateRasterR);
                inv_RW3_fn_vCopyRasterSize(inv_RW3_lightmapRasterG, inv_RW3_lightmapTemplateRasterG);
                inv_RW3_fn_vCopyRasterSize(inv_RW3_lightmapRasterB, inv_RW3_lightmapTemplateRasterB);
                inv_RW3_fn_bWriteRasterTex0(inv_RW3_lightmapRasterR, 0, 1);
                inv_RW3_fn_bWriteRasterTex0(inv_RW3_lightmapRasterG, 0, 1);
                inv_RW3_fn_bWriteRasterTex0(inv_RW3_lightmapRasterB, 0, 1);
            } else {
                inv_RW3_fn_vCopyRasterSize(inv_RW3_lightmapRasterR, inv_RW3_lightmapSlots[slot].r);
                inv_RW3_fn_vCopyRasterSize(inv_RW3_lightmapRasterG, inv_RW3_lightmapSlots[slot].g);
                inv_RW3_fn_vCopyRasterSize(inv_RW3_lightmapRasterB, inv_RW3_lightmapSlots[slot].b);
                inv_RW3_fn_bWriteRasterTex0(inv_RW3_lightmapRasterR, 0, 1);
                inv_RW3_fn_bWriteRasterTex0(inv_RW3_lightmapRasterG, 0, 1);
                inv_RW3_fn_bWriteRasterTex0(inv_RW3_lightmapRasterB, 0, 1);
            }

            t.d[0] = 0;
            t.d[1] = 0x5000000600000000ULL;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0x1000000000008005ULL;
            t.d[1] = 0xE;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0x89;
            t.d[1] = 0x43;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0x5;
            t.d[1] = 0x9;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = D_00393C18 & ~0x401ULL;
            t.d[0] |= 0x50000;
            t.d[1] = 0x48;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = qword_378840[0];
            t.d[1] = 0x4F;
            {
                u64 lo = ((LmWords*) &t)->d[0];
                u64 hi = ((LmWords*) &t)->d[1];
                ((LmWords*) &t)->d[0] = lo | 0x100000000ULL;
                RWPS2_MAKE_QWORD(q, hi, t.d[0]);
            }
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0x31;
            t.d[1] = 0x15;
            RWPS2_MAKE_QWORD(q, t.d[1], t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            t.d[0] = 0xF0000000;
            RWPS2_MAKE_QWORD(q, 0, t.d[0]);
            *(u128*) inv_GLI_packetCursor++ = q;

            sweFinaliseOpenLocalPkt(1, 0);
        }

        LM_FLUSH_QUEUE(arg0);
        func_00150808(raster, slot);
        if (raster != NULL) {
            skyTexCacheRasterLock(raster, 0);
        }
    }
    return 1;
}
