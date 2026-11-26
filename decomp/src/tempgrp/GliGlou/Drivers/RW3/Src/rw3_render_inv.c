#include "common.h"

#include "GLI/GLI.h"

#include "renderware.h"

#include "include_asm.h"
#include "SCT/SCT.h"

#define GEOMETRIC_ELEMENT_MESH 1

typedef struct LightColor {
    f32 components[4];
} LightColor;
extern LightColor qword_38A3A0;
extern LightColor qword_38A3B0;

void inv_RW3_fn_vAddAtomicRequest(f32 alpha, RpAtomic* atomic, s32 comparisonFlags, s32 mode, char* name,
                   s32 drawFlags);

void func_00152558(AtomicDrawRequest* request)
{
    RpAtomic* atomic;
    RwFrame* savedFrame;
    GLI_tdstRenderContext* context;
    tdstFogParams_* fogParameters;
    GLI_tdstRenderContext* fogContext;
    void* pipeline;
    LightColor color;
    LightColor fadeColor;

    atomic = request->atomic;
    savedFrame = NULL;
    if (!(request->flags & DRAW_REQUEST_KEEP_FRAME)) {
        savedFrame = atomic->object.object.parent;
        RpAtomicSetFrame(atomic, request->frame);
    }
    inv_GLI_vPopLightsInViewport();
    if ((request->flags & DRAW_REQUEST_KEEP_FRAME) || request->mode == 2 ||
        (request->flags & DRAW_REQUEST_UNLIT)) {
        color = qword_38A3A0;
        inv_RW3_fn_vSetSingleLightColor(&color);
    } else {
        RW3_Light_AddListOfLightsToWorld(&request->lights);
    }

    context = inv_GLI_renderContext;
    context->lCurrentDrawMask = request->comparisonFlags;
    if (request->mode == 2) {
        context->fogEnabled = 0;
    } else {
        context->fogEnabled = request->fogParameters != NULL;
    }
    fogContext = inv_GLI_renderContext;
    fogParameters = request->fogParameters;
    fogContext->fogParameters = fogParameters;
    if (fogContext->fogEnabled != 0) {
        if (fogContext->appliedFogParameters != fogParameters) {
            inv_GLI_RW_vApplyFogParameters(fogParameters);
            inv_GLI_renderContext->appliedFogParameters = inv_GLI_renderContext->fogParameters;
        }
        if (inv_GLI_fogState == 0) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 1);
            inv_GLI_fogState = 1;
        }
    } else if (inv_GLI_fogState != 0) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
        inv_GLI_fogState = 0;
    }

    if (inv_GLI_renderContext->lCurrentDrawMask & 0x40000000) {
        if (inv_GLI_zTestState == 0) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
        }
    } else if (inv_GLI_zTestState != 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
        inv_GLI_zTestState = 0;
    }

    inv_RW3_fn_vSetAtomicAlpha(request->alpha, atomic);
    pipeline = inv_RW3_fn_pGetAtomicPipeline(atomic);
    if (inv_RW3_fn_bIsLightmapPipeline(pipeline) != 1) {
        pipeline = NULL;
    } else if (request->alpha != 1.0f) {
        fadeColor = qword_38A3B0;
        inv_RW3_fn_vSetSingleLightColor(&fadeColor);
        inv_RW3_fn_vSetAtomicPipeline(atomic, NULL);
    } else {
        pipeline = NULL;
    }

    if (inv_GLI_blendModeState != request->mode) {
        switch (request->mode) {
        case 1:
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
            break;
        case 2:
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
            break;
        case 4:
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
            break;
        case 3:
        case 5:
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
            break;
        case 6:
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDDESTCOLOR);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
            break;
        case 7:
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDDESTCOLOR);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDSRCCOLOR);
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            break;
        case 8:
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
            break;
        default:
            inv_GLI_blendModeState = 0;
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = 1;
            break;
        }
        inv_GLI_blendModeState = request->mode;
    }

    atomic->renderCallBack(atomic);
    if (pipeline != NULL) {
        inv_RW3_fn_vSetAtomicPipeline(atomic, pipeline);
    }
    if (!(request->flags & DRAW_REQUEST_KEEP_FRAME)) {
        RpAtomicSetFrame(atomic, savedFrame);
    }
}

void func_00152978(struct GLD_tdstViewportAttributes_* viewport)
{
    DrawQueueEntry* entry;
    GLI_tdstRenderContext* context;
    s32 fogEnabled;
    tdstFogParams_* fogParameters;
    s32 remaining;

    entry = inv_GLI_drawQueue;
    if (inv_GLI_texturedRequestCount + inv_GLI_atomicRequestCount == 0) {
        return;
    }
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
    if (inv_GLI_zTestState != 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
        inv_GLI_zTestState = 0;
    }

    for (remaining = inv_GLI_texturedRequestCount + inv_GLI_atomicRequestCount; remaining != 0;
         entry++, remaining--) {
        if (entry->kind == DRAW_KIND_ATOMIC) {
            func_00152558(entry->object.atomic);
            continue;
        }
        context = inv_GLI_renderContext;
        fogEnabled = entry->object.textured->fogParameters != NULL;
        context->fogEnabled = fogEnabled;
        fogParameters = entry->object.textured->fogParameters;
        context->fogParameters = fogParameters;
        if (fogEnabled != 0) {
            if (context->appliedFogParameters != fogParameters) {
                inv_GLI_RW_vApplyFogParameters(fogParameters);
                inv_GLI_renderContext->appliedFogParameters = inv_GLI_renderContext->fogParameters;
            }
            if (inv_GLI_fogState == 0) {
                RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 1);
                inv_GLI_fogState = 1;
            }
        } else if (inv_GLI_fogState != 0) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
            inv_GLI_fogState = 0;
        }
        inv_RW3_fn_vDrawTexturedQuads(viewport, &entry->object.textured->quad, entry->object.textured->texture);
    }
    inv_GLI_vPopLightsInViewport();
    inv_GLI_texturedRequestCount = 0;
    inv_GLI_atomicRequestCount = 0;
}

static inline void inv_GLI_fn_vResetSortPass(void)
{
    GLI_tdstRenderContext* context = inv_GLI_renderContext;

    context->unk20AC = 0;
    if (context->unk20B0 != 0) {
        context->unk20B0 = 3;
    }
}

static inline GeometricObjectView* inv_GLI_fn_p_stCurrentObject(GLI_tdstRenderContext* context)
{
    return *(void* volatile*) &context->geometricObject;
}

void func_00152B10(void)
{
    GLI_tdstRenderContext* context;
    GLI_tdstRenderContext* sortContext;
    GLI_tdstRenderContext* drawContext;
    GeometricObjectView* object;
    GeometricObjectView* loopObject;
    RW3ObjectView* rw3Object;
    GliMaterialData* material;
    GliTexture* texture;
    RpClump* clump;
    RpAtomic** atomics;
    RpAtomic* atomic;
    u8* modes;
    u8* mode;
    u8 type;
    u8 drawFlags;
    s32 textured;
    s32 atomicIndex;
    s32 resetWord;
    u32 index;
    f32 alpha;
    LightColor color;

    {
        GLI_tdstRenderContext* first;
        GeometricObjectView* firstGeometry;
        RW3ObjectView* firstObject;

        first = inv_GLI_renderContext;
        atomicIndex = 0;
        firstGeometry = first->geometricObject;
        firstObject = firstGeometry->rw3Object;
        modes = firstObject->atomicStates;
        rw3Object = firstObject;
        if (firstGeometry->flags & 0xB) {
            RW3_UpdateFrameWithGeometricObjectMatrix(firstObject->frame, first->currentMatrix);
        }
    }
    {
        GLI_tdstRenderContext* first;
        GeometricObjectView* firstGeometry;

        first = inv_GLI_renderContext;
        index = 0;
        firstGeometry = first->geometricObject;
        inv_RW3_fn_vScrollObjectTexCoords(firstGeometry->rw3Object, first);
    }
    {
        GLI_tdstRenderContext* first;
        GeometricObjectView* firstGeometry;

        first = inv_GLI_renderContext;
        context = *&first;
        firstGeometry = inv_GLI_fn_p_stCurrentObject(context);
        atomics = firstGeometry->rw3Object->atomics;
        loopObject = firstGeometry;
    }

    for (; index < loopObject->elementCount; index++) {
        object = context->geometricObject;
        type = object->elementTypes[index];
        if (type != GEOMETRIC_ELEMENT_MESH) {
            goto next;
        }
        material = object->elements[index]->material;
        context->currentMaterial = material;
        texture = material->layers[0].texture;
        context->currentTexture = texture;
        textured = texture != NULL && texture->loaded != 0;

        if (textured && !(object->flags & 8)) {
            if (texture->flags & 0x100) {
                if (inv_SCT_ucDrawPass == 0) {
                    atomicIndex++;
                    goto next;
                }
            } else if (inv_SCT_ucDrawPass == type) {
                goto drawn;
            }
        }

        inv_GLI_fn_vResetSortPass();

        {
            GLI_tdstRenderContext* forceContext;
            GliTexture* sortTexture;

            sortContext = inv_GLI_renderContext;
            sortContext->lCurrentDrawMask =
                sortContext->currentMaterial->type & sortContext->lHierachDrawMask;
            if (sortContext->unk20B0 == 0) {
                if (textured) {
                    sortTexture = sortContext->currentTexture;
                    forceContext = sortContext;
                    if (sortTexture->flags & 0x28) {
                        goto forced;
                    }
                }
            }
            drawContext = inv_GLI_renderContext;
            mode = modes + atomicIndex;
            if (drawContext->lCurrentDrawMask & 8) {
                goto unsorted;
            }
            if (drawContext->globalAlpha < 0.98f) {
                resetWord = drawContext->inv_lDrawFlags;
                goto sorted;
            }
            goto unsorted;
        forced:
            drawContext = forceContext;
            resetWord = forceContext->inv_lDrawFlags;
        sorted:
            drawContext->inv_lDrawFlags = resetWord | GLI_DRAW_DEFERRED_inv;
            if (textured) {
                if (drawContext->currentTexture->flags & 8) {
                    mode = modes;
                    mode += atomicIndex;
                    goto sortedCall;
                }
                drawContext->unk20AC = 1;
            }
            mode = modes;
            mode += atomicIndex;
        sortedCall:
            func_00153970(mode);
            clump = inv_RW3_sortedClump;
            goto selected;
        unsorted:
            func_001539C0(mode, drawContext);
            clump = inv_RW3_unsortedClump;
        }
    selected:
        if (clump == inv_RW3_unsortedClump) {
            inv_RW3_fn_vSetAtomicAlpha(1.0f, atomics[atomicIndex]);
            if (inv_GLI_blendModeState != rw3Object->atomicStates[atomicIndex]) {
                switch (rw3Object->atomicStates[atomicIndex]) {
                case 1:
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
                    break;
                case 2:
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
                    break;
                case 4:
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
                    break;
                case 3:
                case 5:
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                    break;
                case 6:
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDDESTCOLOR);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                    break;
                case 7:
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDDESTCOLOR);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDSRCCOLOR);
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    break;
                case 8:
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDZERO);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    break;
                default:
                    inv_GLI_blendModeState = 0;
                    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
                    inv_GLI_zTestState = 1;
                    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
                    inv_GLI_zWriteState = 1;
                    break;
                }
                inv_GLI_blendModeState = rw3Object->atomicStates[atomicIndex];
            }

            if (inv_GLI_currentCamera != inv_GLI_secondaryCamera) {
                GLI_tdstRenderContext* current;
                GeometricObjectView* currentGeometry;

                current = inv_GLI_renderContext;
                currentGeometry = current->geometricObject;
                if (currentGeometry->flags & 8) {
                    if (!textured) {
                        func_0014EA40();
                        goto render;
                    }
                    if (!(current->currentTexture->flags & 0x400)) {
                        func_0014EA40();
                        goto render;
                    }
                }
                color = qword_38A3A0;
                inv_RW3_fn_vSetSingleLightColor(&color);
            }
        render:
            atomic = atomics[atomicIndex];
            atomic->renderCallBack(atomic);
            if (inv_GLI_currentCamera != inv_GLI_secondaryCamera) {
                inv_GLI_vPopLightsInViewport();
            }
        drawn:
            atomicIndex++;
        } else {
            if (inv_GLI_currentCamera != inv_GLI_secondaryCamera) {
                GLI_tdstRenderContext* current;
                u32 offset;
                RpAtomic** entry;
                s32 renderFlags;
                u8 drawMode;

                drawFlags = rw3Object->mode;
                if (textured) {
                    current = inv_GLI_renderContext;
                    if (current->currentTexture->flags & 0x400) {
                        drawFlags |= 0x10;
                    }
                }
                current = inv_GLI_renderContext;
                offset = (u32) atomicIndex << 2;
                entry = (RpAtomic**) (offset + (u32) atomics);
                renderFlags = current->lHierachDrawMask & 8;
                alpha = 1.0f;
                drawMode = *mode;
                if (renderFlags == 0) {
                    alpha = current->globalAlpha;
                }
                inv_RW3_fn_vAddAtomicRequest(alpha, *entry, current->lCurrentDrawMask, drawMode, "UNAVAILABLE",
                              drawFlags);
            } else {
                GLI_tdstRenderContext* current;
                RpAtomic* first;

                textured = atomicIndex * 4;
                current = inv_GLI_renderContext;
                first = *(RpAtomic* volatile*) (textured + (u32) atomics);
                alpha = current->globalAlpha;
                inv_RW3_fn_vSetAtomicAlpha(alpha, first);
                if (inv_GLI_blendModeState != *mode) {
                    switch (*mode) {
                    case 1:
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
                        break;
                    case 2:
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
                        break;
                    case 4:
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
                        break;
                    case 3:
                    case 5:
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                        break;
                    case 6:
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDDESTCOLOR);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                        break;
                    case 7:
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDDESTCOLOR);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDSRCCOLOR);
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                        break;
                    case 8:
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDZERO);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
                        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
                        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
                        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                        break;
                    default:
                        inv_GLI_blendModeState = 0;
                        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
                        inv_GLI_zTestState = 1;
                        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
                        inv_GLI_zWriteState = 1;
                        break;
                    }
                    inv_GLI_blendModeState = *mode;
                }
                if (inv_GLI_zWriteState != 0) {
                    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
                    inv_GLI_zWriteState = 0;
                }
                atomic = atomics[atomicIndex];
                atomic->renderCallBack(atomic);
            }
            atomicIndex++;
        }
    next:
        context = inv_GLI_renderContext;
        loopObject = inv_GLI_fn_p_stCurrentObject(context);
    }
}
