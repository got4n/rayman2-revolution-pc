#include "common.h"

#include "GLI/GLI.h"

#include "MTH/MTH.h"
#include "renderware.h"
#include "sdk.h"
#include "GAM/GAM.h"
extern s32 D_00393424;
extern s32 D_00393428;
extern s32 D_0039342C;
extern s32 inv_GLI_vertex2DCount;
extern s8 D_00393413;
extern u32 D_00393B24;
extern u32 D_00393B40;
extern RwCamera* inv_GLI_primaryCamera;

f32 RwCameraGetNearClipPlane(void*);
f32 tanf(f32);
f32* RwCameraGetViewWindow(void*);
s32 GLI_RW_bVideoModeSelect(void);
s32 GLI_RW_pCameraCreate(s32, s32, f32*);
s32 RwEngineOpen(void*);
s32 RwImageRegisterImageFormat(void*, void*, void*);
s32 RwRasterGetStride(void*);
s32 RwRasterGetWidth(void*);
s32 func_00192390(void*, void*, void*);
s32 func_001ABF78(u32, u32, u32);
s32 inv_GLI_RW_bCreateRasterizerResources(s32, s32, s32, s32);
s32 inv_GLI_RW_bRegisterImageFormats(void);
s32 inv_GLI_RW_bDestroyRasterizerResources(s32);
void GLI_RW_xDesinitAPI(void);
void* RwCameraSetFogDistance(void* camera, f32 distance);
void RwEngineInit(void*, s32);
void RwIm2DRenderPrimitive(s32, void*, s32);
void inv_GLI_RW_vCreateSecondaryCamera(void);
void inv_GLI_RW_vResizeCameraViewport(RwCamera*, s32*, f32, f32);
RwCamera* RwCameraCreate(void);
RwRaster* RwCameraGetZRaster(void*);
RwCamera* RwCameraSetFarClipPlane(void*, f32);
RwCamera* RwCameraSetFrame(void*, void*);
RwCamera* RwCameraSetNearClipPlane(void*, f32);
RwCamera* RwCameraSetRaster(void*, void*);
RwCamera* RwCameraSetZRaster(void*, void*);
RwFrame* RwFrameCreate(void);
void* RwRasterGetParent(void*);
RwRaster* RwRasterSubRaster(void*, void*, s32*);

void RwCameraSetViewWindow(void* camera, f32* viewWindow);
void* RwEngineGetVideoModeInfo(void* modeInfo, s32 modeIndex);

void inv_GLI_RW_vReset2DVertexBuffer(void)
{
    inv_GLI_vertex2DCount = 0;
}

void inv_GLI_RW_vAdd2DRectangle(s32 x0, s32 y0, s32 x1, s32 y1)
{
    typedef union ColorBits {
        f32 f;
        u8 b[4];
        u32 u;
    } ColorBits;

    typedef struct Vtx64 {
        f32 x;
        f32 y;
        f32 z;
        u8 pad0C[0x0C];
        f32 rhw;
        u8 pad1C[4];
        ColorBits c;
        f32 c1;
        f32 c2;
        f32 c3;
        u8 pad30[0x10];
    } Vtx64;

    extern Vtx64 inv_GLI_vertices2D[];

    f32 rhw;
    f32 z;
    f32 fx0, fy0, fx1, fy1;
    Vtx64* vertex;

    rhw = 1.0f / RwCameraGetNearClipPlane(inv_GLI_primaryCamera);
    if (inv_GLI_vertex2DCount + 6 < 25) {
        vertex = &inv_GLI_vertices2D[inv_GLI_vertex2DCount];
        memset(vertex, 0, 0x180);

        vertex->c.u = 0;
        fx0 = (f32) x0;
        fy1 = (f32) y1;
        fy0 = (f32) y0;
        z = rhw * 65535.0f;
        vertex->x = fx0;
        vertex->c1 = (f32) vertex->c.b[1];
        vertex->c2 = (f32) vertex->c.b[0];
        fx1 = (f32) x1;
        vertex->c3 = (f32) vertex->c.b[3];
        vertex->c.f = (f32) vertex->c.b[2];
        vertex->y = fy1;
        vertex->rhw = rhw;
        vertex->z = z;

        vertex++;
        vertex->c.u = 0;
        vertex->x = fx0;
        vertex->y = fy0;
        vertex->c1 = (f32) vertex->c.b[1];
        vertex->c2 = (f32) vertex->c.b[0];
        vertex->c3 = (f32) vertex->c.b[3];
        vertex->c.f = (f32) vertex->c.b[2];
        vertex->rhw = rhw;
        vertex->z = z;

        vertex++;
        inv_GLI_vertex2DCount += 6;
        vertex->c.u = 0;
        vertex->x = fx1;
        vertex->y = fy0;
        {
            u8 q1 = vertex->c.b[1];
            u8 q0 = vertex->c.b[0];
            vertex->c1 = (f32) q1;
            *(volatile f32*) &vertex->c2 = (f32) q0;
        }
        {
            f32 q3 = (f32) vertex->c.b[3];
            f32 q2 = (f32) * (volatile u8*) &vertex->c.b[2];
            vertex->c3 = q3;
            vertex->c.f = q2;
        }
        vertex->rhw = rhw;
        vertex->z = z;

        vertex++;
        vertex->c.u = 0;
        vertex->y = fy0;
        vertex->x = fx1;
        vertex->c1 = (f32) vertex->c.b[1];
        vertex->c2 = (f32) vertex->c.b[0];
        vertex->c3 = (f32) vertex->c.b[3];
        vertex->c.f = (f32) vertex->c.b[2];
        vertex->rhw = rhw;
        vertex->z = z;

        vertex++;
        vertex->c.u = 0;
        vertex->x = fx1;
        vertex->y = fy1;
        vertex->c1 = (f32) vertex->c.b[1];
        vertex->c2 = (f32) vertex->c.b[0];
        vertex->c3 = (f32) vertex->c.b[3];
        vertex->c.f = (f32) vertex->c.b[2];
        vertex->rhw = rhw;
        vertex->z = z;

        vertex++;
        vertex->x = fx0;
        *(volatile f32*) &vertex->y = fy1;
        *(volatile u32*) &vertex->c.u = 0;
        vertex->z = z;
        vertex->rhw = rhw;
        vertex->c1 = (f32) vertex->c.b[1];
        vertex->c2 = (f32) vertex->c.b[0];
        vertex->c3 = (f32) vertex->c.b[3];
        vertex->c.f = (f32) vertex->c.b[2];
    }
}

void inv_GLI_RW_vFlush2DVertexBuffer(void)
{
#ifndef REVO_UNK_32FE20_VTX
    extern u8 inv_GLI_vertices2D[];
#endif

    s32 rs1;
    s32 rsE;
    s32 rs6;
    s32 rs8;

    if (inv_GLI_vertex2DCount == 0) {
        return;
    }
    RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &rs1);
    RwRenderStateGet(rwRENDERSTATEFOGENABLE, &rsE);
    RwRenderStateGet(rwRENDERSTATEZTESTENABLE, &rs6);
    RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, &rs8);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
    inv_GLI_fogState = 0;
    RwIm2DRenderPrimitive(3, inv_GLI_vertices2D, inv_GLI_vertex2DCount);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) rs1);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) rsE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) rs6);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) rs8);
}

void inv_GLI_RW_vApplyFogParameters(tdstFogParams_* fog)
{
    s32 red;
    s32 green;
    s32 blue;
    f32 intensityLow;
    f32 nearDistance;
    f32 farDistance;
    f32 slope;
    f32 span;
    f32 offset;

    if (fog == NULL) {
        return;
    }

    intensityLow = fog->xBlendStart;
    span = fog->xBlendEnd - intensityLow;
    nearDistance = fog->xDepthStart;
    farDistance = fog->xDepthEnd;
    if (span != 0.0f) {
        slope = (farDistance - nearDistance) / span;
        offset = intensityLow * slope;
        slope = slope * 255.0f;
        nearDistance = nearDistance - offset;
        farDistance = nearDistance + slope;
        fog->xInfinite = farDistance;
        if (500.0f < farDistance) {
            fog->xInfinite = 500.0f;
        }
        inv_RW3_fn_vSendFogDistances(nearDistance, farDistance);
        RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*) rwFOGTYPELINEAR);
        RwCameraSetFogDistance(inv_GLI_currentCamera, nearDistance);
    } else {
        printf("ERROR --> Somebody ask for a FOG Intensity range of ZERO (rw3_domat.c) !!\n");
    }

    {
        f32 r = fog->uColor.stColor.xR;
        f32 g = fog->uColor.stColor.xG;
        f32 b = fog->uColor.stColor.xB;
        f32 scaledRed = r * 255.0f;
        f32 scaledGreen = g * 255.0f;
        f32 scaledBlue = b * 255.0f;

        red = inv_MTH_fn_lToInteger(scaledRed);
        green = inv_MTH_fn_lToInteger(scaledGreen);
        blue = inv_MTH_fn_lToInteger(scaledBlue);
    }
    red = (red > -1) ? red : 0;
    green = (green > -1) ? green : 0;
    blue = (blue > -1) ? blue : 0;
    red = red << 16;
    red = red | blue;
    green = green << 8;
    green = green | (s32) 0xFF000000;
    RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void*) (red | green));
}

void GLI_RW_vFogOn(void)
{
    s32 mode4;

    if (inv_GLI_zWriteState == 0) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
    }

    mode4 = 4;
    if (inv_GLI_blendModeState != (u8) mode4) {
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
        inv_GLI_blendModeState = (u8) mode4;
    }
}

s32 inv_GLI_RW_bRender2DVertexBuffer(void)
{
    if (RwCameraBeginUpdate((*&inv_GLI_currentCamera)) != 0) {
        GLI_RW_vFogOn();
        if (inv_GLI_blendModeState != 0) {
            inv_GLI_blendModeState = 0;
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = 1;
            inv_GLI_blendModeState = 0;
        }
        inv_GLI_RW_vFlush2DVertexBuffer();
        RwCameraEndUpdate((*&inv_GLI_currentCamera));
        return 1;
    }
    return 0;
}

s32 func_0011BC98(void)
{
    if (RW3_g_pWorld != 0) {
        inv_RW3_fn_vResetAtomicRequests();
    }
    return 1;
}

void GLI_RW_vDoMaterialSelection(void)
{
    GLI_tdstRenderContext* ctx;
    s32 active;
    GliTexture* tex;
    GliTexture* activeTex;
    GLI_tdstRenderContext* ctx2;
    u32 work;
    s32 addr8;
    s32 addr;
    s32 mode;
    s32 mode4;
    RwTexture* texture;

    active = 0;
    ctx = inv_GLI_renderContext;
    work = (u32) ctx->currentTexture;
    activeTex = (GliTexture*) work;
    if (work != 0) {
        active = activeTex->loaded != 0;
    }

    ctx->unk20AC = 0;
    if (ctx->unk20B0 != 0) {
        ctx->unk20B0 = 3;
    } else {
        if (active != 0) {
            if ((activeTex->flags & 0x38) != 0) {
                goto block_9;
            }
        }
    }
    work = ctx->lCurrentDrawMask;
    if ((work & 8) == 0) {
        if (ctx->globalAlpha < 0.98f) {
            goto block_9;
        }
    }
    if ((ctx->currentMaterial->type & 0x04000000) != 0) {
        goto block_main;
    }

block_9:
    ctx->inv_lDrawFlags |= GLI_DRAW_DEFERRED_inv;
    if (active != 0) {
        if ((activeTex->flags & 8) == 0) {
            ctx->unk20AC = 1;
        }
    }
    return;

block_main:
    ctx->inv_lDrawFlags &= ~GLI_DRAW_DEFERRED_inv;

    if ((work & 0x40000000) != 0) {
        if (inv_GLI_zTestState == 0) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
        }
    } else if (inv_GLI_zTestState != 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
        inv_GLI_zTestState = 0;
    }

    ctx2 = inv_GLI_renderContext;
    if (ctx2->fogEnabled != 0) {
        tdstFogParams_* fogParam;

        fogParam = ((volatile GLI_tdstRenderContext*) ctx2)->fogParameters;
        if (((volatile GLI_tdstRenderContext*) ctx2)->appliedFogParameters != fogParam) {
            inv_GLI_RW_vApplyFogParameters(fogParam);
            ctx2 = inv_GLI_renderContext;
            ctx2->appliedFogParameters = ctx2->fogParameters;
        }
        if (inv_GLI_fogState == 0) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 1);
            inv_GLI_fogState = 1;
        }
    } else if (inv_GLI_fogState != 0) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
        inv_GLI_fogState = 0;
    }

    if ((ctx->lCurrentDrawMask & 0x08000000) != 0) {
        if (inv_GLI_zWriteState == 0) {
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = 1;
        }
    } else if (inv_GLI_zWriteState != 0) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
        inv_GLI_zWriteState = 0;
    }

    if (active != 0) {
        tex = ctx->currentTexture;
        if ((tex->flags & 0x100) != 0) {
            mode = 5;
            if (inv_GLI_blendModeState != mode) {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                inv_GLI_blendModeState = (u8) mode;
                tex = ctx->currentTexture;
            }
        } else if ((ctx->unk20B0 & 2) != 0) {
            mode = 3;
            if (inv_GLI_blendModeState != mode) {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                inv_GLI_blendModeState = (u8) mode;
                tex = ctx->currentTexture;
            }
        } else {
            mode = 4;
            if (inv_GLI_blendModeState != mode) {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
                inv_GLI_blendModeState = (u8) mode;
                tex = ctx->currentTexture;
            }
        }

        addr8 = tex->addressMode;
        addr = addr8 & 0xFF;
        if (addr != inv_GLI_textureAddressState) {
            inv_GLI_textureAddressState = (u8) addr8;
            switch (addr) {
            default:
                tex = ctx->currentTexture;
                break;
            case 0:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSCLAMP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSCLAMP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void*) rwTEXTUREADDRESSCLAMP);
                tex = ctx->currentTexture;
                break;
            case 1:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSCLAMP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void*) rwTEXTUREADDRESSWRAP);
                tex = ctx->currentTexture;
                break;
            case 2:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void*) rwTEXTUREADDRESSCLAMP);
                tex = ctx->currentTexture;
                break;
            case 3:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void*) rwTEXTUREADDRESSWRAP);
                tex = ctx->currentTexture;
                break;
            }
        }

        texture = tex->rwTexture;
        if (texture != 0) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, texture->raster);
        } else {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
        }
    } else {
        mode4 = 4;
        if (inv_GLI_blendModeState != mode4) {
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
            inv_GLI_blendModeState = (u8) mode4;
        }
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
    }

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*) rwFILTERLINEAR);
}

void GLI_RW_vFogOff(void)
{
    if (inv_GLI_blendModeState != 0) {
        inv_GLI_blendModeState = 0;
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        inv_GLI_zTestState = 1;
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
        inv_GLI_blendModeState = 0;
    }
    if (inv_GLI_zWriteState != 0) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
        inv_GLI_zWriteState = 0;
    }
    if (inv_GLI_blendModeState != 3) {
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
        inv_GLI_blendModeState = 3;
    }
}

void GLI_RW_vDoTextureSelection(void)
{
    GLI_tdstRenderContext* ctx;
    s32 mode;
    s32 hasAlpha;
    GliTexture* tex;
    s32 addr8;
    s32 addr;
    RwTexture* texture;

    s32 stateArg;
    s32 wrap4;

    ctx = inv_GLI_renderContext;
    hasAlpha = 0;
    if ((ctx->lCurrentDrawMask & 0x40000000) != 0) {
        if (inv_GLI_zTestState == 0) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
        }
    } else if (inv_GLI_zTestState != 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
        inv_GLI_zTestState = 0;
    }
    tex = ctx->currentTexture;
    if (tex != 0 && tex->loaded != 0) {
        if ((tex->flags & 0x20) != 0) {
            mode = 2;
            if (inv_GLI_blendModeState != mode) {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
                inv_GLI_blendModeState = (u8) mode;
                tex = ctx->currentTexture;
            }
            hasAlpha = 1;
        } else {
            mode = 3;
            if (inv_GLI_blendModeState != mode) {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                inv_GLI_blendModeState = (u8) mode;
                tex = ctx->currentTexture;
            }
        }
        addr8 = tex->addressMode;
        addr = addr8 & 0xFF;
        if (addr != inv_GLI_textureAddressState) {
            inv_GLI_textureAddressState = (u8) addr8;
            switch (addr) {
            case 0:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSCLAMP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSCLAMP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void*) rwTEXTUREADDRESSCLAMP);
                tex = ctx->currentTexture;
                goto after_addr;
            case 1:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSCLAMP);
                stateArg = rwRENDERSTATETEXTUREADDRESSV;
                wrap4 = rwTEXTUREADDRESSWRAP;
                goto set4;
            case 2:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, (void*) rwTEXTUREADDRESSCLAMP);
                tex = ctx->currentTexture;
                goto after_addr;
            case 3:
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
                RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, (void*) rwTEXTUREADDRESSWRAP);
                stateArg = rwRENDERSTATETEXTUREADDRESSV;
                wrap4 = rwTEXTUREADDRESSWRAP;
                goto set4;
            default:
                tex = ctx->currentTexture;
                goto after_addr;
            set4:
                RwRenderStateSet(stateArg, (void*) wrap4);
                tex = ctx->currentTexture;
                goto after_addr;
            }
        }
    after_addr:
        texture = tex->rwTexture;
        if (texture != 0) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, texture->raster);
        } else {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
        }
        ctx = (GLI_tdstRenderContext*) 1;
    } else {
        s32 mode1 = 1;
        if (inv_GLI_blendModeState != mode1) {
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
            inv_GLI_blendModeState = (u8) mode1;
        }
        D_00393413 = 0;
        ctx = (GLI_tdstRenderContext*) 1;
    }
    if (hasAlpha == (s32) ctx) {
        inv_GLI_renderContext->fogEnabled = 0;
    }
    if (inv_GLI_renderContext->fogEnabled != 0) {
        if (inv_GLI_renderContext->appliedFogParameters != inv_GLI_renderContext->fogParameters) {
            inv_GLI_RW_vApplyFogParameters(inv_GLI_renderContext->fogParameters);
            inv_GLI_renderContext->appliedFogParameters = inv_GLI_renderContext->fogParameters;
        }
        if (inv_GLI_fogState == 0) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 1);
            inv_GLI_fogState = (s16) (s32) ctx;
        }
    } else if (inv_GLI_fogState != 0) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
        inv_GLI_fogState = 0;
    }
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*) rwFILTERLINEAR);
}

s32 inv_GLI_RW_bLockCameraRaster(s32* outPtr, s32* outStride)
{
    RwRaster* raster;
    s32* pointerOut;
    s32* strideOut;
    void* bufferOrResult;

    pointerOut = outPtr;
    strideOut = outStride;
    raster = RwCameraGetRaster(inv_GLI_currentCamera);
    if (raster != NULL) {
        bufferOrResult = RwRasterLock(raster, 0, 1);
        if (bufferOrResult != NULL) {
            goto L_ok;
        }
    }
    bufferOrResult = NULL;
    return (s32) bufferOrResult;
L_ok:
    *pointerOut = (s32) bufferOrResult;
    *strideOut = RwRasterGetStride(raster);
    bufferOrResult = (void*) 1;
L_done:
    return (s32) bufferOrResult;
}

s32 inv_GLI_RW_bUnlockCameraRaster(void)
{
    void* p_stRaster;
    RwRaster* p_stUnlocked;

    p_stRaster = RwCameraGetRaster(inv_GLI_currentCamera);
    p_stUnlocked = RwRasterUnlock(p_stRaster);
    p_stRaster = (void*) ((s32) p_stRaster ^ (s32) p_stUnlocked);
    return (s32) p_stRaster < 1u;
}

void inv_GLI_RW_vDisableRasterCompression(GliDisplayStateView* options)
{
    s32 mask;
    f32 value;

    memset(options, 0, 0x2C);
    D_0039342C = 1;
    mask = 0xFFFF0000;
    mask |= 0xFFFF;
    options->flags = 0x11;
    options->unk08 = 7;
    options->unk14 = 0x100;
    options->unk18 = mask;
    printf("INFO : don't use compression\n");
    value = 60.0f;
    options->unk24 = 3;
    options->frameRate = value;
    options->unk20 = 0;
}

void GLI_RW_xInitRasterize(s32 a0, s32 a1, s32 a2, s32 a3, s32 depth)
{
    inv_GLI_RW_bCreateRasterizerResources(D_00393424, a1, a2, a3);
    inv_RW3_fn_vInitScreenQuadVertices();
    inv_RW3_fn_vInitQuadBatches();
    inv_RW3_fn_vInitAtomicRequests();
    inv_GLI_RW_vDisableRasterCompression(&inv_GLI_stDisplayState);
    return inv_RW3_fn_vInitMeshIndices();
}

void loc_11C5D0(void)
{
    return inv_GLI_RW_bDestroyRasterizerResources(1);
}

void inv_GLI_RW_vShowCameraRaster(void)
{
    GLI_tdstRenderContext* context;
    s32 lValue;
    RwCamera* camera;

    lValue = GLI_xIsGliInit();
    if (lValue != 0) {
        lValue = D_00393428;
        if (lValue != 0) {
            context = inv_GLI_renderContext;
            camera = inv_GLI_currentCamera;
            context->unk20B0 = 0;
            context->inv_bUseSinEffectBase = 0;
            RwCameraShowRaster(camera, 0, 0);
        }
    }
    return;
}

void inv_GLI_RW_vClearCamera(s32 bClearZ, s32 a1, s32 a2)
{
    s32 bClear;
    s32 lClearMode;
    s32 bInit;
    u8 ucColor[0x10];

    bClear = bClearZ;
    bInit = GLI_xIsGliInit();
    if (bInit != 0) {
        lClearMode = 2;
        lClearMode = bClear ? lClearMode : 0;
        RwCameraClear(inv_GLI_currentCamera, ucColor, lClearMode);
    }
    return;
}

void inv_GLI_RW_vClearCameraIfEnabled(s32 left, s32 right, s32 top, s32 bottom)
{
    if (inv_GLI_renderContext->inv_bNoCameraClear == 0)
        inv_GLI_RW_vClearCamera(1, 0, 0);
}

s32 inv_GLI_RW_bDestroyRasterizerResources(s32 unused)
{
    GLI_RW_xDesinitAPI();
    return 1;
}

s32 inv_GLI_RW_bCreateRasterizerResources(s32 a0, s32 a1, s32 lWidth, s32 lHeight)
{
    s32 lCameraWidth;
    s32 lCameraHeight;
    s32 lResult;
    s32 lValue;
    u8 ucClearColor[0x10];

    lCameraHeight = lHeight;
    lCameraWidth = lWidth;
    lResult = RwResourcesSetArenaSize(0x200000);
    lValue = 1;
    if (lResult != lValue) {
        return 0;
    }
    lResult = GLI_RW_pCameraCreate(lCameraWidth, lCameraHeight, 0);
    if (lResult == 0) {
        GLI_RW_xDesinitAPI();
        return 0;
    }
    ((void* (*) (void* s, s32 c, s32 n)) memset)(ucClearColor, 0, 4);
    lValue = 0xFF;
    ucClearColor[3] = (u8) lValue;
    RwCameraClear(inv_GLI_currentCamera, (RwRGBA*) ucClearColor, 1);
    RwCameraShowRaster((*&inv_GLI_currentCamera), 0, 0);
    RwCameraClear(inv_GLI_currentCamera, (RwRGBA*) ucClearColor, 1);
    RwCameraShowRaster((*&inv_GLI_currentCamera), 0, 0);
    inv_RW3_fn_vCreateWorld(0);
    inv_GLI_RW_vResizeCameraViewport((*&inv_GLI_currentCamera), 0, 0.7f, 1.3333334f);
    func_00153560();
    return 1;
}

s32 inv_GLI_RW_bRegisterImageFormats(void)
{
    s32 bRegistered;

    bRegistered = RwImageRegisterImageFormat("bmp", func_00192390, NULL);
    if (bRegistered == 0) {
        return 0;
    }
    RwImageSetPath("ps2_data/vignette/");
    return 1;
}

s32 GLI_RW_pCameraCreate(s32 a0, s32 a1, f32* viewWindow)
{
    RwCamera* camera;
    RwFrame* frame;
    RwRaster* raster;
    u32 height;
    u32 width;

    struct {
        s32 rect[4];
        f32 vw[2];
    } local;

    width = g_lVIDEO_WantedWidth;
    height = g_lVIDEO_WantedHeight;
    local.rect[0] = 0;
    local.rect[1] = 0;
    local.rect[2] = width;
    local.rect[3] = height;

    camera = RwCameraCreate();
    if (camera != 0) {
        inv_GLI_primaryCamera = camera;
        (*&inv_GLI_currentCamera) = camera;

        frame = RwFrameCreate();
        if (frame != 0) {
            RwCameraSetRaster(camera, RwRasterCreate(width, height, g_lVIDEO_WantedDepth, 2));
            raster = RwCameraGetRaster(camera);
            if (raster != 0) {
                RwCameraSetZRaster(camera, RwRasterCreate(width, height, 0, 1));
                if (RwCameraGetZRaster(camera) != 0) {
                    RwCameraSetFrame(camera, frame);
                    RwCameraSetFarClipPlane(camera, 500.0f);
                    RwCameraSetNearClipPlane(camera, 0.25f);

                    if (viewWindow != 0) {
                        RwCameraSetViewWindow(camera, viewWindow);
                    } else {
                        local.vw[0] =
                            ((f32) g_lVIDEO_WantedWidth / (f32) g_lVIDEO_WantedHeight) * 0.75f;
                        local.vw[1] = 0.75f;
                        RwCameraSetViewWindow(camera, local.vw);
                    }
                    inv_GLI_RW_vCreateSecondaryCamera();
                    return 1;
                }
                RwRasterDestroy(raster);
            }
            RwFrameDestroy(frame);
        }
        RwCameraDestroy(camera);
    }
    return 0;
}

s32 GLI_RW_bInitAPI(void* pDisplay, void* pEngineInitParam, s32 (*pfnCallback)(void))
{
    u32 stOpenParams[4];

    RwEngineInit(pEngineInitParam, 0);
    inv_RW_fn_bFileSystemPluginAttach();
    printf("INFO ---> Using POWERPIPE RW3 libs.\n");
    if (RpWorldPluginAttach() == 0) {
        return 0;
    }
    inv_RW3_fn_vRegisterLightmapPlugin();
    if (pfnCallback == 0) {
        stOpenParams[0] = (u32) pDisplay;
    } else {
        if (pfnCallback() == 0) {
            RwEngineTerm();
            return 0;
        }
        stOpenParams[0] = (u32) pDisplay;
    }
    if (RwEngineOpen(stOpenParams) != 0) {
        if (GLI_RW_bVideoModeSelect() != 0) {
            {
                s32 lResult = func_001ABF78(0xC0000, 0x14, 1);
                D_00393B40 = 0;
                D_00393B24 = 0x100000;
                (void) lResult;
            }
            if (RwEngineStart() != 0) {
                if (inv_GLI_RW_bRegisterImageFormats() != 0) {
                    return 1;
                }
                RwEngineStop();
            }
        }
        RwEngineClose();
    }
term:
    RwEngineTerm();
    return 0;
}

void GLI_RW_xDesinitAPI(void)
{
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();
}

s32 GLI_RW_bVideoModeSelect(void)
{
    RwVideoMode modeInfo;
    RwVideoMode selectedModeInfo;
    s32 selectedMode;
    s32 modeCount;
    s32 modeIndex;

    selectedMode = -1;
    modeIndex = 0;
    memset(&selectedModeInfo, 0, sizeof(selectedModeInfo));
    selectedModeInfo.flags = 0x7FFFFFFF;
    modeCount = RwEngineGetNumVideoModes();
    printf("INFO ---> Looking for VideoMode : %dx%dx%d-0x%X\n", g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, g_lVIDEO_WantedDepth, 3);

    if (modeCount > 0) {
        for (; modeIndex < modeCount; modeIndex++) {
            RwEngineGetVideoModeInfo(&modeInfo, modeIndex);
            if (modeInfo.width != g_lVIDEO_WantedWidth ||
                modeInfo.height != g_lVIDEO_WantedHeight ||
                modeInfo.depth != g_lVIDEO_WantedDepth ||
                modeInfo.flags != (rwVIDEOMODEEXCLUSIVE | rwVIDEOMODEINTERLACE)) {
                continue;
            }
            selectedModeInfo = modeInfo;
            selectedMode = modeIndex;
        }
    }

    if (selectedMode < 0) {
        printf("WARNING ---> Can't find VideoMode : %dx%dx%d-0x%X\n", g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, g_lVIDEO_WantedDepth,
               3);
        selectedMode = 0;
        RwEngineGetVideoModeInfo(&selectedModeInfo, 0);
        g_lVIDEO_WantedWidth = selectedModeInfo.width;
        g_lVIDEO_WantedHeight = selectedModeInfo.height;
        g_lVIDEO_WantedDepth = selectedModeInfo.depth;
    }

    printf("INFO ---> Using VideoMode_%d : %dx%dx%d-0x%X\n", selectedMode, g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight,
           g_lVIDEO_WantedDepth, selectedModeInfo.flags);
    if (0) printf("INFO ---> AntiAliasing : FSAA0 method used.\n");
    if (0) printf("INFO ---> AntiAliasing : FSAA1 method used.\n");
    printf("INFO ---> AntiAliasing : No AA.\n");
    printf("INFO ---> Using some RETAINED MODE.\n");
    return RwEngineSetVideoMode(selectedMode);
}

void inv_GLI_RW_vResizeCameraViewport(RwCamera* camera, s32* rect, f32 x, f32 y)
{
    f32 viewWindow[4];
    s32 modeInfo[8];
    s32* sub;
    RwRaster* raster;
    s32 w;
    s32 h;

    if (camera == 0) {
        return;
    }
    RwEngineGetVideoModeInfo(modeInfo, RwEngineGetCurrentVideoMode());
    if (rect == 0) {
        if (modeInfo[3] & rwVIDEOMODEEXCLUSIVE) {
            modeInfo[6] = modeInfo[0];
            modeInfo[7] = modeInfo[1];
        } else {
            modeInfo[6] = RwRasterGetWidth(RwCameraGetRaster(camera));
            modeInfo[7] = RwRasterGetHeight(RwCameraGetRaster(camera));
        }
        modeInfo[5] = 0;
        modeInfo[4] = 0;
        sub = &modeInfo[4];
    } else {
        sub = rect;
    }
    if (modeInfo[3] & rwVIDEOMODEEXCLUSIVE) {
        viewWindow[0] = x;
        viewWindow[1] = x / y;
    } else {
        w = sub[2];
        h = sub[3];
        if (h < w) {
            viewWindow[0] = x;
            viewWindow[1] = (f32) sub[3] * x / (f32) sub[2];
        } else {
            viewWindow[1] = x;
            viewWindow[0] = (f32) w * x / (f32) h;
        }
    }
    raster = RwCameraGetRaster(camera);
    if (raster != 0) {
        RwRasterSubRaster(raster, RwRasterGetParent(raster), sub);
    }
    raster = RwCameraGetZRaster(camera);
    if (raster != 0) {
        RwRasterSubRaster(raster, RwRasterGetParent(raster), sub);
    }
    RwCameraSetViewWindow(camera, viewWindow);
}

void inv_GLI_RW_vCreateSecondaryCamera(void)
{
    RwCamera* cam;
    f32 vw[2];
    RwRaster* raster;
    RwRaster* zraster;

    cam = inv_GLI_primaryCamera;
    inv_GLI_secondaryCamera = 0;
    if (cam == 0) {
        return;
    }
    vw[0] = 0.7f;
    vw[1] = 0.525f;
    raster = RwCameraGetRaster(cam);
    zraster = RwCameraGetZRaster(inv_GLI_primaryCamera);
    inv_GLI_secondaryCamera = RwCameraCreate();
    RwCameraSetFrame(inv_GLI_secondaryCamera, RwFrameCreate());
    RwCameraSetRaster(inv_GLI_secondaryCamera, raster);
    RwCameraSetZRaster(inv_GLI_secondaryCamera, zraster);
    RwCameraSetFarClipPlane(inv_GLI_secondaryCamera, 500.0f);
    RwCameraSetNearClipPlane(inv_GLI_secondaryCamera, 0.25f);
    RwCameraSetViewWindow(inv_GLI_secondaryCamera, vw);
}

void func_0011CEF0(RwCamera* camera)
{
    inv_GLI_currentCamera = camera;
}

void inv_GLI_RW_vSetCameraFOV(GLI_tdstCamera_* p_stCamera)
{
    RwCamera* p_stRwCamera;
    f32 xViewWindow[2];

    if (p_stCamera == g_stEngineStructure.p_stFixCamera) {
        p_stRwCamera = inv_GLI_secondaryCamera;
    } else {
        p_stRwCamera = inv_GLI_primaryCamera;
    }
    if (p_stRwCamera == 0) {
        return;
    }
    xViewWindow[0] = tanf(p_stCamera->xAlphaX * 0.5f);
    xViewWindow[1] = tanf(p_stCamera->xAlphaY * 0.5f);
    RwCameraSetViewWindow(p_stRwCamera, xViewWindow);
}

void inv_GLI_RW_vSetCameraAspectRatio(GLI_tdstCamera_* p_stCamera, f32 xRatio)
{
    RwCamera* cam;
    f32* src;
    f32 vw[2];

    if (p_stCamera == g_stEngineStructure.p_stFixCamera) {
        cam = inv_GLI_secondaryCamera;
    } else {
        cam = inv_GLI_primaryCamera;
    }
    if (cam == 0) {
        return;
    }
    src = RwCameraGetViewWindow(cam);
    vw[0] = src[0];
    vw[1] = src[0] / xRatio;
    RwCameraSetViewWindow(cam, vw);
}
