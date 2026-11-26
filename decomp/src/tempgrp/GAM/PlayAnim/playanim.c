#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "AI/AI.h"

extern s32 PLA_g_ulNbAllocInSupObjHeap;

void PLA_fn_vReleaseSuperObjectInHeap(HIE_tdstSuperObject_*);
void PLA_fn_vInitMatrixHeap(void);
void PLA_fn_vInitSupObjHeap(void);
void PLA_fn_vInitAllChildInHeapNewAnim(HIE_tdstSuperObject_* ctx, tdstAnim3d_* input,
                      u16 index);
void PLA_fn_vCreateMatrixHeap(void);

extern POS_tdstMatrix_* PLA_g_d_stMatrixHeap;

extern u32 inv_GAM_ulTicksPerSecond;
extern u16 PLA_g_a_wStateOfSupObjHeap[];
extern s32 PLA_g_ulCurrentPosInSupObjHeap;

static inline void inv_GAM_fn_vPLASetValue(HIE_tdstSuperObject_* c, s32 v)
{
    c->hLinkedObject = (void*) v;
    c->lTypeOfLinkedObject = v ? 8 : 1;
}

static inline void inv_GAM_fn_vPLASetFromEvent(HIE_tdstSuperObject_* c, HIE_tdstSuperObject_* so, tdstAnim3d_* anim, u16 frame,
                                    tdstElement3d_* cpl, tdstObjectsTableElement_* elts, s32 idx, tdstObjectsTableElement_* elt)
{
    AnimationMorphEntry* p_stMorph;
    void* pMorphResult;
    s32 v;
    s32 st;

    if (anim->d_stMorphDataArray == NULL) {
        if (elt->wTypeOfTarget == 0) {
            inv_GAM_fn_vPLASetValue(c, elt->h_Target);
        }
    } else if (elt->wTypeOfTarget == 0) {
        p_stMorph = inv_PLA_fn_p_stFindMorphData(anim, cpl->ucChannelNumber, frame);
        if (p_stMorph == NULL) {
            MOR_fn_xStopMorph(so, cpl->ucChannelNumber);
            c->lTypeOfLinkedObject = elt->h_Target ? 8 : 1;
            c->hLinkedObject = (void*) elt->h_Target;
        } else if (p_stMorph->morphingAmount < 100) {
            pMorphResult = inv_MOR_fn_p_stStartMorph(so, idx, p_stMorph->target, p_stMorph->morphingAmount, cpl->ucChannelNumber);
            if (pMorphResult != NULL) {
                inv_MOR_fn_vUpdateMorph(pMorphResult);
                v = (s32) inv_MOR_fn_p_vGetMorphTarget(pMorphResult);
                c->hLinkedObject = (void*) v;
                c->lTypeOfLinkedObject = v ? 8 : 1;
            } else {
                c->lTypeOfLinkedObject = elts[idx].h_Target ? 8 : 1;
                c->hLinkedObject = (void*) elts[idx].h_Target;
            }
        } else {
            MOR_fn_xStopMorph(so, cpl->ucChannelNumber);
            v = elts[p_stMorph->target].h_Target;
            c->hLinkedObject = (void*) v;
            c->lTypeOfLinkedObject = v ? 8 : 1;
        }
    }
}

static inline void inv_GAM_fn_vPLAGetVector(MTH3D_tdstVector_* v, f32* x, f32* y, f32* z)
{
    *x = v->xX;
    *y = v->xY;
    *z = v->xZ;
}

void PLA_fn_vDoFrame(tdstAnim3d_* anim, u16 frame, HIE_tdstSuperObject_* so)
{
    tdstFrame3d_* frame3d;
    tdstElement3d_* end;
    gu_tdst3dData_* data;
    tdstObjectsTableElement_* elts;
    f32 rate;
    tdstElement3d_* cpl;
    tdstAChannel_* slot;
    tdstAChannel_* chans;
    HIE_tdstSuperObject_* main;
    HIE_tdstSuperObject_* ch;
    tdstObjectsTableElement_* elt;
    s32 idx;
    MTH3D_tdstVector_* v;
    f32 x, y, z;
    HIE_tdstSuperObject_* p;
    HIE_tdstSuperObject_* sub;

    rate = so->gu_fTransparenceLevel;
    data = ((tdstEngineObject_*) so->hLinkedObject)->h_3dData;
    frame3d = &data->stFrame3d;
    elts = &data->h_CurrentObjectsTable->d_stObjectsTable->stElement;
    fn_vUpdateFrame(so, frame3d, anim, frame);

    cpl = data->stFrame3d.p_stArrayOfElts3d;
    end = cpl + anim->ucMaxNumberOfElements;
    if (cpl != end) {
        chans = data->hArrayOfChannels;
        for (;;) {
            if (cpl->ucChannelNumber == 0xFF) {
                cpl++;
            } else {
                slot = &chans[cpl->ucChannelNumber];
                main = slot->hSupObject;
                if (cpl->eTypeOfElement == 0 && (cpl->wElement & 0x8000)) {
                    idx = cpl->wElement & 0x7FFF;
                    elt = &elts[idx];
                    if (elt->wTypeOfTarget != 0xFFFF) {
                        if (elt->p_stCustomZoom != NULL) {
                            sub = slot->hZoomSupObject;
                            if (sub == NULL) {
                                main->hLinkedObject = 0;
                                main->lTypeOfLinkedObject = HIE_TYPE_EMPTY_inv;
                                ch = PLA_fn_hFindNextFreeSupObj();
                                if (ch->hNextBrotherSta != NULL)
                                    ch->hNextBrotherSta->hPrevBrotherSta = ch->hPrevBrotherSta;
                                if (ch->hPrevBrotherSta != NULL)
                                    ch->hPrevBrotherSta->hNextBrotherSta = ch->hNextBrotherSta;
                                if (ch->hFatherSta != NULL) {
                                    ch->hFatherSta->lNumberOfElementsSta--;
                                    if (ch->hFatherSta->hFirstElementSta == ch)
                                        ch->hFatherSta->hFirstElementSta = ch->hNextBrotherSta;
                                    if (ch->hFatherSta->hLastElementSta == ch)
                                        ch->hFatherSta->hLastElementSta = ch->hPrevBrotherSta;
                                }
                                ch->hNextBrotherSta = NULL;
                                ch->hPrevBrotherSta = NULL;
                                ch->hFatherSta = NULL;
                                ch->hPrevBrotherSta = main->hLastElementSta;
                                ch->hFatherSta = main;
                                if (main->hFirstElementSta == NULL)
                                    main->hFirstElementSta = ch;
                                else
                                    main->hLastElementSta->hNextBrotherSta = ch;
                                main->hLastElementSta = ch;
                                main->lNumberOfElementsSta++;
                                slot->hZoomSupObject = ch;
                            } else
                                ch = sub;
                            inv_GAM_fn_vPLASetFromEvent(ch, so, anim, frame, cpl, elts, idx, elt);
                            ch->hVisualBoundingVolume = 0;
                            ch->hCollideBoundingVolume = 0;
                            gu_MTH4D_M_vSetIdentityMatrix(ch->gu_hLocalMatrix);
                            POS_fn_vSetScaleMatrix(ch->gu_hLocalMatrix, elt->p_stCustomZoom);
                            v = elt->p_stCustomZoom;
                            inv_GAM_fn_vPLAGetVector(v, &x, &y, &z);
                            if (x * (y * z) < 0.0f)
                                ch->gu_lDrawModeMask &= ~0x80000;
                            if (!(ch->flags & 0x10000)) {
                                if (so->gu_lDrawModeMask & 8) {
                                    if (cpl->ucTransparency == 0xFF) {
                                        ch->gu_fTransparenceLevel = rate;
                                        ch->gu_lDrawModeMask |= 8;
                                    } else {
                                        ch->gu_fTransparenceLevel = cpl->ucTransparency;
                                        ch->gu_lDrawModeMask &= ~8;
                                    }
                                } else {
                                    ch->gu_fTransparenceLevel = rate;
                                }
                            }
                        } else {
                            if (slot->hZoomSupObject != NULL) {
                                if (slot->hZoomSupObject->hNextBrotherSta != NULL)
                                    slot->hZoomSupObject->hNextBrotherSta->hPrevBrotherSta = slot->hZoomSupObject->hPrevBrotherSta;
                                if (slot->hZoomSupObject->hPrevBrotherSta != NULL)
                                    slot->hZoomSupObject->hPrevBrotherSta->hNextBrotherSta = slot->hZoomSupObject->hNextBrotherSta;
                                if (slot->hZoomSupObject->hFatherSta != NULL) {
                                    slot->hZoomSupObject->hFatherSta->lNumberOfElementsSta--;
                                    if (slot->hZoomSupObject->hFatherSta->hFirstElementSta == slot->hZoomSupObject)
                                        slot->hZoomSupObject->hFatherSta->hFirstElementSta = slot->hZoomSupObject->hNextBrotherSta;
                                    if (slot->hZoomSupObject->hFatherSta->hLastElementSta == slot->hZoomSupObject)
                                        slot->hZoomSupObject->hFatherSta->hLastElementSta = slot->hZoomSupObject->hPrevBrotherSta;
                                }
                                slot->hZoomSupObject->hNextBrotherSta = NULL;
                                slot->hZoomSupObject->hPrevBrotherSta = NULL;
                                slot->hZoomSupObject->hFatherSta = NULL;
                                PLA_fn_vReleaseSuperObjectInHeap((HIE_tdstSuperObject_*) slot->hZoomSupObject);
                                slot->hZoomSupObject = NULL;
                            }
                            inv_GAM_fn_vPLASetFromEvent(main, so, anim, frame, cpl, elts, idx, elt);
                            main->hVisualBoundingVolume = 0;
                            main->hCollideBoundingVolume = 0;
                        }
                    } else {
                        main->hLinkedObject = 0;
                        main->lTypeOfLinkedObject = HIE_TYPE_EMPTY_inv;
                    }
                }
                if (cpl->eTypeOfElement == 2 || cpl->eTypeOfElement == 3 || cpl->eTypeOfElement == 6 || cpl->eTypeOfElement == 5 ||
                    !(cpl->wElement & 0x8000)) {
                    if (slot->hZoomSupObject != NULL) {
                        if (slot->hZoomSupObject->hNextBrotherSta != NULL)
                            slot->hZoomSupObject->hNextBrotherSta->hPrevBrotherSta = slot->hZoomSupObject->hPrevBrotherSta;
                        if (slot->hZoomSupObject->hPrevBrotherSta != NULL)
                            slot->hZoomSupObject->hPrevBrotherSta->hNextBrotherSta = slot->hZoomSupObject->hNextBrotherSta;
                        if (slot->hZoomSupObject->hFatherSta != NULL) {
                            slot->hZoomSupObject->hFatherSta->lNumberOfElementsSta--;
                            if (slot->hZoomSupObject->hFatherSta->hFirstElementSta == slot->hZoomSupObject)
                                slot->hZoomSupObject->hFatherSta->hFirstElementSta = slot->hZoomSupObject->hNextBrotherSta;
                            if (slot->hZoomSupObject->hFatherSta->hLastElementSta == slot->hZoomSupObject)
                                slot->hZoomSupObject->hFatherSta->hLastElementSta = slot->hZoomSupObject->hPrevBrotherSta;
                        }
                        slot->hZoomSupObject->hNextBrotherSta = NULL;
                        slot->hZoomSupObject->hPrevBrotherSta = NULL;
                        slot->hZoomSupObject->hFatherSta = NULL;
                        PLA_fn_vReleaseSuperObjectInHeap((HIE_tdstSuperObject_*) slot->hZoomSupObject);
                        slot->hZoomSupObject = NULL;
                    }
                    main->hLinkedObject = 0;
                    main->lTypeOfLinkedObject = HIE_TYPE_EMPTY_inv;
                }
                if (!(main->flags & 0x10000)) {
                    if (so->gu_lDrawModeMask & 8) {
                        if (cpl->ucTransparency == 0xFF) {
                            main->gu_fTransparenceLevel = rate;
                            main->gu_lDrawModeMask |= 8;
                        } else {
                            main->gu_fTransparenceLevel = cpl->ucTransparency;
                            main->gu_lDrawModeMask &= ~8;
                        }
                    } else {
                        main->gu_fTransparenceLevel = rate;
                    }
                }
                p = slot->hZoomSupObject;
                if (p == NULL)
                    p = slot->hSupObject;
                cpl++;
                fn_vUpdateSPOFlagsOfChannelForActivation(p, slot->bActiveChannel);
                chans = data->hArrayOfChannels;
            }
        next:
            if (cpl == end)
                break;
        }
        if (cpl != end)
            goto next;
    } else {
        chans = data->hArrayOfChannels;
    }
    fn_vUpdateFrameMatrix(so, frame3d, anim, frame, data->bAnimMatrixChanged, chans);
    data->bAnimMatrixChanged = 0;
    PLA_fn_vRefreshGlobalMatrixUnderCharacter(so);
}

s32 PLA_fn_bSkipCurrentFrame(HIE_tdstSuperObject_* p_stSuperObject)
{
    s32 now;
    s32 startTime;
    s32 animStartTime;
    s32 value;
    u16 frameCount;
    u16 old_frame;
    s32 lastFrame;
    u16 newFrame;
    u16 frame;
    u32 scaled;
    u32 product;
    u8 rate;
    gu_tdst3dData_* obj;
    tdstAnim3d_* anim;
    tdstState_* state;

    obj = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    state = obj->h_CurrentState;
    anim = state->p_stAnim;

    if (anim == 0)
        return 0;

    if (obj->ucFrameRate == 0)
        return 1;
    obj->ucUserEventFlags = 0;
    old_frame = obj->uwNbEngineFrameSinceLastMechEvent;
    obj->lLastComputeFrame = HIE_gs_lCurrentFrame;
    if (old_frame != 0xFFFF)
        obj->uwNbEngineFrameSinceLastMechEvent = old_frame + 1;

    if (obj->bSkipCurrentFrame != 0) {
        if (fn_p_stGetLinearSpeedVector(anim, obj->uwCurrentFrame) != 0)
            fn_vTransferSpeedInformationPLA_DNM(p_stSuperObject, obj->sLastFrame);
        obj->bSkipCurrentFrame = 0;

        rate = obj->ucFrameRate;
        now = inv_fn_ulGetEngineClock();
        startTime = obj->ulStartTime;
        scaled = (u32) (rate * (now - startTime)) / (u32) inv_GAM_ulTicksPerSecond;

        if ((u32) anim->uwNumberOfFrames < scaled) {
            now = inv_fn_ulGetEngineClock();
            frameCount = anim->uwNumberOfFrames;
            animStartTime = obj->ulStartTime;
            product = (u32) inv_GAM_ulTicksPerSecond * (u32) frameCount;
            rate = obj->ucFrameRate;
            value = now - (s32) (product / (u32) rate) - animStartTime;
            obj->ulTimeDelay = value;
            newFrame = anim->uwNumberOfFrames;
            obj->ulTimeDelay = value;
        } else {
            obj->ulTimeDelay = 0;
            newFrame = (u16) scaled;
        }

        if (obj->uwCurrentFrame == newFrame) {
            lastFrame = anim->uwNumberOfFrames;
            if (newFrame != lastFrame) {
                obj->bSkipCurrentFrame = 1;
                lastFrame = anim->uwNumberOfFrames;
            }
        } else {
            lastFrame = anim->uwNumberOfFrames;
        }
        frame = lastFrame & 0xFFFF;

        if (frame == 1) {
            if (obj->bStateJustModified == 0) {
                if (state->h_NextState == state) {
                    obj->bSkipCurrentFrame = frame;
                    obj->uwCurrentFrame = 0;
                    goto frame_done;
                }
            }
        }
        obj->uwCurrentFrame = newFrame;

    frame_done:
        frame = obj->uwCurrentFrame;
        if (frame == anim->uwNumberOfFrames)
            obj->ucFlagEndOfAnim = 1;

        {
            s32 bSkip = 1;
            if (obj->ucRepeatAnimation == 0) {
                bSkip = 1;
                if (frame == anim->uwNumberOfFrames) {
                    bSkip = 1;
                    if (state->h_NextState == 0)
                        obj->ucFlagEndState = bSkip;
                }
            }
            return bSkip;
        }
    }
    return 0;
}

static inline u8 inv_GAM_fn_ucEventPeriodOrOne(s32 period)
{
    if (period == 0)
        return 1;
    return period;
}

s32 PLA_fn_bDoAnimPlayerForCharacter(HIE_tdstSuperObject_* p_stSuperObject, u8 bNoDisplay, u8 _bHandleSkippedEventsIfRelevant)
{
    gu_tdst3dData_* data;
    tdstState_* state;
    tdstAnim3d_* anim;
    tdstState_* next;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* translation;
    MTH3D_tdstVector_ position;
    s32 changed;
    u8 speed;
    u32 frame;
    u32 time;
    u16 eventFrame;
    tdstEvent_* event;
    s32 i;
    tdstEventInTable_* definition;
    u8 c;

    data = *(gu_tdst3dData_**) p_stSuperObject->hLinkedObject;
    state = data->h_CurrentState;
    anim = state->p_stAnim;
    if (anim == NULL) {
        func_0010DA50(p_stSuperObject);
        return 1;
    }
    {
        if (data->uwCurrentFrame == anim->uwNumberOfFrames || data->ucFlagModifState == 1) {
            if (data->ucFlagModifState == 0)
                fn_vPlayLastEventsBeforeRestart(p_stSuperObject);
            else
                fn_vDesInitAllEvents(p_stSuperObject, data->h_WantedState, 1);

            if (data->ucRepeatAnimation != 0 && data->ucFlagModifState == 0) {
                frame = (data->ucFrameRate * (u32) data->ulTimeDelay) / inv_GAM_ulTicksPerSecond %
                        anim->uwNumberOfFrames;
                data->ulStartTime = inv_fn_ulGetEngineClock() -
                                    inv_GAM_ulTicksPerSecond * frame / data->ucFrameRate;
                data->uwCurrentFrame = frame;
                data->sLastFrame = -1;
                data->ucRepeatAnimation--;
            } else {
                __asm__ __volatile__("");
                if (data->ucFlagModifState == 0)
                    data->h_WantedState = data->h_CurrentState->h_NextState;
                next = data->h_WantedState;
                if (data->h_CurrentState != next && data->ucFlagModifState == 0)
                    fn_vDesInitAllEvents(p_stSuperObject, 0, 0);

                changed = 0;
                if (state != next || data->ucFlagModifState == 1)
                    changed = 1;

                if (changed) {
                    matrix = p_stSuperObject->gu_hLocalMatrix;
                    translation = (MTH3D_tdstVector_*) &matrix->stMatrix.stCol_3;
                    position.xX =
                        translation->xX - next->p_stAnim->stOffsetMatrix.xX;
                    position.xY =
                        translation->xY - next->p_stAnim->stOffsetMatrix.xY;
                    position.xZ =
                        translation->xZ - next->p_stAnim->stOffsetMatrix.xZ;
                    MTH3D_M_vAddXYZ_inv(&position, &position, &state->p_stAnim->stOffsetMatrix);
                    if (matrix->ucType == MAT_eTransform_Identity)
                        matrix->ucType = MAT_eTransform_Translation;
                    *translation = position;
                    ((f32*) translation)[3] = 1.0f;
                    _POS_fn_vMulMatrixMatrix(p_stSuperObject->gu_hGlobalMatrix,
                                             p_stSuperObject->hFatherSta->gu_hGlobalMatrix, matrix);

                    if (anim->ucMaxNumberOfElements !=
                            next->p_stAnim->ucMaxNumberOfElements ||
                        data->stFrame3d.p_stArrayOfElts3d == NULL)
                        fn_vAllocArrayOfElts3d(
                            data, next->p_stAnim->ucMaxNumberOfElements);
                    data->h_CurrentState = next;
                    data->stFrame3d.p_stAnim = 0;
                    fn_vUpdateCurrentActivationForCharacter(p_stSuperObject);
                    state = data->h_CurrentState;
                    anim = state->p_stAnim;
                    speed = *(u8*) &state->scSpeedAnim;
                    if (_bHandleSkippedEventsIfRelevant) {
                        fn_vInitAllEvents(p_stSuperObject, 0);
                        event = anim->d_stAnimEvent;
                        i = 0;
                        if (event != NULL) {
                            eventFrame = data->sForcedFrame;
                            for (; i < anim->ucNumberOfEvents; i++, event++) {
                                if (eventFrame < event->uwFrameNumber)
                                    break;
                                definition = fn_p_GetEventInTable(data, event);
                                if (definition->ucTypeOfEvent == 0 || definition->ucTypeOfEvent == 4) {
                                    data->d_ucEventActivation[i] = inv_GAM_fn_ucEventPeriodOrOne(definition->ucFirstCall);
                                }
                            }
                        }
                    } else {
                        fn_vInitAllEvents(p_stSuperObject, (u16) data->sForcedFrame);
                    }
                    fn_v3dDataSetFrameRate(data, speed);
                } else {
                    speed = data->ucFrameRate;
                }

                time = data->ulTimeDelay;
                frame = (speed * time) / inv_GAM_ulTicksPerSecond % anim->uwNumberOfFrames;
                if (changed)
                    data->bStateJustModified = 1;
                if ((u16) data->sForcedFrame != 0) {
                    data->uwCurrentFrame = data->sForcedFrame;
                    data->sForcedFrame = 0;
                } else {
                    data->uwCurrentFrame = frame;
                    data->sLastFrame = -1;
                    data->ulStartTime = data->ulTimePreviousFrame - time;
                }
                data->ucRepeatAnimation = state->ucRepeatAnimation;
                data->h_WantedState = state;
                data->ucFlagEndState = 0;
                data->ucFlagModifState = 0;
            }
        }
        if (!bNoDisplay) {
            PLA_fn_vInitAllChildInHeapNewAnim(p_stSuperObject, anim, data->uwCurrentFrame);
            PLA_fn_vDoFrame(anim, data->uwCurrentFrame, p_stSuperObject);
        }
        if (!_bHandleSkippedEventsIfRelevant)
            fn_vPlayEventsSinceLastOneNoLoop(p_stSuperObject);
        fn_vTransferSpeedInformationPLA_DNM(p_stSuperObject, data->sLastFrame);
    }
    func_0010DA50(p_stSuperObject);
    return 1;
}

void func_0010DA50(HIE_tdstSuperObject_* arg)
{
    gu_tdst3dData_* state;
    tdstState_* animationRef;
    tdstAnim3d_* animationData;
    u32 frameAdvance;
    u16 nextFrame;
    s32 clock;
    s32 previousFrame;
    u32 speed;

    state = ((tdstEngineObject_*) arg->hLinkedObject)->h_3dData;
    animationRef = state->h_CurrentState;
    if (animationRef == NULL) {
        return;
    }
    animationData = animationRef->p_stAnim;
    if (animationData == NULL) {
        return;
    }

    previousFrame = (s16) state->uwCurrentFrame;
    state->sLastFrame = previousFrame;
    speed = state->ucFrameRate;
    frameAdvance =
        (speed * (inv_fn_ulGetEngineClock() - state->ulStartTime)) / inv_GAM_ulTicksPerSecond;

    if (frameAdvance > animationData->uwNumberOfFrames) {
        if (state->ucFlagEndOfAnim == 0) {
            clock = inv_fn_ulGetEngineClock();
            state->ulTimeDelay =
                (clock - state->ulStartTime) -
                (inv_GAM_ulTicksPerSecond * animationData->uwNumberOfFrames) /
                    (u32) state->ucFrameRate;
            nextFrame = animationData->uwNumberOfFrames;
        } else {
            state->ulTimeDelay = 0;
            nextFrame = 0;
            state->ulStartTime = inv_fn_ulGetEngineClock();
        }
    } else {
        state->ulTimeDelay = 0;
        nextFrame = frameAdvance;
    }

    if ((u16) state->uwCurrentFrame == nextFrame && state->bStateJustModified == 0 &&
        nextFrame != animationData->uwNumberOfFrames) {
        state->bSkipCurrentFrame = 1;
    }

    if (animationData->uwNumberOfFrames == 1 && state->bStateJustModified == 0 && animationRef->h_NextState == animationRef) {
        state->bSkipCurrentFrame = 1;
        state->uwCurrentFrame = 0;
    } else {
        state->uwCurrentFrame = nextFrame;
    }

    state->ucFlagEndOfAnim = (nextFrame == animationData->uwNumberOfFrames);
    if (state->ucRepeatAnimation == 0) {
        if ((u16) state->uwCurrentFrame == animationData->uwNumberOfFrames && animationRef->h_NextState == NULL) {
            state->ucFlagEndState = 1;
        }
    }

    state->bStateJustModified = 0;
    state->ulTimePreviousFrame = inv_fn_ulGetEngineClock();
}

void PLA_fn_vInitMatrixHeap(void)
{
    s32 index;
    s32 offset;

    index = 0;
    offset = 0;
    do {
        index = index + 1;
        gu_MTH4D_M_vSetIdentityMatrix(
            (POS_tdstMatrix_*) ((u8*) PLA_g_d_stMatrixHeap + offset));
        offset = offset + 0x54;
    } while ((u32) index < 0x201u);
}

void PLA_fn_vCreateMatrixHeap(void)
{
    PLA_g_d_stMatrixHeap = (POS_tdstMatrix_*) AI_fn_p_vTrueAlloc(gp_6F74, 0xA854);
}

void PLA_fn_vInitSupObjHeap(void)
{
    extern HIE_tdstSuperObject_* PLA_g_d_stSupObjHeap;

    HIE_tdstSuperObject_* p_stSO;
    HIE_tdstSuperObject_* end;
    POS_tdstMatrix_* matrix;

    p_stSO = PLA_g_d_stSupObjHeap;
    end = p_stSO + 0x201;
    matrix = PLA_g_d_stMatrixHeap;
    memset(PLA_g_a_wStateOfSupObjHeap, 0, 0x400);
    while (p_stSO != end) {
        p_stSO->gu_hLocalMatrix = matrix;
        matrix++;
        p_stSO++;
    }
    (&PLA_g_d_stSupObjHeap)[1] = 0;
    PLA_g_ulNbAllocInSupObjHeap = 0;
}

void PLA_fn_vCreateSupObjHeap(void)
{
    extern HIE_tdstSuperObject_* PLA_g_d_stSupObjHeap;
#define gp_67F0 PLA_g_d_stSupObjHeap
    gp_67F0 = HIE_fn_p_stCreateSuperObjectTab(0x201);
}

void PLA_fn_vInitHeaps(void)
{
    PLA_fn_vInitMatrixHeap();
    PLA_fn_vInitSupObjHeap();
}

void PLA_fn_vCreateAndInitHeaps(void)
{
    PLA_fn_vCreateMatrixHeap();
    PLA_fn_vCreateSupObjHeap();
    PLA_fn_vInitMatrixHeap();
    PLA_fn_vInitSupObjHeap();
}

void* PLA_fn_hFindNextFreeSupObj(void)
{
    extern s32 PLA_g_d_stSupObjHeap;

    HIE_tdstSuperObject_* obj;
    HIE_tdstSuperObject_* bro;
    HIE_tdstSuperObject_* father;
    HIE_tdstSuperObject_* ch;
    s32 slot;
    s32 count;
    s32 limit;
    s32 flag;
    s32 base;
    s32 off;
    u16* cell;

    count = PLA_g_ulNbAllocInSupObjHeap;
    count = count + 1;
    slot = PLA_g_ulCurrentPosInSupObjHeap;
    if ((u32) count >= 0x200) {
        obj = (HIE_tdstSuperObject_*) (PLA_g_d_stSupObjHeap + 0x8000);
    } else {
        base = (s32) PLA_g_a_wStateOfSupObjHeap;
        do {
            off = slot << 1;
        } while (0);
        cell = (u16*) (off + base);
        if (*cell != 0) {
            limit = 0x200;
            do {
                slot += 1;
                cell += 1;
                if (slot == limit) {
                    slot = 0;
                    cell = PLA_g_a_wStateOfSupObjHeap;
                }
            } while (*cell != 0);
        }
        *cell = 1;
        PLA_g_ulCurrentPosInSupObjHeap = slot;
        obj = (HIE_tdstSuperObject_*) (PLA_g_d_stSupObjHeap + (slot << 6));
    }
    obj->flags = 0;
    obj->gu_lDrawModeMask = -1;
    PLA_g_ulNbAllocInSupObjHeap = count;
    bro = obj->hNextBrotherSta;
    if (bro != NULL) {
        bro->hPrevBrotherSta = obj->hPrevBrotherSta;
    }
    bro = obj->hPrevBrotherSta;
    if (bro != NULL) {
        bro->hNextBrotherSta = obj->hNextBrotherSta;
    }
    father = obj->hFatherSta;
    if (father != NULL) {
        father->lNumberOfElementsSta -= 1;
        father = obj->hFatherSta;
        ch = father->hFirstElementSta;
        if (ch == obj) {
            father->hFirstElementSta = obj->hNextBrotherSta;
            father = obj->hFatherSta;
        }
        ch = father->hLastElementSta;
        if (ch == obj) {
            father->hLastElementSta = obj->hPrevBrotherSta;
        }
    }
    flag = 1;
    obj->lTypeOfLinkedObject = flag;
    obj->hNextBrotherSta = NULL;
    obj->hPrevBrotherSta = NULL;
    obj->hFatherSta = NULL;
    obj->hLinkedObject = NULL;
    return obj;
}

void PLA_fn_vReleaseSuperObjectInHeap(HIE_tdstSuperObject_* p_stSO)
{
    extern s32 PLA_g_d_stSupObjHeap;
    s32 base;

    base = PLA_g_d_stSupObjHeap;
    if (p_stSO != (HIE_tdstSuperObject_*) (base + 0x8000)) {
        PLA_g_a_wStateOfSupObjHeap[((s32) p_stSO - base) >> 6] = 0;
    }
    PLA_g_ulNbAllocInSupObjHeap -= 1;
}

void PLA_fn_vUpdateTransparencyForModules(HIE_tdstSuperObject_* p_stSuperObject)
{
    HIE_tdstSuperObject_* object = p_stSuperObject;
    tdstAChannel_* p_stChannel;
    u32 transparencyFlag;
    f32 fTransparenceLevel;

    fTransparenceLevel = object->gu_fTransparenceLevel;
    transparencyFlag = (object->gu_lDrawModeMask >> 3) & 1;
    p_stChannel = ((tdstEngineObject_*) object->hLinkedObject)->h_3dData->hFirstActiveChannel;

    if (p_stChannel != 0) {
        do {
            HIE_tdstSuperObject_* p_stTargetSO = p_stChannel->hSupObject;
            if ((p_stTargetSO->flags & 0x10000) != 0) {
                p_stChannel = p_stChannel->p_stNextActiveChannel;
            } else {
                p_stTargetSO->gu_fTransparenceLevel = fTransparenceLevel;
                if (transparencyFlag != 0) {
                    p_stTargetSO->gu_lDrawModeMask |= 8;
                }
                p_stChannel = p_stChannel->p_stNextActiveChannel;
            }
        } while (p_stChannel != 0);
    }
}

typedef struct __attribute__((packed, aligned(4))) FrameTranslationWords {
    u32 x;
    u32 y;
    u32 z;
} FrameTranslationWords;

void PLA_fn_vInitAllChildInHeapNewAnim(HIE_tdstSuperObject_* p_stSuperObject, tdstAnim3d_* p_stCurrentAnim, u16 _uwFrameNumber)
{
#define POOL_AT(base, idx) ((tdstAChannel_*) ((u8*) (base) + ((idx) << 5)))

    gu_tdst3dData_* state;
    void* shared_pp;
    tdstCouple_* old_pairs;
    tdstCouple_* new_pairs;
    tdstElement3d_* token;
    tdstElement3d_* token_end;
    tdstAChannel_* pool;
    tdstAChannel_* wanted;
    tdstAChannel_* current;
    s32 changed;
    u32 count;
    u32 token_index;

    state = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    shared_pp = &state->stFrame3d;
    old_pairs = state->p_stCurrentHieCouples;
    pool = state->hArrayOfChannels;

    fn_vUpdateFrame(p_stSuperObject, shared_pp, p_stCurrentAnim, _uwFrameNumber);

    token = ((tdstFrame3d_*) shared_pp)->p_stArrayOfElts3d;
    new_pairs = ((tdstFrame3d_*) shared_pp)->stHierarchy.d_stCouples;

    if (pool == NULL) {
        fn_vInitCSOList(&pool, state->ulNumberOfChannels);
        state->hArrayOfChannels = pool;
        state->hFirstActiveChannel = NULL;
    }

    changed = 0;
    if (old_pairs != new_pairs) {
        changed = 1;
        if (old_pairs != NULL) {
            count = (u16) state->wCurrentHieNbCouples;
            while (--count != 0xFFFFFFFFU) {
                u32 first;
                u32 base;
                first = old_pairs->wChild;
                base = (u32) pool;
                old_pairs++;
                first <<= 5;
                first += base;
                ((tdstAChannel_*) first)->p_stFatherChannel = NULL;
            }
        }

        count = ((tdstFrame3d_*) shared_pp)->stHierarchy.ulNbOfCouples;
        state->p_stCurrentHieCouples = new_pairs;
        state->wCurrentHieNbCouples = count;
        while (--count != 0xFFFFFFFFU) {
            u32 first;
            u32 second;
            u32 base;
            u32 second_addr;
            first = new_pairs->wChild;
            second = new_pairs->wFather;
            base = (u32) pool;
            first <<= 5;
            second <<= 5;
            new_pairs++;
            first += base;
            second_addr = base + second;
            ((tdstAChannel_*) first)->p_stFatherChannel = (tdstAChannel_*) second_addr;
        }
        token_end = token + p_stCurrentAnim->ucMaxNumberOfElements;
    } else {
        token_end = token + ((volatile tdstAnim3d_*) p_stCurrentAnim)->ucMaxNumberOfElements;
    }

    if (token != token_end) {
        token_index = token->ucChannelNumber;
        if (token_index != 0xFF) {
            wanted = POOL_AT(pool, token_index);
            goto initial_wanted_done;
        }
    }
    wanted = NULL;

initial_wanted_done:

    shared_pp = NULL;
    current = state->hFirstActiveChannel;
    while (wanted != NULL || current != NULL) {
        if (wanted == current) {
            if (changed != 0) {
                if (current->p_stFatherChannel != NULL) {
                    HIE_tdstSuperObject_* obj;
                    HIE_tdstSuperObject_* owner;

                    if (current->p_stFatherChannel->hSupObject == NULL) {
                        current->p_stFatherChannel->hSupObject = PLA_fn_hFindNextFreeSupObj();
                    }
                    obj = current->hSupObject;
                    owner = current->p_stFatherChannel->hSupObject;

                    if (obj->hNextBrotherSta != NULL) {
                        obj->hNextBrotherSta->hPrevBrotherSta = obj->hPrevBrotherSta;
                    }
                    if (obj->hPrevBrotherSta != NULL) {
                        obj->hPrevBrotherSta->hNextBrotherSta = obj->hNextBrotherSta;
                    }
                    if (obj->hFatherSta != NULL) {
                        obj->hFatherSta->lNumberOfElementsSta--;
                        if (obj->hFatherSta->hFirstElementSta == obj) {
                            obj->hFatherSta->hFirstElementSta = obj->hNextBrotherSta;
                        }
                        if (obj->hFatherSta->hLastElementSta == obj) {
                            obj->hFatherSta->hLastElementSta = obj->hPrevBrotherSta;
                        }
                    }
                    obj->hPrevBrotherSta = NULL;
                    obj->hFatherSta = NULL;
                    obj->hNextBrotherSta = NULL;

                    obj->hPrevBrotherSta = owner->hLastElementSta;
                    obj->hFatherSta = owner;
                    if (owner->hFirstElementSta == NULL) {
                        owner->hFirstElementSta = obj;
                    } else {
                        owner->hLastElementSta->hNextBrotherSta = obj;
                    }
                    owner->hLastElementSta = obj;
                    owner->lNumberOfElementsSta++;
                } else {
                    HIE_tdstSuperObject_* obj;

                    obj = current->hSupObject;
                    if (obj->hNextBrotherSta != NULL) {
                        obj->hNextBrotherSta->hPrevBrotherSta = obj->hPrevBrotherSta;
                    }
                    if (obj->hPrevBrotherSta != NULL) {
                        obj->hPrevBrotherSta->hNextBrotherSta = obj->hNextBrotherSta;
                    }
                    if (obj->hFatherSta != NULL) {
                        obj->hFatherSta->lNumberOfElementsSta--;
                        if (obj->hFatherSta->hFirstElementSta == obj) {
                            obj->hFatherSta->hFirstElementSta = obj->hNextBrotherSta;
                        }
                        if (obj->hFatherSta->hLastElementSta == obj) {
                            obj->hFatherSta->hLastElementSta = obj->hPrevBrotherSta;
                        }
                    }
                    obj->hPrevBrotherSta = NULL;
                    obj->hFatherSta = NULL;
                    obj->hNextBrotherSta = NULL;

                    obj->hPrevBrotherSta = p_stSuperObject->hLastElementSta;
                    obj->hFatherSta = p_stSuperObject;
                    if (p_stSuperObject->hFirstElementSta == NULL) {
                        p_stSuperObject->hFirstElementSta = obj;
                    } else {
                        p_stSuperObject->hLastElementSta->hNextBrotherSta = obj;
                    }
                    p_stSuperObject->hLastElementSta = obj;
                    p_stSuperObject->lNumberOfElementsSta++;
                }
            }

            shared_pp = current;
            token++;
            current = current->p_stNextActiveChannel;
            if (token == token_end) {
                wanted = NULL;
                goto loop_tail;
            }
            token_index = token->ucChannelNumber;
            if (token_index != 0xFF) {
                goto set_wanted;
            }
            wanted = NULL;
            goto loop_tail;
        }

        if (current == NULL || (wanted != NULL && wanted < current)) {
            if (wanted->hSupObject == NULL) {
                wanted->hSupObject = PLA_fn_hFindNextFreeSupObj();
            }

            if (wanted->p_stFatherChannel != NULL) {
                HIE_tdstSuperObject_* obj;
                HIE_tdstSuperObject_* owner;

                if (wanted->p_stFatherChannel->hSupObject == NULL) {
                    wanted->p_stFatherChannel->hSupObject = PLA_fn_hFindNextFreeSupObj();
                }
                obj = wanted->hSupObject;
                owner = wanted->p_stFatherChannel->hSupObject;

                if (obj->hNextBrotherSta != NULL) {
                    obj->hNextBrotherSta->hPrevBrotherSta = obj->hPrevBrotherSta;
                }
                if (obj->hPrevBrotherSta != NULL) {
                    obj->hPrevBrotherSta->hNextBrotherSta = obj->hNextBrotherSta;
                }
                if (obj->hFatherSta != NULL) {
                    obj->hFatherSta->lNumberOfElementsSta--;
                    if (obj->hFatherSta->hFirstElementSta == obj) {
                        obj->hFatherSta->hFirstElementSta = obj->hNextBrotherSta;
                    }
                    if (obj->hFatherSta->hLastElementSta == obj) {
                        obj->hFatherSta->hLastElementSta = obj->hPrevBrotherSta;
                    }
                }
                obj->hPrevBrotherSta = NULL;
                obj->hFatherSta = NULL;
                obj->hNextBrotherSta = NULL;

                obj->hPrevBrotherSta = owner->hLastElementSta;
                obj->hFatherSta = owner;
                if (owner->hFirstElementSta == NULL) {
                    owner->hFirstElementSta = obj;
                } else {
                    owner->hLastElementSta->hNextBrotherSta = obj;
                }
                owner->hLastElementSta = obj;
                owner->lNumberOfElementsSta++;
            } else {
                HIE_tdstSuperObject_* obj;

                obj = wanted->hSupObject;
                if (obj->hNextBrotherSta != NULL) {
                    obj->hNextBrotherSta->hPrevBrotherSta = obj->hPrevBrotherSta;
                }
                if (obj->hPrevBrotherSta != NULL) {
                    obj->hPrevBrotherSta->hNextBrotherSta = obj->hNextBrotherSta;
                }
                if (obj->hFatherSta != NULL) {
                    obj->hFatherSta->lNumberOfElementsSta--;
                    if (obj->hFatherSta->hFirstElementSta == obj) {
                        obj->hFatherSta->hFirstElementSta = obj->hNextBrotherSta;
                    }
                    if (obj->hFatherSta->hLastElementSta == obj) {
                        obj->hFatherSta->hLastElementSta = obj->hPrevBrotherSta;
                    }
                }
                obj->hPrevBrotherSta = NULL;
                obj->hFatherSta = NULL;
                obj->hNextBrotherSta = NULL;

                obj->hPrevBrotherSta = p_stSuperObject->hLastElementSta;
                obj->hFatherSta = p_stSuperObject;
                if (p_stSuperObject->hFirstElementSta == NULL) {
                    p_stSuperObject->hFirstElementSta = obj;
                } else {
                    p_stSuperObject->hLastElementSta->hNextBrotherSta = obj;
                }
                p_stSuperObject->hLastElementSta = obj;
                p_stSuperObject->lNumberOfElementsSta++;
            }

            if (shared_pp != NULL) {
                wanted->p_stNextActiveChannel = ((tdstAChannel_*) shared_pp)->p_stNextActiveChannel;
                ((tdstAChannel_*) shared_pp)->p_stNextActiveChannel = wanted;
            } else {
                wanted->p_stNextActiveChannel = state->hFirstActiveChannel;
                state->hFirstActiveChannel = wanted;
            }
            shared_pp = wanted;
            token++;
            if (token == token_end) {
                goto no_wanted_insert;
            }
            token_index = token->ucChannelNumber;
            if (token_index == 0xFF) {
                goto no_wanted_insert;
            }
        } else {
            goto removal_check;
        }
    set_wanted:
        wanted = POOL_AT(pool, token_index);
        goto loop_tail;

    no_wanted_insert:
        wanted = NULL;
        goto loop_tail;

    removal_check:
        if (wanted == NULL || current < wanted) {
            if (current->hZoomSupObject != NULL) {
                if (current->hZoomSupObject->hNextBrotherSta != NULL) {
                    current->hZoomSupObject->hNextBrotherSta->hPrevBrotherSta =
                        current->hZoomSupObject->hPrevBrotherSta;
                }
                if (current->hZoomSupObject->hPrevBrotherSta != NULL) {
                    current->hZoomSupObject->hPrevBrotherSta->hNextBrotherSta =
                        current->hZoomSupObject->hNextBrotherSta;
                }
                if (current->hZoomSupObject->hFatherSta != NULL) {
                    current->hZoomSupObject->hFatherSta->lNumberOfElementsSta--;
                    if (current->hZoomSupObject->hFatherSta->hFirstElementSta == current->hZoomSupObject) {
                        current->hZoomSupObject->hFatherSta->hFirstElementSta =
                            current->hZoomSupObject->hNextBrotherSta;
                    }
                    if (current->hZoomSupObject->hFatherSta->hLastElementSta == current->hZoomSupObject) {
                        current->hZoomSupObject->hFatherSta->hLastElementSta =
                            current->hZoomSupObject->hPrevBrotherSta;
                    }
                }
                current->hZoomSupObject->hNextBrotherSta = NULL;
                current->hZoomSupObject->hPrevBrotherSta = NULL;
                current->hZoomSupObject->hFatherSta = NULL;
                PLA_fn_vReleaseSuperObjectInHeap(current->hZoomSupObject);
                current->hZoomSupObject = NULL;
            }

            if (current->hSupObject->hNextBrotherSta != NULL) {
                current->hSupObject->hNextBrotherSta->hPrevBrotherSta =
                    current->hSupObject->hPrevBrotherSta;
            }
            if (current->hSupObject->hPrevBrotherSta != NULL) {
                current->hSupObject->hPrevBrotherSta->hNextBrotherSta =
                    current->hSupObject->hNextBrotherSta;
            }
            if (current->hSupObject->hFatherSta != NULL) {
                current->hSupObject->hFatherSta->lNumberOfElementsSta--;
                if (current->hSupObject->hFatherSta->hFirstElementSta == current->hSupObject) {
                    current->hSupObject->hFatherSta->hFirstElementSta = current->hSupObject->hNextBrotherSta;
                }
                if (current->hSupObject->hFatherSta->hLastElementSta == current->hSupObject) {
                    current->hSupObject->hFatherSta->hLastElementSta = current->hSupObject->hPrevBrotherSta;
                }
            }
            current->hSupObject->hNextBrotherSta = NULL;
            current->hSupObject->hPrevBrotherSta = NULL;
            current->hSupObject->hFatherSta = NULL;
            PLA_fn_vReleaseSuperObjectInHeap(current->hSupObject);
            current->hSupObject = NULL;

            if (shared_pp != NULL) {
                ((tdstAChannel_*) shared_pp)->p_stNextActiveChannel = current->p_stNextActiveChannel;
            } else {
                state->hFirstActiveChannel = current->p_stNextActiveChannel;
            }
            current = current->p_stNextActiveChannel;
        }
        goto loop_tail;

    loop_tail:;
    }

    {
        tdstAChannel_* scan;

        scan = state->hFirstActiveChannel;
        while (scan != NULL) {
            *(FrameTranslationWords*) &scan->inv_stLastPosition =
                *(FrameTranslationWords*) &scan->hSupObject->gu_hGlobalMatrix->stMatrix.stCol_3;
            scan = scan->p_stNextActiveChannel;
        }
    }
}

void PLA_fn_vDesInitAllChildOfCharacter(HIE_tdstSuperObject_* p_stSuperObject, tdstEngineObject_* p_stObject)
{
    gu_tdst3dData_* h_Current3dData;
    tdstAChannel_* p_stChannel;
    tdstState_* h_State;
    tdstAChannel_* hArray;
    tdstAnim3d_* p_stCurrentAnim;
    tdstFrame3d_* p_stFrame3d;

    p_stCurrentAnim = 0;
    if (p_stObject == 0)
        return;

    h_Current3dData = p_stObject->h_3dData;
    h_State = h_Current3dData->h_CurrentState;
    if (h_State != 0)
        p_stCurrentAnim = h_State->p_stAnim;

    hArray = h_Current3dData->hArrayOfChannels;
    p_stChannel = h_Current3dData->hFirstActiveChannel;

    if (hArray != 0) {
        if (p_stChannel != 0) {
            p_stFrame3d = &h_Current3dData->stFrame3d;
            do {
                if (p_stSuperObject != 0 && p_stCurrentAnim != 0 && p_stCurrentAnim->d_stMorphDataArray != 0) {
                    MOR_fn_xStopMorph(p_stSuperObject, (s32) (((u8*) p_stChannel - (u8*) hArray) << 11) >> 16);
                }

                if (p_stChannel->hZoomSupObject != 0) {
                    if (p_stChannel->hZoomSupObject->hNextBrotherSta != 0)
                        p_stChannel->hZoomSupObject->hNextBrotherSta->hPrevBrotherSta = p_stChannel->hZoomSupObject->hPrevBrotherSta;
                    if (p_stChannel->hZoomSupObject->hPrevBrotherSta != 0)
                        p_stChannel->hZoomSupObject->hPrevBrotherSta->hNextBrotherSta = p_stChannel->hZoomSupObject->hNextBrotherSta;
                    if (p_stChannel->hZoomSupObject->hFatherSta != 0) {
                        p_stChannel->hZoomSupObject->hFatherSta->lNumberOfElementsSta--;
                        if (p_stChannel->hZoomSupObject->hFatherSta->hFirstElementSta == p_stChannel->hZoomSupObject)
                            p_stChannel->hZoomSupObject->hFatherSta->hFirstElementSta = p_stChannel->hZoomSupObject->hNextBrotherSta;
                        if (p_stChannel->hZoomSupObject->hFatherSta->hLastElementSta == p_stChannel->hZoomSupObject)
                            p_stChannel->hZoomSupObject->hFatherSta->hLastElementSta = p_stChannel->hZoomSupObject->hPrevBrotherSta;
                    }
                    p_stChannel->hZoomSupObject->hNextBrotherSta = 0;
                    p_stChannel->hZoomSupObject->hPrevBrotherSta = 0;
                    p_stChannel->hZoomSupObject->hFatherSta = 0;
                    PLA_fn_vReleaseSuperObjectInHeap(p_stChannel->hZoomSupObject);
                    p_stChannel->hZoomSupObject = 0;
                }

                if (p_stChannel->hSupObject != 0) {
                    if (p_stChannel->hSupObject->hNextBrotherSta != 0)
                        p_stChannel->hSupObject->hNextBrotherSta->hPrevBrotherSta = p_stChannel->hSupObject->hPrevBrotherSta;
                    if (p_stChannel->hSupObject->hPrevBrotherSta != 0)
                        p_stChannel->hSupObject->hPrevBrotherSta->hNextBrotherSta = p_stChannel->hSupObject->hNextBrotherSta;
                    if (p_stChannel->hSupObject->hFatherSta != 0) {
                        p_stChannel->hSupObject->hFatherSta->lNumberOfElementsSta--;
                        if (p_stChannel->hSupObject->hFatherSta->hFirstElementSta == p_stChannel->hSupObject)
                            p_stChannel->hSupObject->hFatherSta->hFirstElementSta = p_stChannel->hSupObject->hNextBrotherSta;
                        if (p_stChannel->hSupObject->hFatherSta->hLastElementSta == p_stChannel->hSupObject)
                            p_stChannel->hSupObject->hFatherSta->hLastElementSta = p_stChannel->hSupObject->hPrevBrotherSta;
                    }
                    p_stChannel->hSupObject->hNextBrotherSta = 0;
                    p_stChannel->hSupObject->hPrevBrotherSta = 0;
                    p_stChannel->hSupObject->hFatherSta = 0;
                    PLA_fn_vReleaseSuperObjectInHeap(p_stChannel->hSupObject);
                    p_stChannel->hSupObject = 0;
                }

                p_stChannel = p_stChannel->p_stNextActiveChannel;
            } while (p_stChannel != 0);
        } else {
            p_stFrame3d = &h_Current3dData->stFrame3d;
        }
    } else {
        p_stFrame3d = &h_Current3dData->stFrame3d;
    }

    h_Current3dData->bSkipCurrentFrame = 0;
    if (p_stSuperObject != 0)
        fn_vDesInitAllEvents(p_stSuperObject, 0, 1);

    fn_vFreeCSOList(&hArray, (u32) h_Current3dData->ulNumberOfChannels);
    h_Current3dData->hArrayOfChannels = 0;
    h_Current3dData->hFirstActiveChannel = 0;
    fn_vFreeArrayOfElts3d(h_Current3dData);

    p_stFrame3d->p_stAnim = 0;
    if (p_stFrame3d->p_stAngularSpeedMatrix != 0) {
        inv_MEM_fn_vFreeInHeap(gp_6F74, p_stFrame3d->p_stAngularSpeedMatrix);
        p_stFrame3d->p_stAngularSpeedMatrix = 0;
    }

    h_Current3dData->p_stCurrentHieCouples = 0;
    h_Current3dData->wCurrentHieNbCouples = 0;
    h_Current3dData->ucFlagModifState = 1;
    h_Current3dData->bSkipCurrentFrame = 0;
}

void PLA_fn_vRefreshGlobalMatrixOfAnimsHierarchy(HIE_tdstSuperObject_* superObject)
{
    HIE_tdstSuperObject_* curr;

    if (superObject->lTypeOfLinkedObject & 9) {
        _POS_fn_vMulMatrixMatrix(superObject->gu_hGlobalMatrix, superObject->hFatherSta->gu_hGlobalMatrix,
                                 superObject->gu_hLocalMatrix);
        superObject->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;

        curr = superObject->hFirstElementSta;
        if (curr != NULL) {
            do {
                PLA_fn_vRefreshGlobalMatrixOfAnimsHierarchy(curr);
                curr = curr->hNextBrotherSta;
            } while (curr != NULL);
        }
    }
}

void PLA_fn_vRefreshGlobalMatrixUnderCharacter(HIE_tdstSuperObject_* superObject)
{
    HIE_tdstSuperObject_* curr;

    if (fn_ulStandardGameGetCustomBitsSO(superObject) & 2) {
        curr = superObject->hFirstElementSta;
        if (curr != NULL) {
            do {
                PLA_fn_vRefreshGlobalMatrixOfAnimsHierarchy(curr);
                curr = curr->hNextBrotherSta;
            } while (curr != NULL);
        }
    }
}

s32 PLA_fn_bSetNewState(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_WantedState, u8 _bForce, u8 _bHandleSkippedEventsIfRelevant, u8 _bIgnoreTransitions)
{
    gu_tdst3dData_* h_Current3dData;
    tdstTransition_* h_Transition;
    tdstState_* h_ResultState;
    u32 nextFrame;
    u32 adjustedClock;

    h_Current3dData = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;

    if (h_WantedState == h_Current3dData->h_FirstStateOfAction) {
        if (!_bForce) {
            return 1;
        }
    }

    if (!_bIgnoreTransitions) {
        h_Transition = fn_hIfExistTransitionalWithTargetState(h_Current3dData->h_WantedState, h_WantedState);
        if (h_Transition == NULL) {
            if (h_Current3dData->h_WantedState->ucTransitionStatusFlag == 1) {
                return 0;
            }
            if (fn_hIfExistProhibitedState(h_Current3dData->h_WantedState, h_WantedState) != 0) {
                return 0;
            }
            h_ResultState = h_WantedState;
        } else {
            h_ResultState = h_Transition->h_StateToGo;
        }
    } else {
        h_ResultState = h_WantedState;
        h_Transition = NULL;
    }

    PLA_fn_vStopCurrentMorphAnim(p_stSuperObject);

    if (h_Transition != NULL && h_Transition->ucLinkingType == 1) {
        nextFrame = h_Current3dData->uwCurrentFrame;
        nextFrame *= h_ResultState->p_stAnim->uwNumberOfFrames;
        nextFrame /= h_Current3dData->h_CurrentState->p_stAnim->uwNumberOfFrames;
        if (nextFrame >= h_ResultState->p_stAnim->uwNumberOfFrames) {
            nextFrame = h_ResultState->p_stAnim->uwNumberOfFrames - 1;
        }
        adjustedClock = inv_fn_ulGetEngineClock() - h_Current3dData->ulStartTime;
        adjustedClock *= h_ResultState->p_stAnim->uwNumberOfFrames;
        adjustedClock /= h_Current3dData->h_CurrentState->p_stAnim->uwNumberOfFrames;
        adjustedClock = inv_fn_ulGetEngineClock() - adjustedClock;
        h_Current3dData->sForcedFrame = nextFrame;
        h_Current3dData->sLastFrame = nextFrame;
        h_Current3dData->ulStartTime = adjustedClock;
        h_Current3dData->ulTimePreviousFrame =
            adjustedClock +
            ((u32) nextFrame * inv_GAM_ulTicksPerSecond) / h_ResultState->scSpeedAnim;
        h_Current3dData->bSkipCurrentFrame = 0;
    } else {
        h_Current3dData->ulTimeDelay = 0;
        h_Current3dData->uwCurrentFrame = 0;
        h_Current3dData->ulTimePreviousFrame = inv_fn_ulGetEngineClock();
        h_Current3dData->bSkipCurrentFrame = 0;
    }

    h_Current3dData->h_StateInLastFrame = h_Current3dData->h_CurrentState;
    h_Current3dData->uwNbEngineFrameSinceLastMechEvent = 0xFFFF;
    h_Current3dData->ucFlagModifState = 1;
    h_Current3dData->h_FirstStateOfAction = h_WantedState;
    h_Current3dData->h_WantedState = h_ResultState;
    h_Current3dData->ucFlagEndOfAnim = ((u16) h_Current3dData->sForcedFrame == h_ResultState->p_stAnim->uwNumberOfFrames);

    PLA_fn_bDoAnimPlayerForCharacter(p_stSuperObject, 0, _bHandleSkippedEventsIfRelevant);
    return 1;
}

s32 PLA_fn_bSetNewStateWithRandom(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_WantedState)
{
    tdstEngineObject_* object = p_stSuperObject->hLinkedObject;
    gu_tdst3dData_* data = object->h_3dData;
    tdstState_* animation;
    f32 scale;
    s32 frame;
    tdstState_* next;

    if (h_WantedState == data->h_FirstStateOfAction)
        return 1;
    fn_hIfExistTransitionalWithTargetState(data->h_WantedState, h_WantedState);
    if (fn_hIfExistProhibitedState(data->h_WantedState, h_WantedState) != 0)
        return 0;
    if (data->h_WantedState->ucTransitionStatusFlag == 1)
        return 0;

    animation = data->h_CurrentState;
    next = (tdstState_*) ((u32) h_WantedState + (u32) animation - (u32) animation);
    data->ucFlagModifState = 1;
    data->uwNbEngineFrameSinceLastMechEvent = 0xFFFF;
    data->h_StateInLastFrame = animation;
    data->ulStartTime = 0;
    data->h_WantedState = next;
    data->h_FirstStateOfAction = next;
    scale = (f32) (next->p_stAnim->uwNumberOfFrames - 1) + 1.0f;
    frame = (s32) ((f32) rand() * (scale * RAND_SCALE_inv)) & 0xFF;
    data->ulTimeDelay = 0;
    data->sForcedFrame = frame;
    data->sLastFrame = frame - 1;
    data->ulTimePreviousFrame = inv_fn_ulGetEngineClock();
    PLA_fn_bDoAnimPlayerForCharacter(p_stSuperObject, 0, 0);
    return 1;
}

void PLA_fn_vSetCurrFrame(HIE_tdstSuperObject_* p_stSuperObject, s16 _wFrame)
{
    gu_tdst3dData_* h_Current3dData;
    s32 savedFrameRate;
    s32 divisor;
    s32 frameCount;
    s32 selectedFrame;
    s32 randomValue;
    s32 previousFrame;
    s32 clock;
    f32 scale;
    f32 randomFloat;

    h_Current3dData = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    savedFrameRate = h_Current3dData->ucFrameRate;
    divisor = savedFrameRate;
    selectedFrame = (s32) _wFrame;
    frameCount = (s32) h_Current3dData->h_CurrentState->p_stAnim->uwNumberOfFrames;
    if (selectedFrame == -32768) {
        scale = (f32) (frameCount - 1) + 1.0f;
        randomValue = rand();
        scale = scale * RAND_SCALE_inv;
        randomFloat = (f32) randomValue;
        selectedFrame = (s16) (s32) (randomFloat * scale);
    } else {
        if (selectedFrame < 0) {
            selectedFrame = 0;
        } else if (selectedFrame >= frameCount) {
            selectedFrame = (s16) (frameCount - 1);
        }
    }
    previousFrame = selectedFrame - 1;
    h_Current3dData->sLastFrame = (u16) previousFrame;
    h_Current3dData->ucFlagModifState = 1;
    h_Current3dData->sForcedFrame = (u16) selectedFrame;
    h_Current3dData->h_WantedState = h_Current3dData->h_CurrentState;
    h_Current3dData->ucFlagEndOfAnim = 0;
    h_Current3dData->ucFlagEndState = 0;
    if (savedFrameRate == 0) {
        divisor = (u8) h_Current3dData->h_CurrentState->scSpeedAnim;
        fn_v3dDataSetFrameRate(h_Current3dData, divisor);
    }
    clock = inv_fn_ulGetEngineClock();
    clock = clock - (inv_GAM_ulTicksPerSecond * (u32) selectedFrame) / (u32) divisor;
    h_Current3dData->ulStartTime = clock;
    clock = inv_fn_ulGetEngineClock();
    clock = clock - inv_GAM_ulTicksPerSecond / (u32) divisor;
    h_Current3dData->ulTimePreviousFrame = clock;
    PLA_fn_bDoAnimPlayerForCharacter(p_stSuperObject, 0, 0);
    fn_v3dDataSetFrameRate(h_Current3dData, savedFrameRate);
}

u16 PLA_fn_uwGetNbFrameInCurrAnim(HIE_tdstSuperObject_* p_stSuperObject)
{
    tdstEngineObject_* p_stEngineObject = p_stSuperObject->hLinkedObject;

    return p_stEngineObject->h_3dData->h_CurrentState->p_stAnim->uwNumberOfFrames;
}

int PLA_fn_vInitNewState(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_State)
{
    gu_tdst3dData_* h_Current3dData;
    tdstAnim3d_* animationData;

    h_Current3dData = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    if (h_Current3dData->h_Family->hForStateArray.hFirstElementOpt == NULL) {
        return 0;
    }
    h_Current3dData->ucFlagEndState = 0;
    h_Current3dData->ucFlagModifState = 1;
    h_Current3dData->uwCurrentFrame = 0;
    h_Current3dData->sLastFrame = 0;
    h_Current3dData->h_WantedState = h_State;
    animationData = h_State->p_stAnim;
    if (animationData != 0) {
        fn_vAllocArrayOfElts3d(h_Current3dData, animationData->ucMaxNumberOfElements);
    } else {
        fn_vFreeArrayOfElts3d(h_Current3dData);
    }
    h_Current3dData->h_CurrentState = h_State;
    fn_vUpdateCurrentActivationForCharacter(p_stSuperObject);
    h_Current3dData->h_FirstStateOfAction = h_State;
    fn_v3dDataSetFrameRate(h_Current3dData, (u8) h_State->scSpeedAnim);
    if (h_State->p_stAnim != 0) {
        fn_vInitAllEvents(p_stSuperObject, 0);
    }
    return 1;
}

s32 PLA_fn_bTestNewState(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_WantedState)
{
    tdstEngineObject_* object = p_stSuperObject->hLinkedObject;
    gu_tdst3dData_* data = object->h_3dData;
    tdstState_* h_State = data->h_WantedState;

    if (h_WantedState == data->h_FirstStateOfAction)
        return 1;
    if (fn_hIfExistTransitionalWithTargetState(h_State, h_WantedState) == NULL) {
        if (h_State->ucTransitionStatusFlag == 1)
            return 0;
        if (fn_hIfExistProhibitedState(h_State, h_WantedState) != 0)
            return 0;
    }
    return 1;
}

s32 PLA_fn_bTestCurrentAction(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_FirstStateOfAction)
{
    return h_FirstStateOfAction ==
           ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData->h_FirstStateOfAction;
}

s32 PLA_fn_stGetLinearSpeedVector(HIE_tdstSuperObject_* self, s16 start_idx, MTH3D_tdstVector_* out)
{
    gu_tdst3dData_* h_Current3dData;
    tdstAnim3d_* p_stCurrentAnim;
    MTH3D_tdstVector_* pt;
    s32 i;
    s32 count;

    h_Current3dData = ((tdstEngineObject_*) self->hLinkedObject)->h_3dData;
    p_stCurrentAnim = h_Current3dData->h_CurrentState->p_stAnim;
    if (p_stCurrentAnim != NULL) {
        f32 denom;
        f32 xInvLength;
        f32 scale;
        pt = fn_p_stGetLinearSpeedVector(p_stCurrentAnim, h_Current3dData->uwCurrentFrame);
        count = 1;
        if (pt != NULL) {
            *out = *pt;
        } else {
            f32 zero = 0.0f;
            MTH3D_M_vSetXYZ_inv(out, zero, zero, zero);
        }

        for (i = (s32) start_idx + 1; i < (s32) h_Current3dData->uwCurrentFrame; i++) {
            pt = fn_p_stGetLinearSpeedVector(p_stCurrentAnim, (u16) i);
            if (pt != NULL) {
                MTH3D_M_vAddXYZ_inv(out, out, pt);
            }
            count++;
        }

        denom = (f32) count;
        xInvLength = 1.0f / denom;
        inv_MTH3D_fn_vKTimesXYZ(out, xInvLength, out);
        scale = (f32) h_Current3dData->ucFrameRate;
        inv_MTH3D_fn_vKTimesXYZ(out, scale, out);

        return 1;
    }

    return 0;
}

s32 PLA_fn_stGetAngularSpeedMatrix(HIE_tdstSuperObject_* obj, s16 frame, MTH3D_tdstMatrix_* out)
{
    MTH3D_tdstMatrix_ frameMat;
    MTH3D_tdstMatrix_ product;
    gu_tdst3dData_* h_Current3dData;
    tdstAnim3d_* p_stCurrentAnim;
    s32 frameIndex;
    s32 found;

    found = 0;
    h_Current3dData = ((tdstEngineObject_*) obj->hLinkedObject)->h_3dData;
    p_stCurrentAnim = h_Current3dData->h_CurrentState->p_stAnim;
    if (p_stCurrentAnim != NULL) {
        frameIndex = frame + 1;

        {
            volatile MTH3D_tdstMatrix_* vout = out;
            vout->stCol_0.xY = 0.0f;
            vout->stCol_2.xZ = 1.0f;
            vout->stCol_0.xX = 1.0f;
            vout->stCol_0.xZ = 0.0f;
            MTH3D_M_vSetXYZ_inv(&vout->stCol_1, 0.0f, 1.0f, 0.0f);
            vout->stCol_2.xX = 0.0f;
            vout->stCol_2.xY = 0.0f;
        }

        if ((s32) h_Current3dData->uwCurrentFrame >= frameIndex) {
            do {
                if (fn_p_stGetAngularSpeedMatrix(p_stCurrentAnim, (u16) frameIndex, &frameMat) != 0) {
                    found = 1;

                    MTH3D_M_vSetXYZ_inv(&product.stCol_0,
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xX, out->stCol_1.xX,
                                                         out->stCol_2.xX, frameMat.stCol_0.xX,
                                                         frameMat.stCol_0.xY, frameMat.stCol_0.xZ),
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xY, out->stCol_1.xY,
                                                         out->stCol_2.xY, frameMat.stCol_0.xX,
                                                         frameMat.stCol_0.xY, frameMat.stCol_0.xZ),
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xZ, out->stCol_1.xZ,
                                                         out->stCol_2.xZ, frameMat.stCol_0.xX,
                                                         frameMat.stCol_0.xY, frameMat.stCol_0.xZ));

                    MTH3D_M_vSetXYZ_inv(&product.stCol_1,
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xX, out->stCol_1.xX,
                                                         out->stCol_2.xX, frameMat.stCol_1.xX,
                                                         frameMat.stCol_1.xY, frameMat.stCol_1.xZ),
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xY, out->stCol_1.xY,
                                                         out->stCol_2.xY, frameMat.stCol_1.xX,
                                                         frameMat.stCol_1.xY, frameMat.stCol_1.xZ),
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xZ, out->stCol_1.xZ,
                                                         out->stCol_2.xZ, frameMat.stCol_1.xX,
                                                         frameMat.stCol_1.xY, frameMat.stCol_1.xZ));

                    MTH3D_M_vSetXYZ_inv(&product.stCol_2,
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xX, out->stCol_1.xX,
                                                         out->stCol_2.xX, frameMat.stCol_2.xX,
                                                         frameMat.stCol_2.xY, frameMat.stCol_2.xZ),
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xY, out->stCol_1.xY,
                                                         out->stCol_2.xY, frameMat.stCol_2.xX,
                                                         frameMat.stCol_2.xY, frameMat.stCol_2.xZ),
                                        inv_MTH_fn_xMulAdd3Grouped(out->stCol_0.xZ, out->stCol_1.xZ,
                                                         out->stCol_2.xZ, frameMat.stCol_2.xX,
                                                         frameMat.stCol_2.xY, frameMat.stCol_2.xZ));

                    *out = product;
                }
                frameIndex++;
            } while ((s32) h_Current3dData->uwCurrentFrame >= frameIndex);
        }
    }
    return found;
}

s32 PLA_fn_bTestIfEndOfAction(HIE_tdstSuperObject_* p_stSuperObject)
{
    gu_tdst3dData_* data = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    if (data->ucFlagEndState) {
        if (data->h_CurrentState == data->h_WantedState)
            return 1;
    }
    return 0;
}

s32 PLA_fn_bTestIfEndOfAnim(HIE_tdstSuperObject_* p_stSuperObject)
{
    gu_tdst3dData_* state = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    return (s32) (state->ucFlagEndOfAnim != 0);
}
