#include "common.h"

#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"

#include "include_asm.h"
#include "GAM/GAM.h"
#include "AI/AI.h"
#ifdef PC_PORT
#include "PC/PC.h"

static f32 inv_AGO_fn_xHudNarrow(void)
{
    return (f32) pc_hud_narrow() / 65536.0f;
}

static f32 inv_AGO_fn_xHudPlace(f32 x, f32 width, f32 narrow)
{
    if (x < width * 0.3f) {
        return x * narrow;
    }
    if (x <= width * 0.6f) {
        return x * narrow + (width - width * narrow) * 0.5f;
    }
    return x * narrow + (width - width * narrow);
}
#endif
#include "AGO/AGO.h"

typedef struct AGO_Icone_ {
    f32 Coords[4];
    s32 Material;
    f32 Transparency;
} AGO_Icone_;

typedef struct AGO_Gauge_ {
    f32 Coords[4];
    s32 EmptyMaterial;
    s32 FullMaterial;
    f32 Position;
    f32 Transparency;
} AGO_Gauge_;

extern AGO_Icone_ g_TheIcones[];

#define AGO_PART_COUNT 300
#define AGO_PART_DEAD 0x01000000

typedef struct AGO_Part_ AGO_Part_;
typedef void (*AGO_tdfnPartCallback)(AGO_Part_* p);

typedef union AGO_tduAnimTime_ {
    u32 ul;
    f32 x;
} AGO_tduAnimTime_;

typedef union AGO_tduLimitParam_ {
    f32 f_ZLimit;
    f32 fTimeLimit;
} AGO_tduLimitParam_;

struct AGO_Part_ {
    u32 ulType;
    u32 ulBehaviourFlags;
    AGO_tduAnimTime_ AnimTime;
    f32 fCreationTime;
    MTH3D_tdstVector_ stInitialPosition;
    MTH3D_tdstVector_ stPosition;
    MTH3D_tdstVector_ stDirection;
    MTH3D_tdstVector_ stAdditionnalVector;
    f32 fScale[2];
    f32 fAnimTimeScale;
    AGO_tduLimitParam_ LimitParam;
    void* VisualMaterial;
    void* AdditionnalMaterial;
    f32 Transparency;
    AGO_tdfnPartCallback pfnCallback;
};

typedef struct _JFFTXT_ {
    u8* pString;
    f32 a3_Pos[3];
    u8 ucTransparency;
    u8 bFrozenDisplay;
    u8 pulse;
    u8 bOther;
    s32 EffectParameter;
    u8 ucForcedColor;
    s32 cellWidth;
    s32 cellHeight;
    s32 advance;
    s32 lineHeight;
} _JFFTXT_;

extern AGO_Part_ g_AGO[];

extern MTH3D_tdstVector_ AGO_gExtraParameter1;

extern MTH3D_tdstVector_ AGO_gExtraParameter2;

extern MTH3D_tdstVector_ inv_AGO_stPrevExtraParamPosition;

extern f32 g_ffFrameTime;

extern f32 g_fTimeDifference;

extern f32 inv_CreateType67AGO_NextBubble;

extern f32 inv_AGO_stType28Direction2D[];

extern u32 g_ulNumberOfAgos;

extern f32 inv_AGO_fadeValue;

extern f32 g_ffLastFrameTime;

extern s32 g_NumberOfZeldaAgos;

extern s32 dword_252570[];

extern u32 AGO_g_OddFrame;

extern u32 AGO_gExtraMaterial;

extern u8 inv_AGO_fadeState;

extern u8 g_bUsingMapMondeAGOs;

extern u8 LastFrozen;

extern u8 byte_2538AA[];

extern _JFFTXT_ g_JFFTXT[];

extern struct GLD_tdstViewportAttributes_ unk_2524F8;

s32 inv_TXT_fn_lGetStringPixelWidth(u8*, s32);

s32 inv_toupper(s32);

void AGO_vDisplayPart(s32);

void AGO_vProcessPart(s32);

void inv_RW3_fn_vDrawScreenSprite(void*, f32, f32, f32, f32, void*);

void inv_TXT_fn_vComputeCharMetrics(s32*, s32*, s32*, s32*, f32);

void inv_AGO_vCompactDeadParts(void);

void GLI_vDisplayFrame(MTH3D_tdstVector_* topLeft, MTH3D_tdstVector_* bottomRight, f32 alpha, f32 luminance,
                                    struct GLD_tdstViewportAttributes_* viewport);
#ifdef PC_PORT
/* A veil with the HUD at the sides is narrowed and placed as a text starting at its left edge is, so that it stays
   under its text (the dialogues' box). A full screen veil, and one reaching the right edge (the driver stretches
   it), are drawn as they are. */
static void inv_AGO_fn_vDisplayHudFrame(MTH3D_tdstVector_* topLeft, MTH3D_tdstVector_* bottomRight, f32 alpha,
                                        f32 luminance, struct GLD_tdstViewportAttributes_* viewport)
{
    f32 narrow = inv_AGO_fn_xHudNarrow();
    MTH3D_tdstVector_ a = *topLeft;
    MTH3D_tdstVector_ b = *bottomRight;

    if (narrow != 1.0f && b.xX < 90.0f && !(a.xX <= 0.0f && b.xX >= 100.0f)) {
        f32 offset = (inv_AGO_fn_xHudPlace(a.xX * 10.0f, 1000.0f, narrow) - a.xX * 10.0f * narrow) * 0.1f;

        a.xX = a.xX * narrow + offset;
        b.xX = b.xX * narrow + offset;
    }
    GLI_vDisplayFrame(&a, &b, alpha, luminance, viewport);
}
#endif

void AGO_vAddParticleContd(u32 PartType, MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection, void* hVisualMaterial, f32 parameter);

extern s8 D_00393CF1;

extern s32 g_AGO_PloufMaterial;

extern s32 AGO_g_BigRainMaterial;

extern f32 g_fNextPlouf;

extern f32 inv_AGO_vAddParticle_NextBubble10;

extern f32 inv_AGO_vAddParticle_NextBubble26;

extern f32 inv_AGO_vAddParticle_NextBubble27;

extern u8 AGO_bPrevAGOWasExtraParam;

void inv_AGO_vResetGauges(void);

void inv_AGO_vResetIcons(void);

void TXT_vInit(void);

f32 cosf(f32);
f32 cosf(f32);

void AGO_Define_OrthogonalVectors(MTH3D_tdstVector_* p_stDirection, MTH3D_tdstVector_* p_stV1, MTH3D_tdstVector_* p_stV2);

extern u8 unk_380DC8[];

extern u8 unk_380E88[];

void TXT_vAffiche(void);

extern MTH3D_tdstVector_ inv_AGO_stType28Position;

f32 sinf(f32);
f32 sinf(f32);

extern f32 inv_CreateType21AGO_Offset;

extern MTH3D_tdstVector_ inv_AGO_stType28Direction;

extern s32 inv_CreateType28AGO_skipCount;

void AGO_vNormThisVector(MTH3D_tdstVector_* v);

extern f32 inv_AGO_xType428ZLimit;

extern u8 g_DoSmallPlouf;

extern s32 inv_AGO_vAddParticle_skipCount175;

void inv_strcpy3(u8* dst, const u8* src);

s32 atoi(void* buf);

void JFFTXT_vGetStringExtents(_JFFTXT_* record, void* work, void* result);

void JFFTXT_vDrawString(_JFFTXT_* rec);

extern u8 inv_GLI_ucTextGradientRed;

extern u8 inv_GLI_ucTextGradientGreen;

extern u8 inv_GLI_ucTextGradientBlue;

extern int strcmp(const char* a, const char* b);

static inline void inv_AGO_fn_vJitterPoint(MTH4D_tdstVector_* out,
                                           const MTH4D_tdstVector_* v1, f32 in_dx, f32 in_dy,
                                           f32 xJitterMax, f32 scale, f32 xJitterMin)
{
    f32 ox, vx, dx;
    f32 oy, vy, dy;
    f32 xRandom;
    f32 xTemp;

    xRandom = (f32) rand() * xJitterMax * scale + xJitterMin;

    ox = out->xX;
    vx = v1->xX;
    dx = in_dx;

    oy = out->xY;
    vy = v1->xY;
    dy = in_dy;

    xTemp = xRandom * vx;
    vx = xTemp;
    xTemp = xRandom * vy;
    vy = xTemp;

    dx = dx + vx;
    dy = dy + vy;
    ox = ox + dx;
    oy = oy + dy;

    out->xY = oy;
    out->xX = ox;
}

typedef struct MTH2D_tdstVector_ {
    f32 xX;
    f32 xY;
} MTH2D_tdstVector_;

void AGO_vInit(void)
{
    g_ulNumberOfAgos = 0;
    D_00393CF1 = 1;
    g_ffLastFrameTime = 0;
    g_NumberOfZeldaAgos = 0;
    g_AGO_PloufMaterial = 0;
    AGO_g_BigRainMaterial = 0;
    g_fNextPlouf = 0;
    AGO_bPrevAGOWasExtraParam = 0;
    inv_AGO_vResetGauges();
    inv_AGO_vResetIcons();
    TXT_vInit();
    inv_AGO_fadeState = 0;
    inv_AGO_fadeValue = 0.0f;
}

s32 inv_AGO_ulFindFreePart(void)
{
    AGO_Part_* p = g_AGO;
    u32 i = 0;
    u32 ulOldestIndex;
    u32 ulType;
    f32 fOldestTime;

    while (p->ulType != 0x60000000 && i < AGO_PART_COUNT) {
        if (p->ulType & AGO_PART_DEAD) {
            p->ulType = 0;
            return i;
        }
        p++;
        i++;
    }
    if (i >= AGO_PART_COUNT - 1) {
        return -1;
    }
    ulOldestIndex = i;
    fOldestTime = p->fCreationTime;
    for (i = ulOldestIndex + 1; i < AGO_PART_COUNT; i++, p++) {
        ulType = p->ulType;
        if (ulType & AGO_PART_DEAD) {
            p->ulType = 0;
            return i;
        }
        if (p->fCreationTime < fOldestTime && ulType != 0x60000000 && (ulType & 0xFF) != 0x13) {
            fOldestTime = p->fCreationTime;
            ulOldestIndex = i;
        }
    }
    return ulOldestIndex;
}

typedef struct SpawnFrameVector {
    MTH3D_tdstVector_ v;
    f32 w;
} SpawnFrameVector;

AGO_Part_* AGO_vCreatePart(u32 parm_ulType, MTH3D_tdstVector_* parm_a3_fPosition, MTH3D_tdstVector_* parm_a3_fDirection, f32 TimeLimit, f32 fScaleX, f32 fScaleY, void* pVisualMaterial)
{
    AGO_Part_* p;
    SpawnFrameVector V1;
    SpawnFrameVector V2;
    s32 slot;
    s32 type;
    s32 typeCopy;
    s32 i;
    s32 bits;
    s32 truncated;
    f32 wrap;
    f32 Angle;
    f32 Cos1;
    f32 Sin1;
    f32 sign;
    f32 negSign;

    if ((parm_ulType & 0x80000) && g_ulNumberOfAgos != AGO_PART_COUNT) {
        if (g_NumberOfZeldaAgos == 0x41) {
            return 0;
        }
        g_NumberOfZeldaAgos++;
    }
    if (g_ulNumberOfAgos == AGO_PART_COUNT) {
        slot = inv_AGO_ulFindFreePart();
        if (slot == -1) {
            return 0;
        }
        p = &g_AGO[slot];
    } else {
        p = &g_AGO[g_ulNumberOfAgos++];
    }

    p->ulType = parm_ulType;
    p->ulBehaviourFlags = 0;
    p->stPosition = *parm_a3_fPosition;
    p->stInitialPosition = *parm_a3_fPosition;
    if (parm_a3_fDirection != 0) {
        p->stDirection = *parm_a3_fDirection;
    }
    p->fCreationTime = g_ffFrameTime = inv_fn_xGetEngineClockTime();
    p->fCreationTime = inv_fn_xGetEngineClockTime();
    p->fScale[0] = fScaleX;
    p->fScale[1] = fScaleY;
    p->VisualMaterial = pVisualMaterial;
    truncated = inv_MTH_fn_lToInteger(gs_fCoefOfMultiplicationForAnimatedTexture);
    p->AnimTime.ul = truncated;
    p->fAnimTimeScale = g_AGO_AnimTimeScale;
    p->Transparency = 255.0f;
    p->LimitParam.fTimeLimit = TimeLimit;
    if (parm_ulType == 0x10003300) {
        if (parm_a3_fDirection != 0) {
            p->Transparency = parm_a3_fDirection->xX;
        }
    }
    if ((parm_ulType & 0xF000) == 0x7000) {
        p->Transparency = 5.0f;
    }
    type = parm_ulType & 0xFF;
    typeCopy = (s16) type;

    if (type == 6) {
        MTH3D_M_vSetXYZ_inv(&p->stInitialPosition, (-3.5f) + 7.0f * (f32) rand() / RAND_RANGE_inv,
                            (-3.5f) + 7.0f * (f32) rand() / RAND_RANGE_inv,
                            (-3.5f) + 7.0f * (f32) rand() / RAND_RANGE_inv);
    }
    if (type == 7 || type == 0xB) {
        if (parm_ulType & 0x80000) {
            p->stInitialPosition.xX = (-1.5f) + 3.0f * (f32) rand() / RAND_RANGE_inv;
            p->stInitialPosition.xZ = 1.0f + 3.0f * (f32) rand() / RAND_RANGE_inv;
        } else {
            p->stInitialPosition.xX = (-0.5f) + (f32) rand() / RAND_RANGE_inv;
            p->stInitialPosition.xZ = 1.0f + (f32) rand() / RAND_RANGE_inv;
        }
    }
    if (type == 8) {
        MTH3D_M_vSetXYZ_inv(&p->stInitialPosition, 0.0f + 0.005f * (f32) rand() / RAND_RANGE_inv,
                            (-0.05f) + 0.1f * (f32) rand() / RAND_RANGE_inv,
                            5.0f + (f32) rand() / RAND_RANGE_inv);
        p->Transparency = 200.0f;
    }
    if (typeCopy == 0x12) {
        f32* dst = (f32*) &p->stInitialPosition;
        f32* ref = (f32*) &p->stPosition;
        f32* src = (f32*) &p->stDirection;

        p->LimitParam.fTimeLimit = 0.0f;
        for (i = 0; i < 3; i++) {
            dst[i] = (src[i] - ref[i]) * 0.05f;
        }
    }
    if (type == 0xC) {
        s32 turns;

        p->stInitialPosition = *parm_a3_fPosition;
        AGO_Define_OrthogonalVectors(parm_a3_fDirection, &V1.v, &V2.v);
        Angle = 0.0f + 7.0f * (f32) rand() / RAND_RANGE_inv;
        p->fAnimTimeScale = Angle;
        p->AnimTime.x = 0.5f + 0.5f * (f32) rand() / RAND_RANGE_inv;
        wrap = Angle * MTH_INV_TWO_PI_inv;
        turns = inv_MTH_fn_lToInteger(wrap);
        wrap = Angle - ((f32) turns) * MTH_TWO_PI_inv;
        Cos1 = cosf(Angle);
        if (inv_MTH_fn_xAbsolute(wrap) < MTH_PI_inv) {
            sign = inv_MTH_fn_xSign(wrap);
        } else {
            bits = *(u32*) &wrap;
            if ((bits & 0x7FFFFFFF) == 0) {
                negSign = 0.0f;
            } else {
                bits = (bits & 0x80000000) | 0x3F800000;
                negSign = *(f32*) &bits;
            }
            sign = -negSign;
        }
        Sin1 = MTH_fn_xSqrtOpt2(1.0f - Cos1 * Cos1) * sign;
        Sin1 = Sin1 * TimeLimit;
        Cos1 = Cos1 * TimeLimit;
        MTH3D_M_vSetXYZ_inv(&p->stPosition, parm_a3_fPosition->xX + V1.v.xX * Sin1 + V2.v.xX * Cos1,
                            parm_a3_fPosition->xY + V1.v.xY * Sin1 + V2.v.xY * Cos1,
                            parm_a3_fPosition->xZ + V1.v.xZ * Sin1 + V2.v.xZ * Cos1);
    }

    return p;
}

void inv_AGO_vCompactDeadParts(void)
{
    u32 Limit = g_ulNumberOfAgos;
    u32 NumToSkip = 0;
    u32 Source;
    AGO_Part_* psrc = g_AGO;
    AGO_Part_* pdst = g_AGO;

    for (Source = 0; Source < Limit; psrc++, Source++) {
        if (psrc->ulType & AGO_PART_DEAD) {
            if (psrc->ulType & 0x80000) {
                g_NumberOfZeldaAgos--;
            }
            NumToSkip++;
            continue;
        } else if (NumToSkip != 0) {
            g_ulNumberOfAgos -= NumToSkip;
            if (Limit != Source) {
                memmove(pdst, psrc, (Limit - Source) * sizeof(AGO_Part_));
            }
            psrc = pdst;
            NumToSkip = 0;
        }
        pdst++;
    }
    g_ulNumberOfAgos -= NumToSkip;
}

void inv_AGO_vResetGauges(void)
{
    s32 remaining = 5;
    s32 base = (s32) unk_380DC8;
    u8* cursor = (u8*) (base + 0x18);
    do {
        *(s32*) (cursor + 4) = 0;
        remaining--;
        *(s32*) cursor = 0;
        cursor += 0x20;
    } while (remaining >= 0);
    remaining = 5;
    base = (s32) unk_380E88;
    cursor = (u8*) (base + 0x18);
    do {
        *(s32*) (cursor + 4) = 0;
        remaining--;
        *(s32*) cursor = 0;
        cursor += 0x20;
    } while (remaining >= 0);
}

void inv_AGO_vResetIcons(void)
{
    s32 remaining = 34;
    s32 base = (s32) g_TheIcones;
    u8* cursor = (u8*) (base + 0x344);
    do {
        *(s32*) cursor = 0;
        remaining--;
        cursor -= 0x18;
    } while (remaining >= 0);
}

void inv_AGO_vSetGauge(f32* a0, f32* a1, s32 a2, f32 x, s32 a3)
{
    s32 bank;
    s32 index;
    s32 offset;
    u8* base;
    AGO_Gauge_* dst;

    bank = byte_2538AA[0];
    index = (s32) x;
    if (bank != 0) {
        offset = index << 5;
        base = unk_380E88;
    } else {
        offset = index << 5;
        base = unk_380DC8;
    }
    offset = offset + (s32) base;
    dst = (AGO_Gauge_*) offset;
    do {
        dst->Coords[0] = a0[0];
        dst->Coords[1] = a0[1];
        dst->Coords[2] = a1[0];
        {
            f32 by = a1[1];
            dst->EmptyMaterial = a2;
            dst->Coords[3] = by;
        }
        dst->FullMaterial = a3;
        dst->Position = a0[2];
        dst->Transparency = a1[2];
    } while (0);
}

void inv_AGO_vSetIcon(MTH3D_tdstVector_* source0, MTH3D_tdstVector_* source1, f32 indexValue, s32 value)
{
    s32 index;

    index = (s32) indexValue;
    g_TheIcones[index].Coords[0] = source0->xX;
    g_TheIcones[index].Coords[1] = source0->xY;
    g_TheIcones[index].Coords[2] = source1->xX;
    g_TheIcones[index].Coords[3] = source1->xY;
    g_TheIcones[index].Material = value;
    g_TheIcones[index].Transparency = source1->xZ;
}

void inv_AGO_vDisplayIcons(void)
{
    s32 i;
    s32 end;

    if (byte_2538AA[0] != 0) {
        i = 15;
        end = 35;
    } else {
        i = 0;
        end = 25;
    }

    while (i < end) {
        if (0.0f < g_TheIcones[i].Transparency) {
            GLI_vSetGlobalAlpha(g_TheIcones[i].Transparency);
#ifdef PC_PORT
            {
                f32 narrow = inv_AGO_fn_xHudNarrow();
                f32 left = g_TheIcones[i].Coords[0];
                f32 right = g_TheIcones[i].Coords[2];
                f32 width = (right - left) * narrow;

                if (narrow != 1.0f && right != 100.0f) {
                    if (left < 30.0f) {
                        left = left * narrow;
                    } else if (left < 70.0f) {
                        left = left + ((right - left) - width) * 0.5f;
                    } else {
                        left = left * narrow + (100.0f - 100.0f * narrow);
                    }
                    right = left + width;
                }
                inv_RW3_fn_vDrawScreenSprite(dword_252570, left, g_TheIcones[i].Coords[1], right, g_TheIcones[i].Coords[3],
                              (void*) g_TheIcones[i].Material);
            }
#else
            inv_RW3_fn_vDrawScreenSprite(dword_252570, g_TheIcones[i].Coords[0], g_TheIcones[i].Coords[1],
                          g_TheIcones[i].Coords[2], g_TheIcones[i].Coords[3],
                          (void*) g_TheIcones[i].Material);
#endif
        }
        i++;
    }
}

void inv_AGO_vDisplayGaugesAndIcons(void)
{
    AGO_Gauge_* p;
    s32 i;
    f32 xTransparency;
    f32 diffx;
    f32 diffy;
    f32 position;

    if ((*(u8(*)[]) & byte_2538AA)[0] != 0) {
        p = (*(AGO_Gauge_(*)[]) & unk_380E88);
    } else {
        p = (*(AGO_Gauge_(*)[]) & unk_380DC8);
    }
    for (i = 0; i < 6; i++, p++) {
        xTransparency = p->Transparency;
        if (xTransparency > 0.0f) {
#ifdef PC_PORT
            AGO_Gauge_ drawn = *p;
            AGO_Gauge_* kept = p;

            drawn.Coords[0] = p->Coords[0] * inv_AGO_fn_xHudNarrow();
            drawn.Coords[2] = p->Coords[2] * inv_AGO_fn_xHudNarrow();
            p = &drawn;
#endif
            diffx = p->Coords[2] - p->Coords[0];
            diffy = p->Coords[3] - p->Coords[1];
            if (diffy < diffx) {
                position = p->Coords[0] + p->Position * diffx * 0.01f;
                GLI_vSetGlobalAlpha(xTransparency);
                inv_RW3_fn_vDrawScreenSprite(
                    dword_252570, p->Coords[0], p->Coords[1], position, p->Coords[3], (void*) p->FullMaterial);
                if (p->Position < 100.0f) {
                    inv_RW3_fn_vDrawScreenSprite(
                        dword_252570, position, p->Coords[1], p->Coords[2], p->Coords[3], (void*) p->EmptyMaterial);
                }
            } else {
                position = p->Coords[3] - p->Position * diffy * 0.01f;
                GLI_vSetGlobalAlpha(xTransparency);
                inv_RW3_fn_vDrawScreenSprite(
                    dword_252570, p->Coords[0], position, p->Coords[2], p->Coords[3], (void*) p->FullMaterial);
                if (p->Position < 100.0f) {
                    inv_RW3_fn_vDrawScreenSprite(
                        dword_252570, p->Coords[0], p->Coords[1], p->Coords[2], position, (void*) p->EmptyMaterial);
                }
            }
#ifdef PC_PORT
            p = kept;
#endif
        }
    }
    inv_AGO_vDisplayIcons();
    TXT_vAffiche();
}

void AGO_vNormThis2DVector(MTH2D_tdstVector_* v);

void AGO_vProcessAllParts(void)
{
    u32 partIndex;
    f32 now;
    f32 zero;

    if (inv_AGO_fadeState == 3) {
        inv_AGO_fadeValue += 0.01f;
        zero = 0.0f;
        if (zero <= inv_AGO_fadeValue) {
            inv_AGO_fadeValue = zero;
            inv_AGO_fadeState = 0;
        }
    }

    if (LastFrozen != 0 && g_bInGameMenu == 0 && g_bUsingMapMondeAGOs == 0) {
        LastFrozen = 0;
        g_ulNumberOfAgos = 0;
        g_NumberOfZeldaAgos = 0;
    } else if (LastFrozen == 0 && g_bInGameMenu != 0 && g_bUsingMapMondeAGOs == 0) {
        LastFrozen = 1;
        for (partIndex = 0; partIndex < g_ulNumberOfAgos; partIndex++) {
            g_AGO[partIndex].ulType |= 0x40;
        }
    }

    now = inv_fn_xGetEngineClockTime();
    g_ffFrameTime = now;

    if (g_ffLastFrameTime == 0.0f) {
        g_fTimeDifference = 0.0f;
        g_ffLastFrameTime = now;
        return;
    }

    g_fTimeDifference = now - g_ffLastFrameTime;
    g_ffLastFrameTime = now;
    if (g_ulNumberOfAgos == 0) {
        return;
    }

    AGO_g_OddFrame = (AGO_g_OddFrame == 0);

    for (partIndex = 0; partIndex < g_ulNumberOfAgos;) {
        AGO_Part_* part;
        u32 flags;
        u32 flags2;
        s32 trigger;

        part = &g_AGO[partIndex];
        trigger = 0;
        flags = part->ulType;

        if ((flags & 0x40) == 0) {
            if ((flags & 0x10000) != 0) {
                if (part->LimitParam.fTimeLimit < g_ffFrameTime - part->fCreationTime) {
                    trigger = 1;
                }
            }

            if ((flags & 0x40000) != 0) {
                if (part->stPosition.xZ > part->LimitParam.f_ZLimit) {
                    trigger = 1;
                }
            }

            flags2 = part->ulBehaviourFlags;
            if ((flags2 & 4) != 0) {
                if (part->stPosition.xZ <= part->LimitParam.f_ZLimit) {
                    trigger = 1;
                }
            }

            if ((flags2 & 0x10) != 0) {
                MTH3D_tdstVector_ delta;
                f32 dot;
                f32* src;
                f32* ref;
                f32* dst;
                f32* view;
                f32* base;
                s32 component;

                view = inv_AGO_stType28Direction2D;
                base = (*(f32(*)[]) & inv_AGO_stType28Position);
                do {
                    partIndex++;
                } while (0);
                ref = base;
                dst = (f32*) &delta;
                src = (f32*) &part->stPosition;
                component = 2;
                do {
                    *dst++ = *src++ - *ref++;
                } while (--component >= 0);
                AGO_vNormThis2DVector(&delta);
                dot = view[0] * delta.xX + view[1] * delta.xY;
                if (dot < 0.3f) {
                    part->pfnCallback(part);
                }
            } else {
                partIndex++;
            }

            if (part->Transparency <= 0.0f) {
                trigger = 1;
            }

            if (trigger == 0) {
                if ((part->ulType & 0x1000000) == 0) {
                    goto loop_check;
                }
            }

            if ((part->ulType & 0x20000) != 0) {
                if ((part->ulType & 0x80000) != 0) {
                    f32 randomValue;
                    randomValue = (f32) rand();
                    randomValue = randomValue * 7.0f;
                    randomValue = randomValue * RAND_SCALE_inv;
                    randomValue = randomValue + -6.0f;
                    if (0.0f < randomValue) {
                        AGO_vAddParticle(0x44, &part->stPosition, &part->stDirection,
                                         part->VisualMaterial, 1.0f);
                    }
                }
            }
            part->ulType |= 0x1000000;
        } else {
            partIndex++;
        }
    loop_check:;
    }

    inv_AGO_vCompactDeadParts();

    for (partIndex = 0; partIndex < g_ulNumberOfAgos; partIndex++) {
        AGO_vProcessPart(partIndex);
    }
}

#ifdef PC_PORT
extern s32 inv_GAM_bDrawingInBetween;

#endif
void AGO_vDisplayAllParts(void)
{
    f32 savedAlpha;
    s32 partIndex;

#ifdef PC_PORT
    if (!inv_GAM_bDrawingInBetween) {
        AGO_g_OddFrame = AGO_g_OddFrame < 1;
    }
#else
    AGO_g_OddFrame = AGO_g_OddFrame < 1;
#endif
    savedAlpha = GLI_vGetGlobalAlpha();
    inv_GLI_vSaveRenderFlags();
    for (partIndex = 0; partIndex < ((s32) g_ulNumberOfAgos); partIndex++) {
        AGO_vDisplayPart(partIndex);
    }
    inv_GLI_vRestoreRenderFlags();
    GLI_vSetGlobalAlpha(savedAlpha);
}

static inline f32 inv_AGO_fn_xPhaseSignDefault(f32 phase, f32 epsilon, f32 zero)
{
    u32 bits;
    f32 sign;

    if (inv_MTH_fn_xAbsolute(phase) < epsilon) {
        sign = inv_MTH_fn_xSign(phase);
    } else {
        f32 xUnitSign;
        xUnitSign = inv_MTH_fn_xSign(phase);
        sign = -xUnitSign;
    }
    return sign;
}

static __inline__ f32 inv_AGO_fn_xPhaseSign(f32 phase, f32 epsilon)
{
    u32 bits;
    f32 sign;
    if (inv_MTH_fn_xAbsolute(phase) < epsilon) {
        sign = inv_MTH_fn_xSign(phase);
    } else {
        sign = -inv_MTH_fn_xSign(phase);
    }
    return sign;
}

static __inline__ f32 inv_AGO_fn_xSinCos(f32 x, f32* pc)
{
    s32 ti;
    f32 phase, c, sg;
    ti = inv_MTH_fn_lToInteger(x * MTH_INV_TWO_PI_inv);
    phase = x - (f32) ti * MTH_TWO_PI_inv;
    c = cosf(x);
    *pc = c;
    sg = inv_AGO_fn_xPhaseSign(phase, MTH_PI_inv);
    return MTH_fn_xSqrtOpt2(1.0f - c * c) * sg;
}

static __inline__ f32 inv_AGO_fn_xTruncPhase(f32 x)
{
    s32 ti;
    ti = inv_MTH_fn_lToInteger(x * MTH_INV_TWO_PI_inv);
    return x - (f32) ti * MTH_TWO_PI_inv;
}

static __inline__ f32 inv_AGO_fn_xSinFromCos(f32 c, f32 phase)
{
    f32 sg;
    sg = inv_AGO_fn_xPhaseSign(phase, MTH_PI_inv);
    return MTH_fn_xSqrtOpt2(1.0f - c * c) * sg;
}

void AGO_vDisplayPart(s32 ulPartNumber)
{
    AGO_Part_* p;
    u32 flags;
    u32 type;
    u32 mode;
    f32 offA;
    f32 offB;
    f32 offC;
    s32 iturn;

    p = &g_AGO[ulPartNumber];
    offA = 0.0f;
    offB = offA;
    offC = offA;

    if (p->ulBehaviourFlags & 1) {
        g_AGO_AnimTimeScale = p->fAnimTimeScale;
        if (p->Transparency >= 255.0f) {
            GLI_vSetGlobalAlpha(255.0f);
        } else {
            GLI_vSetGlobalAlpha(p->Transparency);
        }
        if (GLI_vDrawWorldSpriteOpt(&p->stPosition, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, p->AnimTime.ul)) {
            p->ulType |= 0x01000000;
        }
        return;
    }

    switch (p->ulType & 0xFF) {
    case 0:
        break;
    case 7:
    case 11: {
        MTH3D_tdstVector_* pos;
        f32 angle;
        f32 wrapped;
        f32 s;
        f32 sign;
        f32 scale;
        pos = &p->stPosition;
        angle = p->stInitialPosition.xZ * (g_ffFrameTime - p->fCreationTime);
        iturn = inv_MTH_fn_lToInteger(angle * MTH_INV_TWO_PI_inv);
        wrapped = angle - (f32) iturn * MTH_TWO_PI_inv;
        offA = cosf(angle);
        sign = inv_AGO_fn_xPhaseSignDefault(wrapped, MTH_PI_inv, 0.0f);
        s = MTH_fn_xSqrtOpt2(1.0f - offA * offA);
        scale = p->stInitialPosition.xX;
        offA *= scale;
        offB = s * sign;
        offB *= scale;
        pos->xX += offB;
        pos->xY += offA;
        pos->xZ += offB + offA;
    } break;
    case 8: {
        MTH3D_tdstVector_* v;
        MTH3D_tdstVector_* pos;
        f32 angle;
        f32 wrapped;
        f32 c;
        f32 s;
        f32 sign;
        f32 vx;
        f32 vy;
        v = &p->stInitialPosition;
        pos = &p->stPosition;
        angle = p->stInitialPosition.xZ * (g_ffFrameTime - p->fCreationTime) + p->stDirection.xZ;
        iturn = inv_MTH_fn_lToInteger(angle * MTH_INV_TWO_PI_inv);
        wrapped = angle - (f32) iturn * MTH_TWO_PI_inv;
        c = cosf(angle);
        sign = inv_AGO_fn_xPhaseSignDefault(wrapped, MTH_PI_inv, 0.0f);
        s = MTH_fn_xSqrtOpt2(1.0f - c * c);
        s *= sign;
        offB = v->xX * s;
        offA = v->xX * c;
        offB += v->xY * c;
        offC = v->xY * s;
        pos->xX += offB;
        pos->xY += offA;
        pos->xZ += offC;
    } break;
    }
    if ((p->ulType & 0xFF) == 12) {
        g_AGO_AnimTimeScale = 1.0f;
    } else {
        g_AGO_AnimTimeScale = p->fAnimTimeScale;
    }
    if (p->Transparency >= 255.0f) {
        GLI_vSetGlobalAlpha(255.0f);
    } else {
        GLI_vSetGlobalAlpha(p->Transparency);
    }

    if (p->VisualMaterial != 0) {
        if ((p->ulType & 0xFF) == 0x11) {
            if (fn_bIsEngineInPause() == 0) {
#ifdef PC_PORT
                inv_AGO_fn_vDisplayHudFrame(&p->stPosition, &p->stDirection, p->Transparency, 0.0f, &unk_2524F8);
#else
                GLI_vDisplayFrame(&p->stPosition, &p->stDirection, p->Transparency, 0.0f, &unk_2524F8);
#endif
            }
            p->ulType |= 0x01000000;
        } else if ((p->ulType & 0xFF) == 0x15) {
#ifdef PC_PORT
            inv_AGO_fn_vDisplayHudFrame(&p->stPosition, &p->stDirection, p->Transparency, 255.0f, &unk_2524F8);
#else
            GLI_vDisplayFrame(&p->stPosition, &p->stDirection, p->Transparency, 255.0f, &unk_2524F8);
#endif
            p->ulType |= 0x01000000;
        } else {
            if ((p->ulType & 0xF0000000) == 0x30000000) {
                MTH3D_tdstVector_ b;
                MTH3D_tdstVector_ a;
                f32 len2;
                f32 len;
                f32 inv;
                f32 dx;
                f32 dy;
                f32 threshold;

                threshold = 0.01f;
                dx = -p->stDirection.xX;
                dy = -p->stDirection.xY;
                MTH3D_M_vSetXYZ_inv(&a, dx, dy, 0.0f);
                len2 = dx * dx + dy * dy;
                len = MTH_fn_xSqrtOpt2(len2);
                if (threshold <= len) {
                    inv = 1.0f / len;
                    MTH3D_M_vSetXYZ_inv(&b, dy * inv, p->stDirection.xX * inv, 0.0f);
                    a.xX = dx * inv;
                    a.xY = dy * inv;
                    if (GLI_vDrawWorldOrthoPlanarSprite(&p->stPosition, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, p->AnimTime.ul, &b, &a)) {
                        p->ulType |= 0x01000000;
                    }
                } else {
                    p->ulType |= 0x01000000;
                }
            } else if ((p->ulType & 0xF0000000) == 0x80000000) {
                MTH3D_tdstVector_ d;
                MTH3D_tdstVector_ side;
                MTH3D_tdstVector_ up;
                MTH3D_tdstVector_* pos;
                f32 now;
                f32 phase;
                f32 wave;
                f32 u;
                f32 len2;
                f32 inv;
                f32 dt;
                f32 sn;

                u = (f32) p->AnimTime.ul;
                pos = &p->stPosition;
                now = g_ffFrameTime;
                sn = now - p->fCreationTime;
                wave = ((sn * 1.2134f + p->stPosition.xX) - pos->xY) + pos->xZ * 0.01f;
                dt = g_fTimeDifference;
                p->Transparency -= dt * 70.0f;
                if (p->Transparency <= 0.0f) {
                    p->Transparency = 0.0f;
                    p->ulType |= 0x01000000;
                }
                now = sn;
                phase = now + wave;
                d.xX = u * sinf(phase);
                d.xY = u * cosf(now - wave);
                sn = sinf(now);
                d.xZ = __builtin_fabsf(2.0f - cosf(sn + u * wave));
                p->stPosition.xX += dt * (u * sinf(wave + now * 3.2f) * 2.6f);
                pos->xY += dt * (u * cosf(wave + now * 3.243f + 0.324121f) * 2.5f);
                pos->xZ += dt * (cosf(u * phase) * 4.5f - 3.5f);
                len2 = d.xX * d.xX + d.xY * d.xY + d.xZ * d.xZ;
                if (len2 != 0.0f) {
                    inv = inv_MTH_fn_xDivideBySqrt(1.0f, len2);
                    MTH3D_M_vScaleXYZ_inv(&d, &d, inv);
                    AGO_Define_OrthogonalVectors(&d, &side, &up);
                    GLI_vDrawWorldOrthoPlanarSprite(pos, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, 1, &side, &up);
                }
            } else if ((p->ulType & 0xFF) == 19) {
                MTH3D_tdstVector_ prev;
                MTH3D_tdstVector_ prevLow;
                MTH3D_tdstVector_ cur;
                MTH3D_tdstVector_ curLow;
                MTH3D_tdstVector_ emitPos;
                MTH3D_tdstVector_ emitDir;
                f32 step;
                f32 dz;
                f32 zoff;
                f32 angle;
                f32 z;
                f32 alpha;
                f32 radius;
                f32 alphaStep;
                f32 c;
                f32 sn;
                f32 oldAlpha;
                f32 phase;
                s32 count;

                step = 0.31415924f;
                z = p->stPosition.xZ;
                angle = 0.0f;
                dz = p->stDirection.xZ / 20.0f;
                zoff = 0.3f;
                while (angle + step < p->stDirection.xX) {
                    angle += step;
                    z += dz;
                }
                alpha = p->Transparency;
                radius = p->fScale[0];
                alphaStep = alpha / 40.0f;
                phase = inv_AGO_fn_xTruncPhase(angle);
                c = cosf(angle);
                sn = inv_AGO_fn_xSinFromCos(c, phase);
                MTH3D_M_vSetXYZ_inv(&cur, p->stPosition.xX + radius * sn,
                                    p->stPosition.xY + radius * c, z);
                MTH3D_M_vSetXYZ_inv(&emitPos, cur.xX, cur.xY, z - zoff * 0.5f);
                angle -= step;
                count = 40;
                while (angle >= 0.0f && count != 0) {
                    z -= dz;
                    MTH3D_M_vCopyXYZ_inv(&prev, &cur);
                    phase = inv_AGO_fn_xTruncPhase(angle);
                    c = cosf(angle);
                    {
                        u32 bits;
                        if (inv_MTH_fn_xAbsolute(phase) < MTH_PI_inv) {
                            sn = inv_MTH_fn_xSign(phase);
                        } else {
                            sn = -inv_MTH_fn_xSign(phase);
                        }
                        {
                            f32 t;
                            t = MTH_fn_xSqrtOpt2(1.0f - c * c) * sn;
                            sn = t;
                        }
                    }
                    MTH3D_M_vSetXYZ_inv(&prevLow, prev.xX, prev.xY, prev.xZ - zoff);
                    curLow.xZ = z - zoff;
                    cur.xY = p->stPosition.xY + radius * c;
                    curLow.xY = cur.xY;
                    cur.xZ = z;
                    cur.xX = p->stPosition.xX + radius * sn;
                    curLow.xX = cur.xX;
                    angle -= step;
                    GLI_vSetGlobalAlpha(alpha);
                    GLI_vDrawWorldQuadOpt(&cur, &prev, &prevLow, &curLow, p->VisualMaterial);
                    alpha -= alphaStep;
                    count--;
                }
                GLI_vSetGlobalAlpha((f32) rand() * (p->Transparency - 0.0f) * RAND_SCALE_inv +
                               0.0f);
                GLI_vDrawWorldSpriteOpt(&emitPos, p->AdditionnalMaterial, p->fScale[1], p->fScale[1], &unk_2524F8, 1);
                MTH3D_M_vSetXYZ_inv(&emitDir, (emitPos.xX - p->stPosition.xX) * 3.0f,
                                    (emitPos.xY - p->stPosition.xY) * 3.0f, 2.0f);
                AGO_vCreatePart(0x4001, &emitPos, &emitDir, 0.0f, 0.015f, 0.015f, p->AdditionnalMaterial);
                p->stDirection.xX += p->stDirection.xY * g_fTimeDifference;
                p->stDirection.xY += g_fTimeDifference * 3.0f;
                p->fScale[0] += g_fTimeDifference * 0.5f;
            } else if ((p->ulType & 0xFF) == 20) {
                GLI_vDrawWorldQuadOpt(&p->stInitialPosition, &p->stPosition, &p->stDirection, &p->stAdditionnalVector, p->VisualMaterial);
            } else if ((p->ulType & 0xF0000000) == 0x70000000) {
                f32 start[3];
                f32 cross[3];
                f32 qa[3];
                f32 qb[3];
                f32 qc[3];
                f32 qd[3];
                f32 qh[3];
                f32 qe[3];
                f32 qf[3];
                f32 qg[3];
                f32* vec;
                f32* pos;
                f32* st;
                f32* dir;
                f32 r;
                f32 width;
                f32 sideScale;
                f32 len2;
                f32 dx, dy, dz;
                f32 vx, vy, vz;
                s32 i;

                width = 0.75f;
                r = (f32) rand() * RAND_SCALE_inv + -0.5f;
                dx = p->stDirection.xX;
                vz = p->stInitialPosition.xZ;
                dy = p->stDirection.xY;
                dz = p->stDirection.xZ;
                start[0] = (p->stPosition.xX - dx) + dx * r;
                vy = p->stInitialPosition.xY;
                cross[0] = dy * vz - dz * vy;
                start[1] = (p->stPosition.xY - dy) + dy * r;
                start[2] = (p->stPosition.xZ - dz) + dz * r;
                vx = p->stInitialPosition.xX;
                cross[1] = dz * vx - dx * vz;
                cross[2] = dx * vy - dy * vx;
                len2 = cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2];
                if (1e-05f < len2) {
                    pos = &p->stPosition.xX;
                    vec = &p->stInitialPosition.xX;
                    dir = &p->stDirection.xX;
                    st = start;
                    len2 = inv_MTH_fn_xDivideBySqrt(1.0f, len2);
                    for (i = 0; i < 3; i++) {
                        cross[i] *= len2;
                    }
                    width *= 0.2f;
                    sideScale = 1.4f;
                    for (i = 0; i < 3; i++) {
                        qa[i] = pos[i] + dir[i] * r + sideScale * cross[i] - width * vec[i];
                        qb[i] = pos[i] + dir[i] * r + sideScale * cross[i] + width * vec[i];
                        qc[i] = st[i] + sideScale * cross[i] + width * vec[i];
                        qd[i] = st[i] + sideScale * cross[i] - width * vec[i];
                    }
                    GLI_vDrawWorldQuadOpt((MTH3D_tdstVector_*) qb, (MTH3D_tdstVector_*) qc, (MTH3D_tdstVector_*) qd, (MTH3D_tdstVector_*) qa, p->VisualMaterial);
                    for (i = 0; i < 3; i++) {
                        qe[i] = qb[i] - (qc[i] - qb[i]) * 0.5f;
                        qf[i] = qc[i] - (qc[i] - qb[i]) * 0.5f;
                        qg[i] = qd[i] - (qd[i] - qa[i]) * 0.5f;
                        qh[i] = qa[i] - (qd[i] - qa[i]) * 0.5f;
                    }
                    GLI_vDrawWorldQuadOpt((MTH3D_tdstVector_*) qe, (MTH3D_tdstVector_*) qf, (MTH3D_tdstVector_*) qg, (MTH3D_tdstVector_*) qh, p->VisualMaterial);
                    for (i = 0; i < 3; i++) {
                        qa[i] = pos[i] + dir[i] * r - sideScale * cross[i] - width * vec[i];
                        qb[i] = pos[i] + dir[i] * r - sideScale * cross[i] + width * vec[i];
                        qc[i] = st[i] - sideScale * cross[i] + width * vec[i];
                        qd[i] = st[i] - sideScale * cross[i] - width * vec[i];
                    }
                    GLI_vDrawWorldQuadOpt((MTH3D_tdstVector_*) qb, (MTH3D_tdstVector_*) qc, (MTH3D_tdstVector_*) qd, (MTH3D_tdstVector_*) qa, p->VisualMaterial);
                    for (i = 0; i < 3; i++) {
                        qe[i] = qb[i] - (qc[i] - qb[i]) * 0.5f;
                        qf[i] = qc[i] - (qc[i] - qb[i]) * 0.5f;
                        qg[i] = qd[i] - (qd[i] - qa[i]) * 0.5f;
                        qh[i] = qa[i] - (qd[i] - qa[i]) * 0.5f;
                    }
                    GLI_vDrawWorldQuadOpt((MTH3D_tdstVector_*) qe, (MTH3D_tdstVector_*) qf, (MTH3D_tdstVector_*) qg, (MTH3D_tdstVector_*) qh, p->VisualMaterial);
                }
            } else if ((p->ulType & 0xF0000000) == 0x40000000 ||
                       (p->ulType & 0xF0000000) == 0x50000000 ||
                       (p->ulType & 0xF0000000) == 0x60000000) {
                MTH3D_tdstVector_ delta;
                MTH3D_tdstVector_ perp;
                MTH3D_tdstVector_ cross;
                MTH3D_tdstVector_ center;
                f32 xx2;
                f32 zz2;
                f32 sq0;
                f32 sq1;
                f32 sq2;
                f32 radius;
                f32 len2;
                f32 len;
                f32 inv;

                delta.xX = p->stPosition.xX - p->stInitialPosition.xX;
                delta.xY = p->stPosition.xY - p->stInitialPosition.xY;
                if ((p->ulType & 0xF0000000) == 0x60000000) {
                    delta.xZ = 0.0f;
                } else {
                    delta.xZ = p->stPosition.xZ - p->stInitialPosition.xZ;
                }
                MTH3D_M_vSetXYZ_inv(&center, p->stInitialPosition.xX + delta.xX * 0.5f,
                                    p->stInitialPosition.xY + delta.xY * 0.5f,
                                    p->stInitialPosition.xZ + delta.xZ * 0.5f);
                perp.xX = -delta.xY;
                perp.xZ = 0.0f;
                if ((p->ulType & 0xF0000000) == 0x60000000) {
                    perp.xY = delta.xX;
                    xx2 = delta.xX * delta.xX + delta.xY * delta.xY;
                    zz2 = xx2 + delta.xZ * delta.xZ;
                } else {
                    perp.xY = delta.xZ;
                    xx2 = delta.xY * delta.xY + delta.xZ * delta.xZ;
                    zz2 = xx2 + delta.xX * delta.xX;
                }
                if (0.05f <= xx2) {
                    radius = MTH_fn_xSqrtOpt2(zz2);
                    zz2 = inv_MTH_fn_xDivideBySqrt(1.0f, zz2);
                    xx2 = inv_MTH_fn_xDivideBySqrt(1.0f, xx2);
                    p->fScale[0] = radius * 0.5f;
                    perp.xX *= xx2;
                    perp.xY *= xx2;
                    MTH3D_M_vScaleXYZ_inv(&delta, &delta, zz2);
                    if ((p->ulType & 0xF0000000) == 0x60000000) {
                        GLI_vDrawWorldOrthoPlanarSprite(&center, p->VisualMaterial, 0.4f, p->fScale[0], &unk_2524F8, p->AnimTime.ul, &perp, &delta);
                    } else if ((p->ulType & 0xF0000000) == 0x50000000) {
                        GLI_vDrawWorldOrthoPlanarSprite(&center, p->VisualMaterial, 0.3f, p->fScale[0], &unk_2524F8, p->AnimTime.ul, &perp, &delta);
                        {
                            f32 cx;
                            f32 len2, inv;
                            DNM_p_stVectorCrossProduct(&cross, &delta, &perp);
                            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(cross.xX, cross.xY, cross.xZ);
                            len = MTH_fn_xSqrtOpt2(len2);
                            if (len != 0.0f) {
                                inv = 1.0f / len;
                                inv_MTH3D_fn_vKTimesXYZ(&cross, inv, &cross);
                            }
                        }
                        GLI_vDrawWorldOrthoPlanarSprite(&center, p->VisualMaterial, 0.3f, p->fScale[0], &unk_2524F8, p->AnimTime.ul, &cross, &delta);
                    } else if (AGO_g_OddFrame != 0) {
                        GLI_vDrawWorldOrthoPlanarSprite(&center, p->VisualMaterial, 0.1f, p->fScale[0], &unk_2524F8, p->AnimTime.ul, &perp, &delta);
                    } else {
                        {
                            f32 cx;
                            f32 len2, inv;
                            DNM_p_stVectorCrossProduct(&cross, &delta, &perp);
                            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(cross.xX, cross.xY, cross.xZ);
                            len = MTH_fn_xSqrtOpt2(len2);
                            if (len != 0.0f) {
                                inv = 1.0f / len;
                                inv_MTH3D_fn_vKTimesXYZ(&cross, inv, &cross);
                            }
                        }
                        GLI_vDrawWorldOrthoPlanarSprite(&center, p->VisualMaterial, 0.1f, p->fScale[0], &unk_2524F8, p->AnimTime.ul, &cross, &delta);
                    }
                }
            } else if ((p->ulType & 0xF0000000) == 0x20000000) {
                MTH3D_tdstVector_ a;
                MTH3D_tdstVector_ b;
                AGO_Define_OrthogonalVectors(&p->stDirection, &a, &b);
                if (GLI_vDrawWorldOrthoPlanarSprite(&p->stPosition, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, p->AnimTime.ul, &a, &b)) {
                    p->ulType |= 0x01000000;
                }
            } else if ((p->ulType & 0xF0000000) == 0x90000000) {
                MTH3D_tdstVector_ up;
                MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
                if (GLI_vDrawWorldOrthoPlanarSprite(&p->stPosition, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, p->AnimTime.ul, &p->stAdditionnalVector, &up)) {
                    p->ulType |= 0x01000000;
                }
            } else if ((p->ulType & 0xF0000000) == 0x10000000) {
                if (GLI_vDrawWorldPlanarSprite(&p->stPosition, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, p->AnimTime.ul)) {
                    p->ulType |= 0x01000000;
                }
            } else {
                if (p->ulType & 0x08000000) {
                    p->stPosition.xZ += p->fScale[1];
                }
                if (GLI_vDrawWorldSpriteOpt(&p->stPosition, p->VisualMaterial, p->fScale[0], p->fScale[1], &unk_2524F8, p->AnimTime.ul)) {
                    p->ulType |= 0x01000000;
                }
                if (p->ulType & 0x08000000) {
                    p->stPosition.xZ -= p->fScale[1];
                }
            }
        }

        if ((p->ulType & 0xFF) == 7 || (p->ulType & 0xFF) == 11) {
            p->stPosition.xX -= offB;
            p->stPosition.xY -= offA;
            p->stPosition.xZ -= offA + offB;
        }
        if ((p->ulType & 0xFF) == 8) {
            p->stPosition.xX -= offB;
            p->stPosition.xY -= offA;
            p->stPosition.xZ -= offC;
        }
    }
}

void AGO_vVectorMulAddInternal(f32* destination, f32* source, f32 factor)
{
    s32 remaining = 2;

    do {
        *destination = *destination + (*source * factor);
        source += 1;
        remaining -= 1;
        destination += 1;
    } while (remaining >= 0);
}

static __inline__ f32 inv_AGO_fn_xRandRange(f32 min, f32 max)
{
    return (f32) rand() * (max - min) * RAND_SCALE_inv + min;
}

static __inline__ void inv_AGO_fn_vApproachVector(AGO_Part_* p, f32 limit, f32 factor)
{
    s32 i;
    s32 hit;
    f32* dst;
    f32* src;
    f32* old;
    f32 d, step, ad;
    hit = 0;
    old = &p->stInitialPosition.xX;
    dst = &p->stPosition.xX;
    src = &p->stDirection.xX;
    i = 2;
    do {
        d = *src - *dst;
        ad = __builtin_fabsf(d);
        if (ad <= limit) {
            hit += 1;
        } else {
            *old = *dst;
            step = d * factor;
            step *= g_fTimeDifference;
            if (ad <= inv_MTH_fn_xAbsolute(step)) {
                *dst = *src;
                hit += 1;
            } else {
                *dst += step;
            }
        }
        old++;
        dst++;
        src++;
        i--;
    } while (i >= 0);
    if (hit != 0)
        p->ulType |= 0x01000000;
}

void AGO_vProcessPart(s32 ulPartNumber)
{
    AGO_Part_* p;
    f32 Angle;
    f32 phase;
    f32 Cos1;
    f32 Sin1;
    s32 i;
    MTH3D_tdstVector_ V1, V2;
    f32* pp;
    f32* pp2;
    f32 unscale;

    p = &g_AGO[ulPartNumber];
    if (p->ulType & 0x40)
        return;

    switch (p->ulType & 0xFF) {
    case 0:
    case 2:
    case 13:
    case 15:
    case 17:
        break;
    case 16:
        p->stDirection.xZ += g_fTimeDifference * -9.81f * 4.0f;
    case 1:
    case 4:
        p->stDirection.xZ += g_fTimeDifference * -9.81f;
    case 5:
        p->stDirection.xZ += g_fTimeDifference * -3.5f;
    case 3:
    case 7:
    case 11:
        AGO_vVectorMulAddInternal(&p->stPosition.xX, &p->stDirection.xX, g_fTimeDifference);
        break;
    case 14:
        inv_AGO_fn_vApproachVector(p, 0.1f, 6.5f);
        break;
    case 10:
        inv_AGO_fn_vApproachVector(p, 0.1f, 1.5f);
        break;
    case 18:
        pp2 = &p->stInitialPosition.xX;
        pp = &p->stPosition.xX;
        for (i = 0; i < 3; i++, pp++, pp2++)
            *pp += *pp2;
        p->LimitParam.fTimeLimit += 1.0f;
        if (p->LimitParam.fTimeLimit >= 19.0f)
            p->ulType |= 0x01000000;
        break;
    case 9:
        pp = &p->stDirection.xX;
        unscale = -1.6f * g_fTimeDifference;
        pp[0] += unscale;
        pp[1] += unscale;
        pp[2] += unscale;
        AGO_vVectorMulAddInternal(&p->stPosition.xX, pp, g_fTimeDifference);
        break;
    case 8:
        pp = &p->stInitialPosition.xX;
        if (pp[0] < 0.25f) {
            pp[0] *= 1.51f;
            pp[1] *= 1.1f;
        } else if (pp[0] < 0.5f) {
            pp[0] *= 1.1f;
            pp[1] *= 1.01f;
        } else {
            pp[0] *= 0.9f;
            pp[1] *= 0.98f;
        }
        break;
    case 12:
        p->fAnimTimeScale += g_fTimeDifference * 6.5f * p->AnimTime.x;
        p->AnimTime.x += g_fTimeDifference * 0.2f;
        p->LimitParam.fTimeLimit -= g_fTimeDifference;
        if (p->LimitParam.fTimeLimit <= 0.01f)
            p->ulType |= 0x01000000;
        Angle = p->fAnimTimeScale;
        phase = inv_AGO_fn_xTruncPhase(Angle);
        Cos1 = cosf(Angle);
        Sin1 = inv_AGO_fn_xSinFromCos(Cos1, phase);
        Cos1 *= p->LimitParam.fTimeLimit;
        Sin1 *= p->LimitParam.fTimeLimit;
        AGO_Define_OrthogonalVectors(&p->stDirection, &V1, &V2);
        MTH3D_M_vSetXYZ_inv(&p->stPosition, p->stInitialPosition.xX + V1.xX * Sin1 + V2.xX * Cos1,
                            p->stInitialPosition.xY + V1.xY * Sin1 + V2.xY * Cos1,
                            p->stInitialPosition.xZ + V1.xZ * Sin1 + V2.xZ * Cos1);
        break;
    case 6: {
        f32 pos0, pos1, pos2;
        f32 cosx, sinx, cosy, siny, cosz, sinz;
        f32 xOffsetX, xOffsetY, xOffsetZ;
        f32 xInvSqrt2, xHalf;

        pos0 = p->stInitialPosition.xX * g_ffFrameTime;
        pos1 = p->stInitialPosition.xY * g_ffFrameTime;
        pos2 = p->stInitialPosition.xZ * g_ffFrameTime;
        xInvSqrt2 = 0.707f;
        xHalf = 0.5f;
        sinx = inv_AGO_fn_xSinCos(pos0, &cosx);
        siny = inv_AGO_fn_xSinCos(pos1, &cosy);
        sinz = inv_AGO_fn_xSinCos(pos2, &cosz);
        xOffsetX = (cosy * cosz) * xInvSqrt2 + (((-siny * cosx) * cosz) - (cosx * sinz)) * xHalf +
                   (((-siny * cosx) * cosz) + (sinx * sinz)) * xHalf;
        xOffsetY = (cosy * sinz) * xInvSqrt2 + (((-sinz * siny) * sinx) + (cosx * cosz)) * xHalf +
                   (((-sinz * siny) * cosx) - (sinx * cosz)) * xHalf;
        xOffsetZ = siny * xInvSqrt2 + (sinx * siny) * xHalf + (cosx * cosy) * xHalf;
        MTH3D_M_vSetXYZ_inv(&p->stPosition, p->stDirection.xX + p->LimitParam.fTimeLimit * xOffsetX,
                            p->stDirection.xY + p->LimitParam.fTimeLimit * xOffsetY,
                            p->stDirection.xZ + p->LimitParam.fTimeLimit * xOffsetZ);
        p->LimitParam.fTimeLimit -= g_fTimeDifference * 4.5f;
        if (p->LimitParam.fTimeLimit <= 0.01f)
            p->ulType |= 0x01000000;
    } break;
    }

    if ((p->ulType & 0x20000) && g_AGO_PloufMaterial && !(p->ulType & 0x80000)) {
        if (p->stPosition.xZ <= p->LimitParam.f_ZLimit) {
            p->stPosition.xZ = p->LimitParam.f_ZLimit;
            p->ulType = 0x10003100;
            p->fScale[0] = 0.1f;
            p->fScale[1] = 0.1f;
            p->fCreationTime = g_ffFrameTime;
            p->Transparency = 250.0f;
            p->VisualMaterial = (void*) g_AGO_PloufMaterial;
        }
    } else if ((p->ulType & 0x02000000) && g_AGO_PloufMaterial && !(p->ulType & 0x80000)) {
        if (p->stPosition.xZ <= p->LimitParam.f_ZLimit) {
            f32 randomValue;
            randomValue = (f32) rand() / RAND_RANGE_inv + 0.0f;
            if (0.5f < randomValue) {
                p->stPosition.xZ = p->LimitParam.f_ZLimit;
                p->ulType = 0x10004100;
                p->ulBehaviourFlags = 0;
                p->fScale[0] = 0.075f;
                p->fScale[1] = 0.075f;
                p->fCreationTime = g_ffFrameTime;
                p->Transparency = 150.0f;
                p->VisualMaterial = (void*) g_AGO_PloufMaterial;
            } else {
                p->ulType |= 0x01000000;
            }
        }
    }

    unscale = 0.0f;
    switch (p->ulType & 0xF00) {
    case 0x900:
        p->fScale[0] += g_fTimeDifference * 5.3f;
        p->fScale[1] += g_fTimeDifference * 20.5f;
        break;
    case 0xA00:
        p->fScale[0] += g_fTimeDifference * 60.0f;
        p->fScale[1] -= g_fTimeDifference * 7.5f;
        if (p->fScale[0] >= 5.0f)
            p->ulType = (p->ulType & 0xFFFFF0FF) | 0xB00;
        break;
    case 0xB00:
        p->fScale[0] -= g_fTimeDifference * 60.0f;
        if (p->fScale[0] <= 1.0f) {
            p->fScale[0] = 2.0f;
            p->ulType = (p->ulType & 0xFFFFF0FF) | 0xD00;
        }
        break;
    case 0xD00:
        p->fScale[1] += g_fTimeDifference * 61.5f;
        p->fScale[0] -= g_fTimeDifference * 16.5f;
        if (p->fScale[1] >= 10.0f) {
            p->ulType |= 0x01000000;
        }
        break;
    case 0x800:
        unscale = 0.05f;
        break;
    case 0x100:
        unscale = 1.3f;
        break;
    case 0x200:
        unscale = 3.7f;
        break;
    case 0x300:
        unscale = 6.5f;
        break;
    case 0x400:
        unscale = 9.5f;
        break;
    case 0x500:
        unscale = -0.5f;
        break;
    case 0x700:
        unscale = -3.5f;
        break;
    case 0x600:
        unscale = -0.2f;
        break;
    }
    if (unscale != 0.0f) {
        unscale *= g_fTimeDifference;
        p->fScale[0] += unscale;
        p->fScale[1] += unscale;
    }

    if ((p->ulType & 0xF00) == 0xE00) {
        f32 randomValue;
        randomValue = inv_AGO_fn_xRandRange(0.0f, 1.0f);
        p->fScale[0] = randomValue;
        p->fScale[1] = randomValue;
    }
    if (p->fScale[0] <= 0.0f) {
        p->fScale[0] = 0.01f;
        p->ulType = p->ulType | 0x01000000;
    }
    if (p->fScale[1] <= 0.0f) {
        p->fScale[1] = 0.01f;
        p->ulType = p->ulType | 0x01000000;
    }

    switch (p->ulType & 0xF000) {
    case 0x8000:
        if (p->LimitParam.fTimeLimit < g_ffFrameTime - p->fCreationTime) {
            p->ulType = (p->ulType & 0xFFFF0FFF) | 0x3000;
        }
        break;
    case 0x7000:
        p->Transparency += g_fTimeDifference * 350.0f;
        if (p->Transparency >= 255.0f) {
            p->Transparency = 255.0f;
            p->fCreationTime = g_ffFrameTime;
            p->ulType = (p->ulType & 0xFFFF0FFF) | 0x8000;
        }
        break;
    case 0x1000:
        p->Transparency -= g_fTimeDifference * 100.0f;
        break;
    case 0x2000:
        p->Transparency -= g_fTimeDifference * 160.0f;
        break;
    case 0x3000:
        p->Transparency -= g_fTimeDifference * 260.0f;
        break;
    case 0x4000:
        p->Transparency -= g_fTimeDifference * 350.0f;
        break;
    case 0xB000:
        p->Transparency -= g_fTimeDifference * 500.0f;
        break;
    case 0x5000:
        p->Transparency -= g_fTimeDifference * 850.0f;
        break;
    case 0x6000:
        p->Transparency -= g_fTimeDifference * 3500.0f;
        if (p->Transparency < 5.0f) {
            p->Transparency = 0.0f;
            p->ulType = p->ulType | 0x01000000;
        }
        break;
    case 0x9000:
        p->Transparency = inv_AGO_fn_xRandRange(100.0f, 255.0f);
        break;
    case 0xA000:
        p->Transparency += g_fTimeDifference * 100.0f;
        break;
    }
    if (p->ulBehaviourFlags & 8)
        p->Transparency -= g_fTimeDifference * 20.0f;
    if (p->Transparency < 0.0f) {
        if ((p->ulType & 0xF000) != 0x7000) {
            p->Transparency = 0.0f;
            p->ulType |= 0x01000000;
        }
    }
}

void CreateType5AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ Pos;
    MTH3D_tdstVector_ Direction;
    f32 zadd = 0.6f;
    f32 randMul = 1.3f;
    f32 randScale = RAND_SCALE_inv;
    f32 half = 0.5f;
    f32 randBias;
    f32 one = 1.0f;
    f32 xSmallScale;
    f32 negHalf;

    p_stPosition->xZ += zadd;
    randBias = 0.7f;

    g_AGO_AnimTimeScale = (f32) rand() * randMul * randScale + randBias;
    Direction.xX = one;
    MTH3D_M_vSetXYZ_inv(&Pos, p_stPosition->xX, p_stPosition->xY + half, p_stPosition->xZ + 0.8f);
    Direction.xY = 0.56f;
    Direction.xZ = -0.53f;
    AGO_vCreatePart(0x1103, &Pos, &Direction, parameter, 0.8f, 0.8f, hVisualMaterial);

    g_AGO_AnimTimeScale = (f32) rand() * randMul * randScale + randBias;
    negHalf = -0.5f;
    MTH3D_M_vSetXYZ_inv(&Pos, p_stPosition->xX, p_stPosition->xY - half, p_stPosition->xZ + 1.5f);
    MTH3D_M_vSetXYZ_inv(&Direction, negHalf, -0.98f, 0.23f);
    AGO_vCreatePart(0x1103, &Pos, &Direction, parameter, half, zadd, hVisualMaterial);

    g_AGO_AnimTimeScale = (f32) rand() * randMul * randScale + randBias;
    xSmallScale = 0.3f;
    MTH3D_M_vSetXYZ_inv(&Pos, p_stPosition->xX, p_stPosition->xY + half, p_stPosition->xZ - half);
    MTH3D_M_vSetXYZ_inv(&Direction, half, 0.16f, 0.13f);
    AGO_vCreatePart(0x1103, &Pos, &Direction, parameter, xSmallScale, xSmallScale, hVisualMaterial);

    g_AGO_AnimTimeScale = (f32) rand() * randMul * randScale + randBias;
    MTH3D_M_vSetXYZ_inv(&Pos, p_stPosition->xX, p_stPosition->xY - half, p_stPosition->xZ);
    MTH3D_M_vSetXYZ_inv(&Direction, one, -0.29f, -0.63f);
    AGO_vCreatePart(0x1103, &Pos, &Direction, parameter, one, one, hVisualMaterial);
}

void CreateType8AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH4D_tdstVector_ stLeftPos;
    MTH4D_tdstVector_ Direction;
    MTH4D_tdstVector_ stRightPos;
    f32 xMinLength;
    f32 xOffsetZ;
    f32 ny;
    f32 nx;
    f32 len2;
    f32 len;
    f32 xScale;
    f32 xJitterMax;
    f32 xJitterMin;
    f32 xTimeLimit;
    f32 xScaleY;

    xMinLength = 0.01f;
    xOffsetZ = 0.2f;
    *(MTH3D_tdstVector_*) &stLeftPos = *p_stPosition;
    stLeftPos.xZ = stLeftPos.xZ + xOffsetZ;
    *(MTH3D_tdstVector_*) &stRightPos = *(MTH3D_tdstVector_*) &stLeftPos;
    *(MTH3D_tdstVector_*) &Direction = *p_stDirection;
    ny = -Direction.xY;
    nx = Direction.xX;
    len2 = ny * ny + nx * nx;
    len = MTH_fn_xSqrtOpt2(len2);
    if (xMinLength <= len) {
    lbl:
        xScale = 0.4f / len;
        xJitterMax = 0.9f;
        parameter = RAND_SCALE_inv;
        xJitterMin = -0.9f;
        xTimeLimit = 200.0f;
        xScaleY = 0.8f;
        ny = ny * xScale;
        nx = nx * xScale;

        inv_AGO_fn_vJitterPoint(&stLeftPos, &Direction, -ny, -nx, xJitterMax, parameter, xJitterMin);
        inv_AGO_fn_vJitterPoint(&stRightPos, &Direction, ny, nx, xJitterMax, parameter, xJitterMin);

        AGO_vCreatePart(0x30005100, (MTH3D_tdstVector_*) &stLeftPos, (MTH3D_tdstVector_*) &Direction, xTimeLimit, xOffsetZ, xScaleY, hVisualMaterial);
        AGO_vCreatePart(0x30005100, (MTH3D_tdstVector_*) &stRightPos, (MTH3D_tdstVector_*) &Direction, xTimeLimit, xOffsetZ, xScaleY, hVisualMaterial);
    }
}

void CreateType52AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    f32 positionWork[4];
    f32 directionWork[4];
    f32* pC;
    f32* pIn0;
    f32* pIn1;
    f32* positionOut;
    f32* directionOut;
    f32 four;
    f32 eight;
    f32 scale;
    f32 negFour;
    f32 d98;
    f32 d9c;
    f32 da0;
    f32 randomValue;
    f32 size;

    s32 remaining;

    *(MTH3D_tdstVector_*) positionWork = *p_stPosition;
    *(MTH3D_tdstVector_*) directionWork = *p_stDirection;
    pC = (f32*) &AGO_gExtraParameter2;
    pIn0 = (f32*) p_stPosition;
    pIn1 = (f32*) p_stDirection;
    positionOut = positionWork;
    directionOut = directionWork;
    four = 4.0f;
    eight = 8.0f;
    scale = RAND_SCALE_inv;
    negFour = -4.0f;
    d98 = 0.3f;
    d9c = 0.4f;
    da0 = -0.2f;
    remaining = 2;
    do {
        randomValue = (f32) rand();
        randomValue = randomValue * eight;
        randomValue = randomValue * scale;
        randomValue = randomValue + negFour;

        *positionOut = *pIn0 + *pIn1 * four + randomValue + *pC * d98;
        *directionOut = (f32) rand() * d9c * scale + da0 + *pC;
        pIn0 = pIn0 + 1;
        pIn1 = pIn1 + 1;
        pC = pC + 1;
        positionOut = positionOut + 1;
        directionOut = directionOut + 1;
        remaining = remaining - 1;
    } while (remaining >= 0);
    size = (f32) rand() * 0.049999982f * RAND_SCALE_inv + 0.4f;

    AGO_vCreatePart(0x90003, (MTH3D_tdstVector_*) positionWork, (MTH3D_tdstVector_*) directionWork, 1.0f, size, size, hVisualMaterial);
}

void CreateType60AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ from;
    MTH3D_tdstVector_ vel;
    MTH3D_tdstVector_* vp;
    f32 eight;
    f32 randScale;
    f32 negFour;
    f32 five;
    f32 rndScale;
    f32 rndBase;
    f32 size;
    s32 remaining;

    if (D_00393CF1 != 0) {
        return;
    }
    if (D_00393CF1 != 0) {
        return;
    }
    from = (*(MTH3D_tdstVector_*) p_stPosition);
    vel = (*(MTH3D_tdstVector_*) p_stDirection);
    vp = &vel;
    (*(void**) &AGO_g_BigRainMaterial) = hVisualMaterial;
    eight = 8.0f;
    randScale = RAND_SCALE_inv;
    negFour = -4.0f;
    five = 5.0f;
    rndScale = 0.01f;
    rndBase = 0.02f;
    remaining = 9;
    do {
        MTH3D_M_vSetXYZ_inv(&vel, (f32) rand() * eight * randScale + negFour,
                            (f32) rand() * eight * randScale + negFour,
                            (f32) rand() * five * randScale + five);
        size = (f32) rand() * rndScale * randScale + rndBase;
        AGO_vCreatePart(0x21001, &from, (MTH3D_tdstVector_*) vp, from.xZ, size, size, hVisualMaterial);
        remaining = remaining - 1;
    } while (remaining >= 0);
}

void CreateType62AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ from __attribute__((aligned(8)));
    MTH3D_tdstVector_ vel __attribute__((aligned(8)));
    MTH3D_tdstVector_* vp;
    f32 randScale;
    f32 negOne;
    f32 sizeScale;
    f32 sizeBase;
    f32 spreadScale;
    f32 spreadBase;
    f32 half;
    f32 size;
    f32 spread;
    f32 random;
    s32 remaining;

    from = (*(MTH3D_tdstVector_*) p_stPosition);
    randScale = RAND_SCALE_inv;
    negOne = -1.0f;
    sizeScale = 0.09f;
    sizeBase = 0.01f;
    spreadScale = 0.8f;
    spreadBase = 0.2f;
    half = 0.5f;
    vel = (*(MTH3D_tdstVector_*) p_stDirection);
    vp = &vel;
    remaining = 9;
    do {
        remaining--;
        vel.xX = (f32) rand() * 2.0f * randScale + negOne;
        random = (f32) rand();
        *(s32*) &vel.xZ = 1;
        *(s32*) &vel.xZ = 2;
        *(s32*) &vel.xZ = 3;
        *(s32*) &vel.xZ = 4;
        *(s32*) &vel.xZ = 5;
        *(s32*) &vel.xZ = 6;
        *(s32*) &vel.xZ = 7;
        *(s32*) &vel.xZ = 8;
        *(s32*) &vel.xZ = 9;
        *(s32*) &vel.xZ = 10;
        *(s32*) &vel.xZ = 11;
        *(s32*) &vel.xZ = 12;
        *(s32*) &vel.xZ = 13;
        *(s32*) &vel.xZ = 14;
        *(s32*) &vel.xZ = 15;
        *(s32*) &vel.xZ = 16;
        *(s32*) &vel.xZ = 17;
        *(s32*) &vel.xZ = 18;
        *(s32*) &vel.xZ = 19;
        *(s32*) &vel.xZ = 20;
        *(s32*) &vel.xZ = 21;
        *(s32*) &vel.xZ = 22;
        *(s32*) &vel.xZ = 23;
        *(s32*) &vel.xZ = 24;
        *(s32*) &vel.xZ = 0;
        vel.xY = random * 2.0f * randScale + negOne;
        random = (f32) rand();
        size = random * sizeScale * randScale + sizeBase;
        if (D_00393CF1 == 0) {
            goto half_spread;
        }
        spread = (f32) rand() * spreadScale * randScale + spreadBase;
        goto spread_join;
        half_spread:
        spread = (f32) rand() * half * randScale + 1.0f;
    spread_join:
        AGO_vCreatePart(0x10003, &from, (MTH3D_tdstVector_*) vp, spread, size, size, hVisualMaterial);
    } while (remaining >= 0);
}

void CreateType35AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection, void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ Pos;
    MTH3D_tdstVector_ Direction;
    MTH3D_tdstVector_ side;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ direction;
    u32 bits;
    f32 inv_len;
    f32 angle;
    f32 turns;
    f32 cosine;
    f32 sign;
    f32 sine;
    f32 len2;

    inv_MTH3D_fn_vSubXYZ(&direction, p_stPosition, p_stDirection);
    len2 = direction.xX * direction.xX + direction.xY * direction.xY + direction.xZ * direction.xZ;
    {
        f32 xInvLength;
        xInvLength = inv_MTH_fn_xDivideBySqrt(1.0f, len2);
        MTH3D_M_vScaleXYZ_inv(&direction, &direction, xInvLength);
    }

    AGO_Define_OrthogonalVectors(&direction, &side, &up);

    {
        f32 phase;
        s32 itrunc;
        f32 period;

        len2 = (f32) rand() * 12.56f * RAND_SCALE_inv + (-6.28f);
        period = MTH_TWO_PI_inv;
        turns = len2 * MTH_INV_TWO_PI_inv;
        itrunc = inv_MTH_fn_lToIntegerOrdered(turns);
        phase = len2 - (f32) itrunc * period;
        cosine = cosf(len2);

        if (__builtin_fabsf(phase) < MTH_PI_inv) {
            sign = inv_MTH_fn_xSign(phase);
        } else {
            sign = -inv_MTH_fn_xSign(phase);
        }
    }

    sine = MTH_fn_xSqrtOpt2(1.0f - cosine * cosine) * sign;

    MTH3D_M_vSetXYZ_inv(&direction, cosine * side.xX + sine * up.xX,
                        cosine * side.xY + sine * up.xY, cosine * side.xZ + sine * up.xZ);

    len2 = direction.xX * direction.xX + direction.xY * direction.xY + direction.xZ * direction.xZ;
    if (len2 < 1.0f) {
        if (AGO_g_OddFrame != 0) {
            len2 = len2 * parameter;
            goto scale_done;
        }
    }
    inv_len = inv_MTH_fn_xDivideBySqrt(1.0f, len2);
    len2 = inv_len * parameter;

scale_done:
    MTH3D_M_vSetXYZ_inv(&Pos, p_stDirection->xX + len2 * direction.xX,
                        p_stDirection->xY + len2 * direction.xY,
                        p_stDirection->xZ + len2 * direction.xZ);
    MTH3D_M_vCopyXYZ_inv(&Direction, p_stPosition);
    {
        f32 xScale = 0.9f;
        AGO_vCreatePart(0x4000000A, &Pos, &Direction, 2.0f, xScale, xScale, hVisualMaterial);
    }
}

void CreateType15AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ Pos;
    MTH3D_tdstVector_ Direction;
    f32 randScale;
    f32 offset;
    f32 speed;
    f32 spread;
    f32 ra;
    f32 rb;
    f32 rc;
    f32 xPos;
    f32 xDir;
    void* part;
    f32* pp;
    f32* posp;
    f32* vp;
    f32* vb;
    f32* vw;
    f32* dirp;
    s32 i;
    s32 j;
    s32 ri;

    randScale = RAND_SCALE_inv;
    vb = (f32*) &Direction;
    vw = vb;
    i = 8;
    do {
        g_AGO_AnimTimeScale = (f32) rand() * 2.8f * randScale + 0.2f;
        ri = rand();
        rb = (f32) ri;
        rb = rb * 1.3f;
        rb = rb * randScale;
        offset = rb + -0.5f;
        ri = rand();
        rc = (f32) ri;
        rc = rc * 1.7f;
        rc = rc * randScale;
        rc = rc + 0.5f;
        speed = rc * parameter;
        ri = rand();
        ra = (f32) ri;
        spread = ra * 0.7f * randScale + 0.8f;
        pp = (f32*) &Pos;
        posp = (f32*) p_stPosition;
        vb = vw;
        vp = vb;
        dirp = (f32*) p_stDirection;
        j = 2;
        do {
            xDir = *dirp++;
            xPos = *posp++;
            *vp++ = xDir * speed;
            *pp++ = xPos + offset * xDir;
            j--;
        } while (j >= 0);
        part = AGO_vCreatePart(0x3009, &Pos, &Direction, speed, spread, spread, hVisualMaterial);
        if (part != 0) {
            ((s32*) part)[1] = 1;
        }
        i--;
    } while (i != 0);
}

void inv_CreateType158AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ Pos;
    MTH3D_tdstVector_ Direction;
    f32 randScale;
    f32 three;
    f32 negOneHalf;
    f32 jitter;
    f32 zero;
    f32 spread;
    f32 speed;
    f32 speedRand;
    f32 xRandom;
    f32 xDir;
    f32 xPos;
    f32* pp;
    f32* posp;
    f32* vp;
    f32* dirp;
    s32 i;
    s32 j;

    randScale = RAND_SCALE_inv;
    three = 3.0f;
    negOneHalf = -1.5f;
    jitter = 0.3f;
    zero = 0.0f;
    spread = 2.8f;
    i = 8;
    do {
        {
            f32 scaled = (f32) rand() * spread * randScale;
            f32 offset = 0.2f;

            __asm__("" : "+f"(offset) : "f"(scaled));
            g_AGO_AnimTimeScale = scaled + offset;
        }
        speedRand = (f32) rand();
        {
            u32 bits;
            f32 twoFive;
            f32 four;

            bits = 0x40200000;
            twoFive = *(f32*) &bits;
            bits = 0x40800000;
            four = *(f32*) &bits;
            speedRand = speedRand * twoFive;
            speedRand = speedRand * randScale;
            speed = speedRand + four;
        }
        pp = (f32*) &Pos;
        posp = (f32*) p_stPosition;
        vp = (f32*) &Direction;
        dirp = (f32*) p_stDirection;
        j = 2;
        do {
            xRandom = (f32) rand();
            *vp++ = *dirp * speed + *dirp * (xRandom * three * randScale + negOneHalf);
            xRandom = (f32) rand();
            xPos = *posp++;
            xDir = *dirp++;
            *pp++ = xPos + (xRandom * jitter * randScale + zero) * xDir;
            j--;
        } while (j >= 0);
        AGO_vCreatePart(0x2103, &Pos, &Direction, speed, parameter, parameter, hVisualMaterial);
        i--;
    } while (i != 0);
}

void CreateType21AGO(f32* p_stPosition, f32* p_stDirection, void* hVisualMaterial, f32 parameter)
{
    f32 pos[3];
    f32 delta[3];
    f32 dur;
    f32 xScale;
    AGO_Part_* part;
    s32 i;
    s32 j;

    dur = parameter * 0.7f;
    for (j = 0; j < 3; j++) {
        delta[j] = (p_stDirection[j] - p_stPosition[j]) * (1.0f / dur);
    }

    for (i = 0; (f32) i < dur; i++) {
        for (j = 0; j < 3; j++) {
            pos[j] = p_stPosition[j] + ((f32) i + inv_CreateType21AGO_Offset) * delta[j];
        }
        xScale = (f32) rand() * (0.35f - 0.2f) * RAND_SCALE_inv + 0.2f;
        part = AGO_vCreatePart(0x6000, (MTH3D_tdstVector_*) pos, NULL, 1.5f, xScale, xScale, hVisualMaterial);
        if (part != NULL) {
            part->Transparency = 260.0f;
        }
    }

    inv_CreateType21AGO_Offset += g_fTimeDifference * 6.123456f;
    while (inv_CreateType21AGO_Offset >= 1.0f) {
        inv_CreateType21AGO_Offset -= 1.0f;
    }
    if (inv_CreateType21AGO_Offset < -1.0f) {
        inv_CreateType21AGO_Offset = -1.0f;
    }
}

void inv_CreateType28AGO_Callback(AGO_Part_* part)
{
    f32 randomScale;
    f32 twelve;
    f32 eight;
    f32 minusSix;
    f32 three;
    f32 xJitter;

    randomScale = RAND_SCALE_inv;
    twelve = 12.0f;
    eight = 8.0f;
    minusSix = -6.0f;
    three = 3.0f;

    xJitter = (f32) rand() * twelve * randomScale + minusSix;
    {
        f32 randomOffset;
        f32 randomFactor;

        part->stPosition.xX = inv_AGO_stType28Position.xX +
                             inv_AGO_stType28Direction.xX * eight + xJitter;
        randomFactor = 0.4f;
        randomOffset = -0.2f;
        part->stPosition.xY = inv_AGO_stType28Position.xY + inv_AGO_stType28Direction.xY * eight +
                              ((f32) rand() * twelve * randomScale + minusSix);
        {
            f32 zRandom;
            f32 zOffset;
            f32 six;
            f32 minusTwo;

            zRandom = (f32) rand();
            six = 6.0f;
            minusTwo = -2.0f;
            zOffset = zRandom * six;
            zOffset *= randomScale;
            zOffset += minusTwo;
            part->stPosition.xZ = inv_AGO_stType28Position.xZ +
                                 inv_AGO_stType28Direction.xZ * eight + zOffset;
        }
        part->stDirection.xX = (f32) rand() * randomFactor * randomScale + randomOffset;
        part->stDirection.xY = (f32) rand() * randomFactor * randomScale + randomOffset;
    }
    part->stDirection.xZ = (f32) rand() * 2.0f * randomScale + 2.5f;
    part->ulType = 0x8700B;
    {
        f32 randomValue;
        f32 minusOnePointFive;

        randomValue = (f32) rand() * three * randomScale;
        minusOnePointFive = -1.5f;
        part->stInitialPosition.xX = randomValue + minusOnePointFive;
    }
    part->stInitialPosition.xZ = (f32) rand() * three * randomScale + 1.0f;
    part->Transparency = 215.0f;
    part->fCreationTime = g_ffFrameTime;
}

void CreateType28AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                       void* hVisualMaterial, f32 parameter)
{
    f32 position[3];
    f32 direction[3];
    f32 extent;
    f32 randomUnit;
    f32 bias;
    f32 directionExtent;
    f32 directionBias;
    f32 scale;
    AGO_Part_* particle;

    inv_AGO_stType28Position = *p_stPosition;
    inv_AGO_stType28Direction = *p_stDirection;
    (*(MTH3D_tdstVector_*) &inv_AGO_stType28Direction2D) = *p_stDirection;
    AGO_vNormThisVector((MTH3D_tdstVector_*) &inv_AGO_stType28Direction);
    AGO_vNormThis2DVector((MTH2D_tdstVector_*) inv_AGO_stType28Direction2D);
    if (inv_CreateType28AGO_skipCount < 2) {
        inv_CreateType28AGO_skipCount++;
        return;
    }
    inv_CreateType28AGO_skipCount = 0;
    extent = 8.0f;
    randomUnit = RAND_SCALE_inv;
    bias = -4.0f;
    position[0] = p_stPosition->xX + p_stDirection->xX * extent +
                  ((f32) rand() * extent * randomUnit + bias);
    directionExtent = 0.4f;
    directionBias = -0.2f;
    position[1] = p_stPosition->xY + p_stDirection->xY * extent +
                  ((f32) rand() * extent * randomUnit + bias);
    position[2] = p_stPosition->xZ + p_stDirection->xZ * extent +
                  ((f32) rand() * randomUnit + -5.0f);
    scale = (f32) rand() * (parameter * 0.15f - parameter * 0.2f);
    scale *= randomUnit;
    scale += parameter * 0.2f;
    direction[0] = (f32) rand() * directionExtent * randomUnit + directionBias;
    direction[1] = (f32) rand() * directionExtent * randomUnit + directionBias;
    direction[2] = (f32) rand() * 2.0f * randomUnit + 2.5f;
    particle = AGO_vCreatePart(0x8700B, (MTH3D_tdstVector_*) position, (MTH3D_tdstVector_*) direction, 0.5f, scale, scale, hVisualMaterial);
    if (particle != 0) {
        particle->ulBehaviourFlags = 0x10;
        particle->pfnCallback = inv_CreateType28AGO_Callback;
    }
}

void inv_CreateType355AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 scale)
{
    MTH3D_tdstVector_ positionWork;
    MTH3D_tdstVector_ delta;
    f32* pDst;
    f32* pSrc;
    f32* pRef;
    AGO_Part_* part;
    s32 component;

    pRef = (f32*) &AGO_gExtraParameter1;
    pDst = (f32*) p_stPosition;
    for (component = 0; component < 3; component++) {
        pDst[component] += pRef[component];
    }
    positionWork = *p_stPosition;
    if (AGO_bPrevAGOWasExtraParam != 0) {
        delta = inv_AGO_stPrevExtraParamPosition;
        pSrc = (f32*) &delta;
        pDst = (f32*) &positionWork;
        for (component = 0; component < 3; component++) {
            pSrc[component] = pDst[component] - pSrc[component];
        }
        part =
            AGO_vCreatePart(0x70004000, &positionWork, &delta, 0.5f, 1.0f, 1.0f, hVisualMaterial);
        if (part != 0) {
            part->stInitialPosition = AGO_gExtraParameter1;
            part->Transparency = 127.0f;
        }
    } else {
        AGO_bPrevAGOWasExtraParam = 1;
    }
    inv_AGO_stPrevExtraParamPosition = *p_stPosition;
}

void inv_CreateType356AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 scale)
{
    MTH3D_tdstVector_ base;
    MTH3D_tdstVector_ rnd;
    f32* pDst;
    f32* pSrc;
    f32 span;
    f32 tiny;
    f32 bias;
    f32 six;
    f32 neg3;
    f32 randomValue;
    s32 remaining;

    pDst = (f32*) p_stPosition;
    pSrc = (f32*) p_stDirection;
    remaining = 2;
    do {
        *pDst = *pDst + *pSrc;
        remaining = remaining - 1;
        pSrc = pSrc + 1;
        pDst = pDst + 1;
    } while (remaining >= 0);

    base = *p_stPosition;

    span = 0.02f;
    tiny = RAND_SCALE_inv;
    bias = 0.04f;
    six = 6.0f;
    neg3 = -3.0f;

    remaining = 11;
    do {
        remaining = remaining - 1;
        randomValue = (f32) rand() * span * tiny + bias;
        MTH3D_M_vSetXYZ_inv(&rnd, (f32) rand() * six * tiny + neg3,
                            (f32) rand() * six * tiny + neg3, (f32) rand() * six * tiny + neg3);
        AGO_vCreatePart(0x5001, &base, &rnd, 0.5f, randomValue, randomValue, hVisualMaterial);
    } while (remaining >= 0);
}

void inv_CreateType396AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection, void* hVisualMaterial, f32 scale)
{
    MTH3D_tdstVector_ Pos;
    f32 Direction[3];
    f32 g0, g1, g2, g3, g4, xRandScale;
    f32 k4, k16, size;
    u32 bits;
    s32 lRandom;
    AGO_Part_* p;
    s32 i;

    (void) p_stDirection;

    g0 = 0.4f;
    xRandScale = RAND_SCALE_inv;
    g1 = 0.20000002f;
    g2 = -0.2f;
    g3 = 0.6f;
    g4 = -0.3f;

    i = 3;
    do {
        i--;
        Pos = *p_stPosition;

        Pos.xZ = Pos.xZ + ((f32) rand() * g0 * xRandScale + g2);
        Direction[0] = (f32) rand() * g3 * xRandScale + g4;
        lRandom = rand();
        bits = 0x40800000;
        k4 = *(f32*) &bits;
        bits = 0x41800000;
        k16 = *(f32*) &bits;
        Direction[1] = (f32) lRandom * k4 * xRandScale + k16;
        Direction[2] = (f32) rand() * g1 * xRandScale + g0;

        size = (f32) rand() * g1 * xRandScale + g0;
        p = AGO_vCreatePart(0x1013, (MTH3D_tdstVector_*) &Pos, (MTH3D_tdstVector_*) Direction, 0.5f, size, size, hVisualMaterial);
        if (p) {
            p->AdditionnalMaterial = (void*) AGO_gExtraMaterial;
        }
    } while (i >= 0);
}

void AGO_vNormThisVector(MTH3D_tdstVector_* v)
{
    f32 x, y, z, length, inv;

    x = v->xX;
    y = v->xY;
    z = v->xZ;
    length = x * x + y * y + z * z;

    if (length != 0.0f) {
        inv = inv_MTH_fn_xDivideBySqrt(1.0f, length);
        MTH3D_M_vSetXYZ_inv(v, x * inv, y * inv, z * inv);
    }
}

void AGO_vNormThis2DVector(MTH2D_tdstVector_* v)
{
    f32 y;
    f32 x;
    f32 sum;
    f32 inv;
    f32 ny;
    f32 nx;
    volatile f32* out;

    x = v->xX;
    y = v->xY;
    sum = MTH_fn_xSqrOpt1(x) + MTH_fn_xSqrOpt1(y);
    if (sum != 0.0f) {
        inv = inv_MTH_fn_xDivideBySqrt(1.0f, sum);
        ny = y * inv;
        nx = x * inv;
        out = &v->xY;
        *out = ny;
        out = &v->xX;
        *out = nx;
    }
}

void inv_CreateType428AGO_Callback(AGO_Part_* part)
{
    f32 randomScale;
    f32 ten;
    f32 forty;
    f32 three;
    f32 span;
    f32 neg4;
    f32 randomValue;

    randomValue = (f32) rand();
    randomScale = RAND_SCALE_inv;
    ten = 10.0f;
    forty = 40.0f;
    three = 3.0f;
    span = 12.0f;
    neg4 = -4.0f;
    randomValue *= span;
    randomValue *= randomScale;
    randomValue += neg4;
    randomValue += span;
    MTH3D_M_vSetXYZ_inv(&part->stPosition,
                        inv_AGO_stType28Position.xX + randomValue * inv_AGO_stType28Direction.xX +
                            ((f32) rand() * forty * randomScale + -20.0f),
                        inv_AGO_stType28Position.xY + randomValue * inv_AGO_stType28Direction.xY +
                            ((f32) rand() * forty * randomScale + -20.0f),
                        inv_AGO_stType28Position.xZ + randomValue * inv_AGO_stType28Direction.xZ +
                            ((f32) rand() * randomScale + 5.0f));
    MTH3D_M_vSetXYZ_inv(&part->stDirection, (f32) rand() * ten * randomScale + -10.0f,
                        (f32) rand() * ten * randomScale + -10.0f,
                        (f32) rand() * ten * randomScale + -70.0f);
    part->ulType = 0x90003003;
    part->stInitialPosition.xX = (f32) rand() * three * randomScale + -1.5f;
    part->stInitialPosition.xZ = (f32) rand() * three * randomScale + 1.0f;
    part->Transparency = 60.0f;
    part->fCreationTime = g_ffFrameTime;
}

void inv_CreateType428AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                      void* hVisualMaterial, f32 parameter)
{
    s32 i;
    s32 ti;
    f32 x;
    f32 y;
    f32 d2;
    f32 radius;
    f32 angle;
    f32 rem;
    f32 c;
    f32 s;
    f32 sign;
    f32 base;
    f32 dx;
    f32 dy;
    u32 bits;
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ dir;
    AGO_Part_* part;

    inv_AGO_stType28Position = *(MTH3D_tdstVector_*) p_stPosition;
    inv_AGO_stType28Direction = *(MTH3D_tdstVector_*) p_stDirection;
    (*(MTH3D_tdstVector_*) &(*(f32(*)[]) & inv_AGO_stType28Direction2D)) =
        *(MTH3D_tdstVector_*) p_stDirection;
    AGO_vNormThisVector(&inv_AGO_stType28Direction);
    AGO_vNormThis2DVector(
        (MTH2D_tdstVector_*) inv_AGO_stType28Direction2D);

    for (i = 0; i < 30; i++) {
        x = (f32) rand() * 12.0f * RAND_SCALE_inv + -4.0f + 12.0f;
        MTH3D_M_vSetXYZ_inv(&pos,
                            p_stPosition->xX + x * p_stDirection->xX +
                                ((f32) rand() * 30.0f * RAND_SCALE_inv + -15.0f),
                            p_stPosition->xY + x * p_stDirection->xY +
                                ((f32) rand() * 30.0f * RAND_SCALE_inv + -15.0f),
                            p_stPosition->xZ + x * p_stDirection->xZ +
                                ((f32) rand() * 2.0f * RAND_SCALE_inv + 8.0f));

        do {
            x = (f32) rand() * 2.0f * RAND_SCALE_inv + -1.0f;
            y = (f32) rand() * 2.0f * RAND_SCALE_inv + -1.0f;
            d2 = x * x + y * y;
        } while (d2 < 0.01f);
        base = 1.0f;
        d2 = inv_MTH_fn_xDivideBySqrt(base, d2);

        MTH3D_M_vSetXYZ_inv(&dir, (f32) rand() * 10.0f * RAND_SCALE_inv + -10.0f,
                            (f32) rand() * 10.0f * RAND_SCALE_inv + -10.0f,
                            (f32) rand() * 10.0f * RAND_SCALE_inv + -70.0f);

        x *= d2;
        y *= d2;
        d2 = (f32) rand() * 0.040000003f * RAND_SCALE_inv + 0.06f;
        part = AGO_vCreatePart(0x90003003, &pos, &dir, 0.5f, d2, (f32) rand() * 0.5f * RAND_SCALE_inv + 0.75f, hVisualMaterial);
        if (part != NULL) {
            part->stAdditionnalVector.xX = x;
            part->Transparency = 60.0f;
            part->stAdditionnalVector.xY = y;
            part->LimitParam.f_ZLimit = inv_AGO_xType428ZLimit;
            part->ulBehaviourFlags = 0x14;
            part->stAdditionnalVector.xZ = 0;
            part->pfnCallback = inv_CreateType428AGO_Callback;
        }
    }

    if (g_DoSmallPlouf != 0) {
        for (i = 0; i < 30; i++) {
            radius = (f32) rand() * 100.0f * RAND_SCALE_inv + 10.0f;
            angle = (f32) rand() * 0.986f * RAND_SCALE_inv + -0.493f;
            dx = (*(MTH3D_tdstVector_*) &(*(f32(*)[]) & inv_AGO_stType28Direction2D)).xX;
            dy = (*(MTH3D_tdstVector_*) &(*(f32(*)[]) & inv_AGO_stType28Direction2D)).xY;

            rem = angle * MTH_INV_TWO_PI_inv;
            ti = inv_MTH_fn_lToInteger(rem);
            rem = angle - (f32) ti * MTH_TWO_PI_inv;
            c = cosf(angle);
            if (__builtin_fabsf(rem) < MTH_PI_inv) {
                sign = inv_MTH_fn_xSign(rem);
            } else {
                bits = *(u32*) &rem;
                if ((bits & 0x7FFFFFFF) == 0) {
                    base = 0.0f;
                } else {
                    bits = (bits & 0x80000000) | 0x3F800000;
                    base = *(f32*) &bits;
                }
                sign = -base;
            }
            s = MTH_fn_xSqrtOpt2(1.0f - c * c) * sign;

            MTH3D_M_vSetXYZ_inv(&pos, p_stPosition->xX + radius * (dx * c - dy * s),
                                p_stPosition->xY + radius * (dx * s + dy * c),
                                inv_AGO_xType428ZLimit);

            if (radius < 10.0f) {
                d2 = 0.1f;
                radius = 180.0f;
            } else if (radius >= 40.0f) {
                radius = 255.0f;
                d2 = 0.3f;
            } else {
                radius -= 10.0f;
                radius = radius * 2.5f + 180.0f;
                d2 = 0.2f;
            }

            part = AGO_vCreatePart(0x10001000, &pos, p_stDirection, 0.5f, d2, d2, (void*) g_AGO_PloufMaterial);
            if (part != NULL) {
                part->Transparency = radius;
                part->LimitParam.f_ZLimit = inv_AGO_xType428ZLimit;
                part->ulBehaviourFlags = 0;
            }
        }
    }
}

void inv_CreateType480AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection, void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ Pos;
    MTH3D_tdstVector_ Direction;
    MTH3D_tdstVector_ stEdge;
    MTH3D_tdstVector_ stOffset;
    MTH3D_tdstVector_* pd;
    f32* pC;
    f32* pD;
    f32* pIn;
    f32* pO;
    f32* pR;
    f32 xCorner;
    f32 scale;
    f32 zero;
    f32 negOne;
    f32 half;
    f32 xCoefA;
    f32 xCoefB;
    f32 len2;
    f32 xSize;
    s32 rnd;
    f32 spread;
    f32 angle;
    AGO_Part_* p;
    s32 i;

    pC = (f32*) &AGO_gExtraParameter2;
    pd = &Direction;
    pR = &stOffset.xX;
    pIn = (f32*) p_stPosition;
    pO = &stEdge.xX;
    pD = (f32*) &AGO_gExtraParameter1;
    i = 2;
    do {
        xCorner = *pC;
        *pO = *pD - xCorner;
        *pR = *pIn - xCorner;
        pC = pC + 1;
        pD = pD + 1;
        pIn = pIn + 1;
        pO = pO + 1;
        pR = pR + 1;
        i = i - 1;
    } while (i >= 0);

    scale = RAND_SCALE_inv;
    zero = 0.0f;
    negOne = -1.0f;
    i = 0;
    do {
        xCoefA = (f32) rand() * scale + zero;
        xCoefB = (f32) rand() * scale + zero;
        MTH3D_M_vSetXYZ_inv(&Pos,
                            AGO_gExtraParameter2.xX + xCoefA * stEdge.xX + xCoefB * stOffset.xX,
                            AGO_gExtraParameter2.xY + xCoefA * stEdge.xY + xCoefB * stOffset.xY,
                            p_stPosition->xZ);
        g_AGO_PloufMaterial = AGO_gExtraMaterial;
        do {
            xCoefA = (f32) rand() * 2.0f * scale + negOne;
            xCoefB = (f32) rand() * 2.0f * scale + negOne;
            len2 = xCoefA * xCoefA + xCoefB * xCoefB;
        } while (len2 < 0.01f);
        xSize = 1.0f;
        len2 = inv_MTH_fn_xDivideBySqrt(xSize, len2);
        Direction.xX = 0.0f;
        Direction.xY = 0.0f;
        rnd = rand();
        xCoefA = xCoefA * len2;
        xCoefB = xCoefB * len2;
        angle = (f32) rnd * 10.0f;
        Direction.xZ = angle * scale + -70.0f;
        xSize = (f32) rand();
        xSize = xSize * 0.040000003f;
        len2 = xSize * scale + 0.06f;
        half = 0.5f;
        spread = (f32) rand() * half * scale + 0.75f;
        p = AGO_vCreatePart(0x92003003, &Pos, pd, half, len2, spread, hVisualMaterial);
        if (p != 0) {
            p->stAdditionnalVector.xX = xCoefA;
            p->Transparency = 180.0f;
            p->stAdditionnalVector.xY = xCoefB;
            p->stAdditionnalVector.xZ = 0.0f;
            p->LimitParam.fTimeLimit = AGO_gExtraParameter1.xZ;
        }
        i = i + 1;
    } while (i < 2);
}

void CreateType67AGO(MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                   void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ vel;
    f32 time;
    f32 size;

    time = inv_fn_xGetEngineClockTime();
    g_ffFrameTime = time;
    if (inv_CreateType67AGO_NextBubble == 0.0f) {
        inv_CreateType67AGO_NextBubble = time;
    }
    if (time < inv_CreateType67AGO_NextBubble) {
        return;
    }
    inv_CreateType67AGO_NextBubble = time + 0.05f;

    pos.xX = p_stPosition->xX + p_stDirection->xX * 8.0f +
             ((f32) rand() * 8.0f / RAND_RANGE_inv + -4.0f) * 2.0f;
    pos.xY = p_stPosition->xY + p_stDirection->xY * 8.0f +
             ((f32) rand() * 8.0f / RAND_RANGE_inv + -4.0f) * 2.0f;
    pos.xZ =
        p_stPosition->xZ + p_stDirection->xZ * 8.0f + ((f32) rand() / RAND_RANGE_inv + -5.0f) * 2.0f;
    size =
        (f32) rand() * (parameter * 0.075f - parameter * 0.05f) / RAND_RANGE_inv + parameter * 0.05f;
    MTH3D_M_vSetXYZ_inv(&vel, (f32) rand() * 0.4f / RAND_RANGE_inv + -0.2f,
                        (f32) rand() * 0.4f / RAND_RANGE_inv + -0.2f,
                        (f32) rand() * 2.0f / RAND_RANGE_inv + 2.5f);

    AGO_vCreatePart(0xA700B, &pos, &vel, 0.5f, size, size, hVisualMaterial);
}

void inv_AGO_vOffsetEmitPosition(MTH3D_tdstVector_* position, MTH3D_tdstVector_* direction)
{
    MTH3D_tdstVector_ side;

    position->xX += direction->xX * 0.13f;
    position->xY += direction->xY * 0.13f;
    position->xZ += direction->xZ * 0.1f + 0.2f;
    MTH3D_M_vSetXYZ_inv(&side, direction->xY, -direction->xX, 0.0f);
    position->xX += side.xX * 0.3f;
    position->xY += side.xY * 0.3f;
}

void AGO_vAddParticle(u32 PartType, MTH3D_tdstVector_* p_stPosition,
                      MTH3D_tdstVector_* p_stDirection, void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ dir;
    MTH3D_tdstVector_ side;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ spawnPos;
    MTH3D_tdstVector_ target;

    AGO_Part_* part;
    f32 scale;
    s32 i, j, count, next;
    MTH3D_tdstVector_* dirPtr;
    f32* cursor;

    g_AGO_AnimTimeScale = 1.0f;
    pos = *p_stPosition;
    dir = *p_stDirection;

    switch (PartType) {
    case 175: {
        f32 lengthSq;
        f32* targetCursor;
        f32* posCursor;
        f32* dirCursor;
        f32* sideCursor;
        f32* spawnCursor;
        MTH3D_tdstVector_* sidePtr;
        MTH3D_tdstVector_* upPtr;
        MTH3D_tdstVector_* spawnPtr;
        MTH3D_tdstVector_* targetPtr;
        f32 randSpan;
        f32 randScale;
        f32 randBias;
        s32 outer;
        s32 k;
        s32 state;
        s32 firstRand;
        s32 secondRand;
        f32 component;
        f32 firstWeight;
        f32 secondWeight;

        lengthSq = 0.0f;
        state = inv_AGO_vAddParticle_skipCount175;
        if (state < 4) {
            inv_AGO_vAddParticle_skipCount175 = state + 1;
            return;
        }
        inv_AGO_vAddParticle_skipCount175 = 0;
        if (parameter == lengthSq) {
            return;
        }

        targetPtr = &target;
        dirPtr = &dir;
        parameter *= 0.5f;
        {
            f32* targetOut;
            posCursor = (f32*) &pos;
            targetOut = (f32*) targetPtr;
            dirCursor = (f32*) dirPtr;
            k = 2;
            do {
                f32 d;
                *targetOut++ = *dirCursor;
                d = *dirCursor - *posCursor++;
                *dirCursor++ = d;
                lengthSq += d * d;
                k--;
            } while (k >= 0);
        }

        if (lengthSq == 0.0f) {
            return;
        }
        sidePtr = &side;
        lengthSq = inv_MTH_fn_xDivideBySqrt(1.0f, lengthSq);
        upPtr = &up;
        spawnPtr = &spawnPos;

        dirCursor = (f32*) dirPtr;
        k = 2;
        do {
            *dirCursor = *dirCursor * lengthSq;
            dirCursor++;
            k--;
        } while (k >= 0);

        randSpan = 16.0f;
        AGO_Define_OrthogonalVectors(dirPtr, sidePtr, upPtr);
        randScale = RAND_SCALE_inv;
        randBias = -8.0f;
        outer = 0;
        do {
            next = outer + 1;
            lengthSq = 0.0f;
            spawnCursor = (f32*) spawnPtr;
            cursor = (f32*) upPtr;
            sideCursor = (f32*) sidePtr;
            k = 2;
            do {
                firstRand = rand();
                secondRand = rand();
                firstWeight = (f32) firstRand;
                secondWeight = (f32) secondRand;
                component = *sideCursor;
                {
                    f32 upComponent = *cursor;
                    cursor++;
                    sideCursor++;
                    firstWeight = firstWeight * randSpan;
                    secondWeight = secondWeight * randSpan;
                    firstWeight = firstWeight * randScale;
                    secondWeight = secondWeight * randScale;
                    firstWeight = firstWeight + randBias;
                    secondWeight = secondWeight + randBias;
                    component = component * firstWeight;
                    upComponent = upComponent * secondWeight;
                    component = component + upComponent;
                }
                *spawnCursor = component;
                spawnCursor++;
                lengthSq += component * component;
                k--;
            } while (k >= 0);
            if ((u8) k & 0x100) {
                *sideCursor = 0.0f;
            }

            {
                f32* out;
                if (lengthSq != 0.0f) {
                    f32 inv;
                    out = (f32*) spawnPtr;
                    inv = 1.0f;
                    inv = inv_MTH_fn_xDivideBySqrt(inv, lengthSq);
                    targetCursor = (f32*) targetPtr;
                    lengthSq = inv * parameter;
                    k = 2;
                    do {
                        *out = *targetCursor + *out * lengthSq;
                        targetCursor++;
                        out++;
                        k--;
                    } while (k >= 0);
                }
            }
            scale = (f32) rand() * 0.105f * randScale + 0.025f;
            part = AGO_vCreatePart(0x12, spawnPtr, &pos, 2.0f, scale, scale, hVisualMaterial);
            if (part != NULL) {
                part->ulBehaviourFlags = 1;
            }
            outer = next;
        } while (outer < 12);
        return;
    }

    case 1:
        parameter += 1.0f;
        AGO_vCreatePart(0x10000, &pos, NULL, 2.0f, parameter, parameter, hVisualMaterial);
        return;

    case 5:
        CreateType5AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 7: {
        f32 now;
        f32 nextBubble;
        g_AGO_PloufMaterial = (s32) hVisualMaterial;
        now = inv_fn_xGetEngineClockTime();
        nextBubble = g_fNextPlouf;
        g_ffFrameTime = now;
        if (nextBubble == 0.0f) {
            nextBubble = now;
            g_fNextPlouf = nextBubble;
        }
        if (nextBubble > now) {
            return;
        }
        nextBubble = now + 0.2f;
        g_fNextPlouf = nextBubble;
        if (dir.xX > 300.0f) {
            dir.xX = 250.0f;
        }
        if (D_00393CF1) {
            AGO_vCreatePart(0x10003100, &pos, &dir, dir.xX, dir.xZ, dir.xZ, hVisualMaterial);
        } else {
            AGO_vCreatePart(0x10003300, &pos, &dir, dir.xX, dir.xZ, dir.xZ, hVisualMaterial);
        }
        return;
    }

    case 8:
        CreateType8AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 9: {
        f32 x;
        f32 four;
        f32 minusTwo;
        f32 randScale;
        f32 zSpan;
        f32 zBias;
        f32 sizeSpan;
        s32 flag;

        randScale = RAND_SCALE_inv;
        scale = (f32) rand() * 0.2f * randScale + 0.2f;
        part = AGO_vCreatePart(0x4500, &pos, NULL, parameter, scale, scale, hVisualMaterial);
        if (part != NULL) {
            part->ulBehaviourFlags = 1;
        }

        four = 4.0f;
        count = 8;
        minusTwo = -2.0f;
        dirPtr = &dir;
        zSpan = 6.2f;
        flag = 1;
        zBias = -0.2f;
        sizeSpan = 0.1f;
        do {
            x = (f32) rand();
            x = x * four;
            x = x * randScale;
            dir.xX = x + minusTwo;
            x = (f32) rand();
            x = x * four;
            x = x * randScale;
            dir.xY = x + minusTwo;
            dir.xZ = (f32) rand() * zSpan * randScale + zBias;
            scale = (f32) rand() * sizeSpan * randScale + sizeSpan;
            part = AGO_vCreatePart(0xB001, &pos, dirPtr, parameter, scale, scale, hVisualMaterial);
            if (part != NULL) {
                part->ulBehaviourFlags = flag;
            }
            count--;
        } while (count != 0);
        if ((u8) (s32) part & 0x100) {
            dir.xZ = zSpan;
        }
        return;
    }

    case 10: {
        f32 now;
        f32 nextBubble;
        now = inv_fn_xGetEngineClockTime();
        nextBubble = inv_AGO_vAddParticle_NextBubble10;
        g_ffFrameTime = now;
        if (nextBubble == 0.0f) {
            nextBubble = now;
            inv_AGO_vAddParticle_NextBubble10 = nextBubble;
        }
        if (nextBubble > now) {
            return;
        }
        nextBubble = now + 0.3f;
        inv_AGO_vAddParticle_NextBubble10 = nextBubble;
        MTH3D_M_vZeroXYZ_inv(&dir);
        scale = 0.4f;
        AGO_vCreatePart(0xB501, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        return;
    }

    case 11: {
        f32 sin1;
        f32 angle, phase, cos1, sign;
        f32 unit;
        u32 bits;
        u32 mask;
        u32 mag;
        f32 radius;
        s32 number;
        s32 step;

        radius = p_stDirection->xX;
        number = (s32) p_stDirection->xY;
        if (number <= 0) {
            return;
        }
        step = 0;
        dirPtr = &dir;
    loop11:
        j = step / number;
        angle = (f32) j * 6.28f;
        phase = angle * MTH_INV_TWO_PI_inv;
        j = inv_MTH_fn_lToIntegerOrdered(phase);
        phase = angle - (f32) j * MTH_TWO_PI_inv;
        cos1 = cosf(angle);
        if (inv_MTH_fn_xAbsolute(phase) < MTH_PI_inv) {
            sign = inv_MTH_fn_xSign(phase);
        } else {
            bits = *(u32*) &phase;
            mask = 0x7FFFFFFF;
            mag = bits & mask;
            if (mag == 0) {
                unit = 0.0f;
                sign = -unit;
            } else {
                mask = 0x80000000;
                bits = (bits & mask) | 0x3F800000;
                unit = *(f32*) &bits;
                sign = -unit;
            }
        }
        step++;
        sin1 = MTH_fn_xSqrtOpt2(1.0f - cos1 * cos1) * sign;
        g_AGO_AnimTimeScale = (f32) rand() * 0.8f * RAND_SCALE_inv + 0.2f;
        {
            f32 xOffset;
            xOffset = radius * cos1;
            {
                f32 yOffset = radius * sin1;
                MTH3D_M_vSetXYZ_inv(&pos, p_stPosition->xX + xOffset, p_stPosition->xY + yOffset,
                                    p_stPosition->xZ);
                dir.xX = xOffset;
                dir.xY = yOffset;
            }
        }
        dir.xZ = 1.53f;
        scale = (f32) rand() * RAND_SCALE_inv + 0.2f;
        AGO_vCreatePart(0x3303, &pos, dirPtr, parameter, scale, scale, hVisualMaterial);
        if (step < number) {
            goto loop11;
        }
        return;
    }

    case 15:
        CreateType15AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 19: {
        f32 lengthSq;
        f32 inv;
        s32 flags;
        lengthSq = p_stDirection->xX * p_stDirection->xX +
                   p_stDirection->xY * p_stDirection->xY +
                   p_stDirection->xZ * p_stDirection->xZ;
        if (lengthSq <= 0.001f) {
            return;
        }
        flags = 0x20000000;
        inv = inv_MTH_fn_xDivideBySqrt(1.0f, lengthSq);
        MTH3D_M_vScaleXYZ_inv(p_stDirection, p_stDirection, inv);
        dir = *p_stDirection;
        flags |= 0x4200;
        part = AGO_vCreatePart(flags, &pos, &dir, parameter, 0.1f, 0.1f, hVisualMaterial);
        if (part != NULL) {
            part->Transparency = 170.0f;
        }
        return;
    }

    case 20: {
        f32 lengthSq;
        f32 inv;
        s32 flags;
        lengthSq = p_stDirection->xX * p_stDirection->xX +
                   p_stDirection->xY * p_stDirection->xY +
                   p_stDirection->xZ * p_stDirection->xZ;
        if (lengthSq <= 0.001f) {
            return;
        }
        flags = 0x20000000;
        inv = inv_MTH_fn_xDivideBySqrt(1.0f, lengthSq);
        MTH3D_M_vScaleXYZ_inv(p_stDirection, p_stDirection, inv);
        dir = *p_stDirection;
        flags |= 0x5400;
        AGO_vCreatePart(flags, &pos, &dir, parameter, 0.5f, 0.5f, hVisualMaterial);
        return;
    }

    case 21:
        CreateType21AGO((f32*) p_stPosition, (f32*) p_stDirection, hVisualMaterial, parameter);
        return;

    case 23: {
        f32 x;
        f32 minusOne;
        f32* out;
        MTH3D_tdstVector_* origin;

        cursor = (f32*) &dir;
        minusOne = -1.0f;
        i = 2;
        origin = (MTH3D_tdstVector_*) cursor;
        inv_AGO_vOffsetEmitPosition(&pos, origin);
        dir = *p_stPosition;
        scale = (f32) rand() * (0.15f - 0.1f) * RAND_SCALE_inv + 0.1f;
        out = (f32*) &pos;
        do {
            x = (f32) rand();
            x = x + x;
            x = x * RAND_SCALE_inv;
            x = x + minusOne;
            *out = *cursor + x;
            cursor++;
            out++;
            i--;
        } while (i >= 0);
        AGO_vCreatePart(0xE, &pos, origin, parameter, scale, scale, hVisualMaterial);
        return;
    }

    case 26: {
        f32 now;
        f32 nextBubble;
        f32 half;
        f32 randScale;
        f32 zero;
        f32 minusOne;
        zero = 0.0f;
        now = inv_fn_xGetEngineClockTime();
        nextBubble = inv_AGO_vAddParticle_NextBubble26;
        g_ffFrameTime = now;
        if (nextBubble == zero) {
            nextBubble = now;
            inv_AGO_vAddParticle_NextBubble26 = nextBubble;
        }
        if (nextBubble > now) {
            return;
        }
        half = 0.5f;
        randScale = RAND_SCALE_inv;
        nextBubble = g_ffFrameTime + ((f32) rand() * (half - zero) * randScale + zero);
        inv_AGO_vAddParticle_NextBubble26 = nextBubble;
        if (parameter < pos.xZ) {
            return;
        }
        scale = (f32) rand() * 0.08f * randScale + 0.02f;
        minusOne = -1.0f;
        MTH3D_M_vSetXYZ_inv(&dir, (f32) rand() * 2.0f * randScale + minusOne,
                            (f32) rand() * 2.0f * randScale + minusOne,
                            (f32) rand() * 3.0f * randScale + 1.0f);
        AGO_vCreatePart(0x40807, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        pos.xY += 0.2f;
        pos.xZ += 0.2f;
        if (pos.xZ < parameter) {
            AGO_vCreatePart(0x40807, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        }
        pos.xY -= 0.5f;
        pos.xZ += 0.1f;
        if (pos.xZ < parameter) {
            AGO_vCreatePart(0x40807, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        }
        return;
    }

    case 27: {
        f32 now;
        f32 nextBubble;
        f32 half;
        f32 randScale;
        f32 zero;
        f32 minusOne;
        zero = 0.0f;
        now = inv_fn_xGetEngineClockTime();
        nextBubble = inv_AGO_vAddParticle_NextBubble27;
        g_ffFrameTime = now;
        if (nextBubble == zero) {
            nextBubble = now;
            inv_AGO_vAddParticle_NextBubble27 = nextBubble;
        }
        if (nextBubble > now) {
            return;
        }
        half = 0.5f;
        randScale = RAND_SCALE_inv;
        nextBubble = g_ffFrameTime + ((f32) rand() * (half - zero) * randScale + zero);
        inv_AGO_vAddParticle_NextBubble27 = nextBubble;
        if (parameter > 7.0f) {
            parameter = 7.0f;
        }
        scale = (f32) rand() * 0.08f * randScale + 0.02f;
        minusOne = -1.0f;
        MTH3D_M_vSetXYZ_inv(&dir, (f32) rand() * 2.0f * randScale + minusOne,
                            (f32) rand() * 2.0f * randScale + minusOne,
                            (f32) rand() * 2.0f * randScale + 2.0f);
        AGO_vCreatePart(0x10807, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        pos.xY += 0.2f;
        pos.xZ += 0.2f;
        AGO_vCreatePart(0x10807, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        pos.xY -= 0.5f;
        pos.xZ += 0.1f;
        AGO_vCreatePart(0x10807, &pos, &dir, parameter, scale, scale, hVisualMaterial);
        return;
    }

    case 28:
        CreateType28AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 30: {
        s32 r;
        r = rand();
        scale = (f32) r * 0.3f * RAND_SCALE_inv + 0.5f;
        AGO_vCreatePart(0x4500, &pos, NULL, parameter, scale, scale, hVisualMaterial);
        count = (s32) parameter;
        if (count == 0) {
            return;
        }
        dirPtr = &dir;
    loop30:
        count--;
        {
            f32 x;
            f32 four;
            four = 4.0f;
            x = (f32) rand();
            x = x * four;
            x = x * RAND_SCALE_inv;
            dir.xX = x + -2.0f;
            x = (f32) rand();
            x = x * four;
            x = x * RAND_SCALE_inv;
            x = x + -2.0f;
            dir.xY = x;
            if ((u8) count & 0x100) {
                dir.xZ = four;
            }
            dir.xZ = (f32) rand() * 3.2f * RAND_SCALE_inv + -0.2f;
            scale = (f32) rand() * (0.15f - 0.1f) * RAND_SCALE_inv + 0.1f;
            AGO_vCreatePart(0x3001, &pos, dirPtr, parameter, scale, scale, hVisualMaterial);
        }
        if (count != 0) {
            goto loop30;
        }
        return;
    }

    case 33: {
        scale = (f32) rand() * (parameter + parameter) * RAND_SCALE_inv - parameter;
        pos.xX -= scale * p_stDirection->xY;
        pos.xY += scale * p_stDirection->xX;
        AGO_vCreatePart(0x3000, &pos, NULL, 2.0f, 2.0f, 2.0f, hVisualMaterial);
        return;
    }

    case 35:
        CreateType35AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 39: {
        scale = p_stDirection->xY * 0.5f;
        AGO_vCreatePart(0x8001100, &pos, NULL, 0.01f, scale, scale, hVisualMaterial);
        return;
    }

    case 42: {
        f32 x;
        f32 minusOne;
        minusOne = -1.0f;
        g_AGO_AnimTimeScale = (f32) rand() * (1.8f - 0.7f) * RAND_SCALE_inv + 0.7f;
        scale = (f32) rand() * 0.6f * RAND_SCALE_inv + 0.2f;
        pos.xX += (f32) rand() * 2.0f * RAND_SCALE_inv + minusOne;
        pos.xY += (f32) rand() * 2.0f * RAND_SCALE_inv + minusOne;
        MTH3D_M_vSetXYZ_inv(&dir, (f32) rand() * 0.4f * RAND_SCALE_inv + -0.2f,
                            (f32) rand() * 0.4f * RAND_SCALE_inv + -0.2f,
                            (f32) rand() * RAND_SCALE_inv + 1.0f);
        AGO_vCreatePart(0x103, &pos, &dir, 0.01f, scale, scale, hVisualMaterial);
        return;
    }

    case 45:
        count = 6;
        dirPtr = &dir;
        do {
            MTH3D_M_vSetXYZ_inv(&dir,
                                (f32) rand() * (2.0f * parameter - -2.0f * parameter) * RAND_SCALE_inv +
                                    -2.0f * parameter,
                                (f32) rand() * (2.0f * parameter - -2.0f * parameter) * RAND_SCALE_inv +
                                    -2.0f * parameter,
                                (f32) rand() * (6.0f * parameter - -0.2f * parameter) * RAND_SCALE_inv +
                                    -0.2f * parameter);
            scale = (f32) rand() * (0.2f * parameter - 0.1f * parameter) * RAND_SCALE_inv +
                    0.1f * parameter;
            AGO_vCreatePart(0x2501, &pos, dirPtr, parameter, scale, scale, hVisualMaterial);
            if ((u8) count & 0x100) {
                dir.xX = parameter + parameter;
            }
            count--;
        } while (count != 0);
        return;

    case 46:
        g_AGO_AnimTimeScale = 0.7f;
        AGO_vCreatePart(0x100, &pos, NULL, 0.01f, 0.5f * parameter, 0.5f * parameter, hVisualMaterial);
        pos.xX +=
            (f32) rand() * (2.0f * parameter - -2.0f * parameter) * RAND_SCALE_inv + -2.0f * parameter;
        pos.xY +=
            (f32) rand() * (2.0f * parameter - -2.0f * parameter) * RAND_SCALE_inv + -2.0f * parameter;
        g_AGO_AnimTimeScale = 1.25f;
        AGO_vCreatePart(0x100, &pos, NULL, 0.01f, 0.5f * parameter, 0.5f * parameter, hVisualMaterial);
        return;

    case 48:
        if (parameter >= 9.0f) {
            AGO_vCreatePart(0x10002400, &pos, NULL, 0.1f, 0.7f * parameter, 0.7f * parameter, hVisualMaterial);
        } else {
            AGO_vCreatePart(0x10003400, &pos, NULL, 0.1f, 0.05f * parameter, 0.05f * parameter, hVisualMaterial);
        }
        return;

    case 50:
        *(MTH3D_tdstVector_*) &AGO_gExtraParameter2 = pos;
        AGO_gExtraMaterial = (u32) hVisualMaterial;
        *(MTH3D_tdstVector_*) &AGO_gExtraParameter1 = dir;
        return;

    case 51: {
        AGO_Part_* created;
        f32* out;
        f32* in;

        created = AGO_vCreatePart(0x50005000, &pos, NULL, 0.1f, 1.0f, 1.0f, hVisualMaterial);
        if (created == NULL) {
            return;
        }
        out = (f32*) &created->stInitialPosition;
        in = (f32*) &dir;
        count = 2;
        do {
            *out = *in;
            in++;
            out++;
            count--;
        } while (count >= 0);
        return;
    }

    case 52:
        CreateType52AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 55: {
        s32 hi;
        count = (s32) dir.xY + 1;
        if (count == 0) {
            return;
        }
        scale = dir.xX;
        dirPtr = &dir;
    loop55:
        count--;
        {
            f32* p = (f32*) dirPtr;
            for (hi = 2; hi >= 0; hi--) {
                *p++ = (f32) rand() * (parameter + parameter) * RAND_SCALE_inv - parameter;
            }
        }
        part = AGO_vCreatePart(0x3003, &pos, dirPtr, 1.0f, scale, scale, hVisualMaterial);
        if (part != NULL) {
            part->ulBehaviourFlags = 1;
        }
        if (count != 0) {
            goto loop55;
        }
        return;
    }

    case 60:
        CreateType60AGO((MTH3D_tdstVector_*) p_stPosition, (MTH3D_tdstVector_*) p_stDirection, hVisualMaterial, parameter);
        return;

    case 62:
        CreateType62AGO((MTH3D_tdstVector_*) p_stPosition, (MTH3D_tdstVector_*) p_stDirection, hVisualMaterial, parameter);
        return;

    case 67:
        CreateType67AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        return;

    case 47:
    case 49:
        return;
    }

    AGO_vAddParticleContd(PartType, p_stPosition, p_stDirection, hVisualMaterial, parameter);
}

static inline s32 inv_AGO_fn_lEqualVectors(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b)
{
    return a->xX == b->xX && a->xY == b->xY && a->xZ == b->xZ;
}

void AGO_vAddParticleContd(u32 PartType, MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection,
                           void* hVisualMaterial, f32 parameter)
{
    MTH3D_tdstVector_ origin;
    MTH3D_tdstVector_ heading;
    u32 count;
    s32 i;
    s32 j;
    f32 sx;

    g_AGO_AnimTimeScale = 1.0f;
    origin = *p_stPosition;
    heading = *p_stDirection;

    switch (PartType) {
    case 0:
        break;
    case 0x44:
        count = 6;
        while (count) {
            count--;
            MTH3D_M_vSetXYZ_inv(&heading,
                                (f32) rand() * (1.0f - -1.0f) * RAND_SCALE_inv + -1.0f,
                                (f32) rand() * (1.0f - -1.0f) * RAND_SCALE_inv + -1.0f,
                                (f32) rand() * (4.0f - -0.2f) * RAND_SCALE_inv + -0.2f);
            sx = (f32) rand() * (0.12f - 0.05f) * RAND_SCALE_inv + 0.05f;
            AGO_vCreatePart(0x3005, &origin, &heading, parameter, sx, sx, hVisualMaterial);
        }
        break;
    case 0x45: {
        s32 n;
        inv_AGO_vOffsetEmitPosition(&origin, &heading);
        for (n = 0; n < 1; n++) {
            AGO_Part_* p;
            f32* p1;
            f32* p2;
            s32 k;
            p2 = &heading.xX;
            p1 = &origin.xX;
            for (k = 0; k < 3; k++) {
                *p2 = *p1 + ((f32) rand() * (1.5f - -1.5f) * RAND_SCALE_inv + -1.5f);
                p1++;
                p2++;
            }
            p = AGO_vCreatePart(0x50006000, &origin, NULL, 0.1f, 5.0f, 5.0f, hVisualMaterial);
            if (p != NULL) {
                f32* pDst;
                f32* pSrc;
                pDst = &p->stInitialPosition.xX;
                pSrc = &heading.xX;
                for (k = 0; k < 3; k++) {
                    *pDst = *pSrc;
                    pDst++;
                    pSrc++;
                }
            }
        }
        break;
    }
    case 0x47: {
        AGO_Part_* p;
        inv_AGO_vOffsetEmitPosition(&origin, &heading);
        sx = (f32) rand() * (parameter * 0.3f - parameter * 0.3f) * RAND_SCALE_inv +
             parameter * 0.3f;
        p = AGO_vCreatePart(0x6000, &origin, NULL, 0.1f, sx, sx, hVisualMaterial);
        if (p != NULL) {
            p->Transparency = 500.0f;
        }
        break;
    }
    case 0x63:
        sx = (f32) rand() * (0.8f - 0.5f) * RAND_SCALE_inv + 0.5f;
        AGO_vCreatePart(0x4500, &origin, NULL, parameter, sx, sx, hVisualMaterial);
        count = (s32) ((parameter + 1.0f) * 2.0f);
        while (count) {
            count--;
            MTH3D_M_vSetXYZ_inv(&heading,
                                (f32) rand() * (2.0f - -2.0f) * RAND_SCALE_inv + -2.0f,
                                (f32) rand() * (2.0f - -2.0f) * RAND_SCALE_inv + -2.0f,
                                (f32) rand() * (6.0f - -0.2f) * RAND_SCALE_inv + -0.2f);
            sx = (f32) rand() * (0.35f - 0.19f) * RAND_SCALE_inv + 0.19f;
            AGO_vCreatePart(0x3001, &origin, &heading, parameter, sx, sx, hVisualMaterial);
        }
        break;
    case 0x64:
        break;
    case 0x69:
        sx = heading.xX;
        count = (s32) heading.xY + 1;
        while (count) {
            count--;
            {
                f32* p2;
                p2 = &heading.xX;
                for (j = 0; j < 3; j++) {
                    *p2 = (f32) rand() * (parameter - -parameter) * RAND_SCALE_inv +
                          -parameter;
                    p2++;
                }
            }
            AGO_vCreatePart(0x3E10, &origin, &heading, 1.0f, sx, sx, hVisualMaterial);
        }
        break;
    case 0x6E: {
        AGO_Part_* p;
        p = AGO_vCreatePart(0x11, &origin, &heading, parameter, 1.0f, 1.0f, hVisualMaterial);
        if (p != NULL) {
            p->Transparency = parameter;
        }
        break;
    }
    case 0x78:
        inv_AGO_vSetGauge((f32*) &origin, (f32*) &heading, AGO_gExtraMaterial, parameter,
                      (s32) hVisualMaterial);
        break;
    case 0x7D:
        inv_AGO_vSetIcon(&origin, &heading, parameter, (s32) hVisualMaterial);
        break;
    case 0x9B:
        sx = heading.xX;
        count = (s32) heading.xY + 1;
        while (count) {
            count--;
            {
                f32* p2;
                p2 = &heading.xX;
                for (j = 0; j < 3; j++) {
                    *p2 = (f32) rand() * (parameter - -parameter) * RAND_SCALE_inv +
                          -parameter;
                    p2++;
                }
            }
            {
                AGO_Part_* p;
                p = AGO_vCreatePart(0x5003, &origin, &heading, 1.0f, sx, sx, hVisualMaterial);
                if (p != NULL) {
                    p->ulBehaviourFlags = 1;
                }
            }
        }
        break;
    case 0x9C:
        parameter += 1.0f;
        g_AGO_AnimTimeScale = 0.3333333f;
        AGO_vCreatePart(0x10000, &origin, NULL, 2.0f, parameter, parameter, hVisualMaterial);
        break;
    case 0x9E:
        inv_CreateType158AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        break;
    case 0xA3: {
        AGO_Part_* p;
        if (parameter == 2.0f) {
            for (i = 0; i < (s32) g_ulNumberOfAgos; i++) {
                p = &g_AGO[i];
                if (p->ulType == 0x60000000) {
                    if (inv_AGO_fn_lEqualVectors(&p->stInitialPosition, &origin) &&
                        inv_AGO_fn_lEqualVectors(&p->stPosition, &heading)) {
                        p->ulType |= 0x01000000;
                        return;
                    }
                    if (inv_AGO_fn_lEqualVectors(&p->stInitialPosition, &heading) &&
                        inv_AGO_fn_lEqualVectors(&p->stPosition, &origin)) {
                        p->ulType |= 0x01000000;
                        return;
                    }
                }
            }
        } else {
            for (i = 0; i < (s32) g_ulNumberOfAgos; i++) {
                p = &g_AGO[i];
                if (p->ulType == 0x60000000) {
                    if ((inv_AGO_fn_lEqualVectors(&p->stInitialPosition, &origin) &&
                         inv_AGO_fn_lEqualVectors(&p->stPosition, &heading)) ||
                        (inv_AGO_fn_lEqualVectors(&p->stInitialPosition, &heading) &&
                         inv_AGO_fn_lEqualVectors(&p->stPosition, &origin))) {
                        return;
                    }
                }
            }
            p = &g_AGO[g_ulNumberOfAgos];
            AGO_vCreatePart(0x60000000, &origin, NULL, 0.1f, 1.0f, 1.0f, hVisualMaterial);
            {
                f32* pDst;
                f32* pSrc;
                pDst = &p->stInitialPosition.xX;
                pSrc = &heading.xX;
                for (i = 0; i < 3; i++) {
                    *pDst = *pSrc;
                    pDst++;
                    pSrc++;
                }
            }
        }
        g_bUsingMapMondeAGOs = 1;
        break;
    }
    case 0xA7:
        break;
    case 0xA8 ... 0x162:
        return;
    case 0x163:
        inv_CreateType355AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        break;
    case 0x164:
        inv_CreateType356AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        break;
    case 0x18C:
        inv_CreateType396AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        break;
    case 0x1AC:
        inv_CreateType428AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        break;
    case 0x1AD:
        g_AGO_PloufMaterial = (s32) hVisualMaterial;
        inv_AGO_xType428ZLimit = p_stPosition->xZ;
        if (parameter == 0.0f) {
            g_DoSmallPlouf = 0;
        } else {
            g_DoSmallPlouf = 1;
        }
        break;
    case 0x1E0:
        inv_CreateType480AGO(p_stPosition, p_stDirection, hVisualMaterial, parameter);
        break;
    }
}

void AGO_Define_OrthogonalVectors(MTH3D_tdstVector_* p_stDirection, MTH3D_tdstVector_* p_stV1, MTH3D_tdstVector_* p_stV2)
{
    MTH3D_tdstVector_ candidate;
    f32 wLengthSquared;
    f32 candidateLengthSquared;
    f32 uLengthSquared;
    f32 ux;
    f32 uy;
    f32 uz;
    f32 crossComponent;

    candidate.xX = 0.0f;
    MTH3D_M_vSetXYZ_inv(p_stV1, -p_stDirection->xY, p_stDirection->xX, 0.0f);
    p_stV2->xX = -p_stDirection->xZ;
    *(u32*) &p_stV2->xY = 0;
    wLengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stV2->xX, p_stV2->xY, p_stDirection->xX);
    p_stV2->xZ = p_stDirection->xX;
    candidate.xY = -p_stDirection->xZ;
    candidate.xZ = p_stDirection->xY;
    candidateLengthSquared =
        MTH_fn_xSqrAddSqrAddSqrOpt2(p_stV2->xY, candidate.xY, candidate.xZ);
    uLengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stV1->xX, p_stV1->xY, p_stV1->xZ);
    if (uLengthSquared < candidateLengthSquared) {
        *(MTH3D_tdstVector_*) p_stV1 = candidate;
        uLengthSquared = candidateLengthSquared;
    }
    if (uLengthSquared < wLengthSquared) {
        *p_stV1 = *p_stV2;
        uLengthSquared = wLengthSquared;
    }
    ux = p_stV1->xX;
    uy = p_stV1->xY;
    uz = p_stV1->xZ;
    uLengthSquared = inv_MTH_fn_xDivideBySqrtTied(1.0f, uLengthSquared);
    MTH3D_M_vSetXYZ_inv(p_stV1, ux * uLengthSquared, uy * uLengthSquared, uz * uLengthSquared);
    crossComponent =
        inv_MTH_fn_xMulSubTied(p_stDirection->xY, p_stV1->xZ, p_stDirection->xZ, p_stV1->xY);
    p_stV2->xX = crossComponent;
    crossComponent =
        inv_MTH_fn_xMulSubTied(p_stDirection->xZ, p_stV1->xX, p_stDirection->xX, p_stV1->xZ);
    p_stV2->xY = crossComponent;
    crossComponent =
        inv_MTH_fn_xMulSubTied(p_stDirection->xX, p_stV1->xY, p_stDirection->xY, p_stV1->xX);
    p_stV2->xZ = crossComponent;
}

void inv_strcpy(u8* dst, u8* src)
{
    int index = 0;
    if (src[0])
        do {
            dst[index] = src[index];
            index++;
        } while (src[index]);
    dst[index] = 0;
}

void inv_strcpy2(u8* dst, s8* src)
{
    int index = 0;
    if (src[0])
        do {
            dst[index] = (u8) src[index];
            index++;
        } while (src[index]);
    dst[index] = 0;
}

void inv_strcpy3(u8* dst, const u8* src)
{
    int index = 0;
    if (src[0])
        do {
            dst[index] = src[index];
            index++;
        } while (src[index]);
    dst[index] = 0;
}

int inv_strlen(u8* string)
{
    int index = 0;
    if (string[0])
        do {
            index++;
        } while (string[index]);
    return index;
}

s32 inv_atoi(const u8* string)
{
    u8 buffer[0x260];
    inv_strcpy3(buffer, string);
    return atoi(buffer);
}

void TXT_vInit(void)
{
    _JFFTXT_ *entry = g_JFFTXT, *end = g_JFFTXT + 40;
    do {
        entry->pString = 0;
        entry->ucTransparency = 0;
        entry->bFrozenDisplay = 0;
        entry->pulse = 0;
        entry->bOther = 0;
        entry->EffectParameter = 0;
        entry++;
    } while ((s32) entry < (s32) end);
}

s32 inv_TXT_fn_lGetFreeScriptStringNumber(void)
{
    _JFFTXT_* base;
    _JFFTXT_* entry;
    s32 i;
    base = g_JFFTXT;
    i = 0x14;
    entry = &base[0x14];
    do {
        if (entry->pString == 0)
            return i;
        ++i;
        entry++;
    } while (i < 0x28);
    return -1;
}

void inv_TXT_fn_vHideScriptStrings(void)
{
    u8 value = 1;
    u8* cursor = (u8*) g_JFFTXT;
    int remaining;
    cursor += 0x380;
    remaining = 19;
    do {
        *cursor = value;
        remaining--;
        cursor += 44;
    } while (remaining >= 0);
}

void inv_TXT_fn_vShowScriptStrings(void)
{
    u8 value = 255;
    u8* cursor = (u8*) g_JFFTXT;
    int remaining;
    cursor += 0x380;
    remaining = 19;
    do {
        *cursor = value;
        remaining--;
        cursor += 44;
    } while (remaining >= 0);
}

void inv_TXT_fn_vComputeCharMetrics(s32* output0, s32* output1, s32* output2, s32* output3, f32 xValue)
{
    f32 xValue1;
    f32 xValue3;
    f32 xValue02;
    s32 converted;

    xValue1 = xValue - 15.0f;
    xValue3 = xValue1 * 2.5f;
    xValue1 = xValue1 + xValue1;
    xValue3 = xValue3 + 38.0f;
    xValue02 = xValue1 + 25.0f;
    xValue1 = xValue1 + xValue1;
    *output3 = (s32) xValue3;
    converted = (s32) xValue02;
    xValue1 = xValue1 + 46.0f;
    *output0 = converted;
    *output2 = converted;
    *output1 = (s32) xValue1;
}

extern void inv_TXT_fn_vComputeCharMetrics(s32* a0, s32* a1, s32* a2, s32* a3, f32 in);

void JFFTXT_vAddText(s32 index, MTH3D_tdstVector_* src, u8* string, f32 scale)
{
    _JFFTXT_* entry;
    f32 tail;

    if (index < 40) {
        entry = &g_JFFTXT[index];
        entry->pString = string;
        entry->ucTransparency = (u8) (s32) scale;
        entry->bFrozenDisplay = 0;
        entry->a3_Pos[0] = src->xX;
        entry->a3_Pos[1] = src->xY;
        tail = src->xZ;
        entry->pulse = 0;
        entry->a3_Pos[2] = tail;
        entry->bOther = 0;
        inv_TXT_fn_vComputeCharMetrics(&entry->advance, &entry->lineHeight, &entry->cellWidth, &entry->cellHeight, tail);
    }
}

void JFFTXT_vExtendText(s32 index, s32 command, f32 argument)
{
    _JFFTXT_* entry;
    u8 flags;
    s32 value;

    if (index >= 0x28) {
        return;
    }
    entry = &g_JFFTXT[index];
    if ((u32) command >= 8) {
        return;
    }
    switch (command) {
    case 0:
        entry->ucTransparency = (u8) (s32) argument;
        break;
    case 1:
        entry->pulse = (u8) (((s32) argument & 0xFF) != 0);
        break;
    case 2:
        entry->bOther = 1;
        if (entry->ucTransparency == 0) {
            entry->ucTransparency = 1;
        }
        break;
    case 3:
        entry->bOther = 2;
        break;
    case 4:
        entry->a3_Pos[2] = argument;
        break;
    case 5:
        if ((s32) argument == 0) {
            flags = entry->bOther;
            entry->bOther = flags & 0xFB;
        } else {
            entry->EffectParameter = (s32) argument;
            flags = entry->bOther;
            entry->bOther = flags | 4;
        }
        break;
    case 6:
        flags = entry->bOther;
        if (argument == 1.0f) {
            entry->bOther = flags | 8;
        } else {
            entry->bOther = flags & 0xF7;
        }
        break;
    case 7:
        value = (s32) argument;
        if ((u32) value >= 5) {
            return;
        }
        switch (value) {
        case 0:
            flags = entry->bOther;
            entry->ucForcedColor = 0;
            entry->bOther = flags & 0xEF;
            break;
        case 1:
            flags = entry->bOther;
            value = 1;
            goto set_type;
        case 2:
            flags = entry->bOther;
            value = 2;
            goto set_type;
        case 3:
            flags = entry->bOther;
            value = 3;
            goto set_type;
        case 4:
            flags = entry->bOther;
            value = 4;
        set_type:
            *(volatile u8*) &entry->ucForcedColor = (u8) value;
            *(volatile u8*) &entry->bOther = flags | 0x10;
            break;
        }
        break;
    }
}

void JFFTXT_vModifyCaracter(int index, int offset, int value)
{
    if (index < 40) {
        u8* data = g_JFFTXT[index].pString;
        if (data)
            data[offset] = (u8) value;
    }
}

void TXT_vAffiche(void)
{
    s32 index;
    _JFFTXT_* record;
    s32 flags;
    u8 work[0x20];

    index = 0;
    while (index < 0x28) {
        record = &g_JFFTXT[index];
        if (record->ucTransparency != 0 && record->pString != 0 && record->pString[0] != 0) {
            flags = record->bOther;
            if ((flags & 1) != 0) {
                s32 up;

                up = (s32) record->ucTransparency + (s32) (g_fTimeDifference * 200.0f);
                if (up >= 0xFF) {
                    record->bOther = (u8) (flags & 0xFE);
                    flags &= 0xFE;
                    up = 0xFF;
                }
                record->ucTransparency = (u8) up;
            } else if ((flags & 2) != 0) {
                s32 down;

                down = (s32) record->ucTransparency - (s32) (g_fTimeDifference * 200.0f);
                if (down <= 0) {
                    record->bOther = (u8) (flags & 0xFD);
                    flags &= 0xFD;
                    down = 0;
                }
                record->ucTransparency = (u8) down;
            }
            if ((flags & 8) != 0) {
                JFFTXT_vGetStringExtents(record, work, work + 0x10);
                GLI_vDisplayFrame((MTH3D_tdstVector_*) work, (MTH3D_tdstVector_*) (work + 0x10), (f32) record->ucTransparency * 0.7f, 0.0f, &unk_2524F8);
            }
            JFFTXT_vDrawString(record);
        }
        index++;
    }
    inv_GLI_cTextGradientMode = 0;
}

s32 inv_toupper(s32 character)
{
    s32 limit = 0x19;
    s32 index = character - 0x61;
    s32 result = character - 0x20;
    if ((u32) limit < (u32) index) {
        result = character;
    }
    return result;
}

s32 inv_TXT_fn_lGetStringPixelWidth(u8* str, s32 advance)
{
    s32 total;
    s32 index;
    u32 character;
    f32 scale;

    total = 0;
    index = 0;
    character = str[0];
    if (character != 0) {
        scale = 0.6f;
        do {
            if (character == '/') {
                index++;
                character = inv_toupper(str[index]) & 0xFF;
                if (character == 'L') {
                    break;
                }
                if (character != ':') {
                    do {
                        index++;
                        character = str[index];
                    } while (character != ':');
                }
            } else if (character == ' ') {
                total += (s32) ((f32) advance * scale);
            } else {
                total += advance;
            }
            index++;
            character = str[index];
        } while (character != 0);
    }
    return total;
}

void JFFTXT_vGetStringExtents(_JFFTXT_* record, void* work, void* result)
{
    typedef struct JffExtent {
        f32 x;
        f32 y;
    } JffExtent;

    _JFFTXT_* rec;
    JffExtent* min;
    JffExtent* max;
    s32 firstCenter;
    s32 center;
    s32 i;
    u8* str;
    u32 c;
    f32 x;
    f32 y;
    f32 right;

    rec = record;
    min = work;
    max = result;
    firstCenter = 1;
    center = 0;
    i = 0;
    x = rec->a3_Pos[0];
    str = rec->pString;
    y = rec->a3_Pos[1];

    min->x = x;
    max->x = x;
    min->y = y;
    max->y = y;

    c = inv_toupper(str[0]) & 0xFF;
    if (c != 0) {
        do {
            if (c == '/') {
                i++;
                c = inv_toupper(str[i]) & 0xFF;
                if (c == 0) {
                    return;
                }

                if (c == 'L') {
                    i++;
                    c = str[i];
                    if (c == 0) {
                        return;
                    }
                    if (c != ':') {
                        return;
                    }
                    if (center != 0) {
                        i++;
                        x = (f32) ((1000 - inv_TXT_fn_lGetStringPixelWidth(str - -i, rec->advance)) >> 1);
                    } else {
                        x = rec->a3_Pos[0];
                        i++;
                    }
                    if (x < min->x) {
                        min->x = x;
                    }
                    y += (f32) rec->lineHeight;
                    if (max->y < y) {
                        max->y = y;
                    }
                load_right:
                    right = max->x;
                    goto update_right;
                }

                if (c == 'O') {
                    i++;
                    c = str[i];
                    if (c == 0) {
                        return;
                    }
                    if (c == ':') {
                        goto o_colon;
                    }
                    i++;
                    do {
                        c = str[i];
                        if (c == 0) {
                            return;
                        }
                        i++;
                    } while (c != ':');
                    goto load_right;
                }

                i++;
                if (c != 'C') {
                    goto load_right;
                }
                c = str[i];
                if (c == 0) {
                    return;
                }
                center ^= 1;
                if (c != ':') {
                    return;
                }
                if (center == 0) {
                    goto c_off_load;
                }
                i++;
                if (firstCenter != 0) {
                    {
                        u32 z;
                        z = 0x43FA0000;
                        min->x = *(f32*) &z;
                        max->x = *(f32*) &z;
                    }
                    firstCenter = 0;
                }
                x = (f32) ((1000 - inv_TXT_fn_lGetStringPixelWidth(str - -i, rec->advance)) >> 1);
                if (x < min->x) {
                    min->x = x;
                }
                right = max->x;
                if (right < x) {
                    max->x = x;
                    right = x;
                }
                goto update_right;
            } else {
                i++;
                if (c == ' ') {
                    right = max->x;
                    x += (f32) (s32) ((f32) rec->advance * 0.6f);
                } else {
                    right = max->x;
                    x += (f32) rec->advance;
                }
                goto update_right;
            }

        o_colon:
            i++;
            goto load_right;

        c_off_load:
            right = max->x;
        c_off:
            i++;

        update_right:
            if (right < x) {
                max->x = x;
            }
            c = inv_toupper(str[i]) & 0xFF;
        } while (c != 0);
    }
    min->x = (min->x - 10.0f) * 0.1f;
    min->y = (min->y - 10.0f) * 0.1f;
    max->x = (max->x + 10.0f) * 0.1f;
    max->y = (max->y + (f32) rec->lineHeight + 10.0f) * 0.1f;
}

void JFFTXT_vDrawString(_JFFTXT_* rec)
{
    char buf[16];
    s32 centerFlag;
    s32 count;
    u32 c;
    s32 i;
    s32 alt;
    u32 color;
    void* glyph;
    u8* str;
    u32 num;
    f32 x;
    f32 y;
    f32 pulse;
    f32 top;
    s32 n;
    s32 glyphIdx;
#ifdef PC_PORT
    f32 narrow = inv_AGO_fn_xHudNarrow();
#endif

    centerFlag = 0;
    color = 0;
    count = 1;
    num = 0;
    i = 0;
    (*&inv_GLI_cTextGradientMode) = 1;
    pulse = 0.0f;
    str = rec->pString;

    if (rec->bOther & 0x10) {
        switch (rec->ucForcedColor) {
        case 2:
            color = 0xFFFA4646;
            break;
        case 1:
            color = 0xFF6EBEFF;
            break;
        case 3:
            color = 0xFFFFFF78;
            break;
        case 4:
            color = 0xFF240F06;
            break;
        }
    } else {
        color = 0xFF6EBEFF;
    }

    if (rec->pulse != 0) {
        pulse = sinf(g_ffFrameTime * 5.0f) * 7.0f;
    }

#ifdef PC_PORT
    x = inv_AGO_fn_xHudPlace(rec->a3_Pos[0], 1000.0f, narrow);
#else
    x = rec->a3_Pos[0];
#endif
    y = rec->a3_Pos[1];
    GLI_vSetGlobalAlpha((f32) rec->ucTransparency);

    c = inv_toupper(str[0]);
    if (c == 0) {
        return;
    }

    do {
        if (c == 0x2F) {
            i++;
            c = inv_toupper(str[i]);
            if (c == 0) {
                return;
            }
            if (c == 0x4C) {
                i++;
                c = str[i];
                if (c == 0) {
                    return;
                }
                if (c != 0x3A) {
                    return;
                }
                if (centerFlag != 0) {
#ifdef PC_PORT
                    x = (1000.0f - (f32) inv_TXT_fn_lGetStringPixelWidth(&str[i + 1], rec->advance) * narrow) * 0.5f;
#else
                    x = (f32) ((1000 - inv_TXT_fn_lGetStringPixelWidth(&str[i + 1], rec->advance)) >> 1);
#endif
                } else {
#ifdef PC_PORT
                    x = inv_AGO_fn_xHudPlace(rec->a3_Pos[0], 1000.0f, narrow);
#else
                    x = rec->a3_Pos[0];
#endif
                }
                y += (f32) rec->lineHeight;
            } else if (c == 0x4F) {
                i++;
                c = str[i];
                num = 0;
                if (c == 0) {
                    return;
                }
                if (c != 0x3A) {
                    do {
                        num *= 10;
                        num += c - 0x30;
                        i++;
                        c = str[i];
                        if (c == 0) {
                            return;
                        }
                    } while (c != 0x3A);
                }
                if (rec->bOther & 0x10) {
                    switch (rec->ucForcedColor) {
                    case 2:
                        color = 0xFFFA4646;
                        break;
                    case 1:
                        color = 0xFF6EBEFF;
                        break;
                    case 3:
                        color = 0xFFFFFF78;
                        break;
                    case 4:
                        color = 0xFF240F06;
                        break;
                    }
                } else if (num == 0) {
                    color = 0xFF6EBEFF;
                } else if (num == 600) {
                    color = 0xFF4DFF19;
                } else if (num == 200) {
                    color = 0xFFFA4646;
                } else if (num == 400) {
                    color = 0xFFFFFF78;
                }
            } else if (c == 0x43) {
                i++;
                c = str[i];
                if (c == 0) {
                    return;
                }
                if (c != 0x3A) {
                    return;
                }
                centerFlag ^= 1;
                if (centerFlag != 0) {
#ifdef PC_PORT
                    x = (1000.0f - (f32) inv_TXT_fn_lGetStringPixelWidth(&str[i + 1], rec->advance) * narrow) * 0.5f;
#else
                    x = (f32) ((1000 - inv_TXT_fn_lGetStringPixelWidth(&str[i + 1], rec->advance)) >> 1);
#endif
                }
            } else if (c == 0x45) {
                i++;
                c = str[i];
                if (c == 0) {
                    return;
                }
                buf[0] = c;
                n = 1;
                if (c != 0x3A) {
                    do {
                        i++;
                        c = str[i];
                        if (c == 0) {
                            return;
                        }
                        buf[n] = c;
                        n++;
                    } while (c != 0x3A);
                }
                buf[n - 1] = 0;
                if (strcmp(buf, "110") == 0) {
                    return;
                }
            } else if (c == 0x50 || c == 0x44 || c == 0x5A || c == 0x57 || c == 0x48) {
                i++;
                c = str[i];
                if (c == 0) {
                    return;
                }
                if (c != 0x3A) {
                    do {
                        i++;
                        c = str[i];
                        if (c == 0) {
                            return;
                        }
                    } while (c != 0x3A);
                }
            }
        } else if (c == 0x20) {
#ifdef PC_PORT
            x += (f32) (s32) ((f32) rec->advance * 0.6f) * narrow;
#else
            x += (f32) (s32) ((f32) rec->advance * 0.6f);
#endif
            count++;
        } else {
            glyphIdx = c < 0xA1 ? c : c - 0x20;
            alt = num < 0x320;
            glyphIdx -= 0x20;
            if (!alt) {
                glyphIdx += num;
                glyph = GLI_pGetVisualMaterialForLetter(glyphIdx);
            } else {
                glyph = GLI_pGetVisualMaterialForLetter(glyphIdx);
            }
            if (glyph != NULL) {
                if (!alt) {
                    (*&inv_GLI_cTextGradientMode) = 0;
                } else {
                    (*&inv_GLI_cTextGradientMode) = color == 0xFF240F06 ? 2 : 1;
                }
                if (rec->bOther & 4) {
                    if (count == rec->EffectParameter) {
                        GLI_vSetGlobalAlpha((f32) (rec->ucTransparency >> 1));
                        pulse = sinf(g_ffFrameTime * 5.0f) * 7.0f;
                    } else {
                        pulse = 0.0f;
                        GLI_vSetGlobalAlpha((f32) rec->ucTransparency);
                    }
                }
                top = y;
                if (c == 0x2E) {
                    top += (f32) (rec->cellHeight >> 2);
                } else if (c == 0x2C) {
                    top += (f32) (rec->cellHeight >> 1);
                } else if (c == 0x27) {
                    top -= (f32) (rec->cellHeight >> 2);
                }
                inv_GLI_ucTextGradientRed = color >> 16;
                inv_GLI_ucTextGradientGreen = color >> 8;
                inv_GLI_ucTextGradientBlue = color;
#ifdef PC_PORT
                inv_RW3_fn_vDrawScreenSprite(dword_252570, x * 0.1f, (top - pulse) * 0.1f, (x + (f32) rec->cellWidth * narrow) * 0.1f,
                              (top + (f32) rec->cellHeight + pulse) * 0.1f, glyph);
#else
                inv_RW3_fn_vDrawScreenSprite(dword_252570, x * 0.1f, (top - pulse) * 0.1f, (x + (f32) rec->cellWidth) * 0.1f,
                              (top + (f32) rec->cellHeight + pulse) * 0.1f, glyph);
#endif
                (*&inv_GLI_cTextGradientMode) = 1;
            }
#ifdef PC_PORT
            x += (f32) rec->advance * narrow;
#else
            x += (f32) rec->advance;
#endif
            count++;
        }
        i++;
        c = inv_toupper(str[i]);
    } while (c != 0);
}
#ifdef PC_PORT
enum {
    JFFTXT_WRAP_WORDS = 160,
    JFFTXT_WRAP_LINES = 8,
    JFFTXT_WRAP_SIZE = 600,
};

typedef struct JFFTXT_tdstWrapWord_ {
    s16 start;
    s16 end;
    s16 gapStart;
    f32 width;
    f32 gap;
} JFFTXT_tdstWrapWord_;

static JFFTXT_tdstWrapWord_ inv_JFFTXT_a_stWrapWord[JFFTXT_WRAP_WORDS];
static f32 inv_JFFTXT_a_xWrapCost[JFFTXT_WRAP_LINES + 1][JFFTXT_WRAP_WORDS + 1];
static s16 inv_JFFTXT_a_wWrapFrom[JFFTXT_WRAP_LINES + 1][JFFTXT_WRAP_WORDS + 1];
static u8 inv_JFFTXT_a_ucWrapFlat[JFFTXT_WRAP_SIZE];
static u8 inv_JFFTXT_a_ucWrapOut[JFFTXT_WRAP_SIZE + 3 * JFFTXT_WRAP_LINES];

static s32 inv_JFFTXT_fn_lCodeLength(const u8* s)
{
    s32 n = 1;

    while (s[n] != 0 && s[n] != ':') {
        n++;
    }
    return s[n] == ':' ? n + 1 : n;
}

static s32 inv_JFFTXT_fn_bIsLineBreak(const u8* s)
{
    return s[0] == '/' && (s[1] == 'L' || s[1] == 'l') && s[2] == ':';
}

static s32 inv_AGO_fn_bStretchedVeilAt(f32 y, f32* left, f32* right)
{
    u32 i;

    for (i = 0; i < g_ulNumberOfAgos; i++) {
        AGO_Part_* p = &g_AGO[i];
        u32 type = p->ulType & 0xFF;

        if ((type == 0x11 || type == 0x15) && p->VisualMaterial != NULL && p->stDirection.xX >= 90.0f &&
            p->stDirection.xX != 100.0f && p->stPosition.xY <= y && y <= p->stDirection.xY) {
            *left = p->stPosition.xX;
            *right = p->stDirection.xX;
            return 1;
        }
    }
    return 0;
}

static f32 inv_JFFTXT_fn_xWrapWidth(const f32* sum, s32 a, s32 b)
{
    return sum[b] - sum[a] - inv_JFFTXT_a_stWrapWord[a].gap;
}

void inv_JFFTXT_fn_vWrapForWideBox(u8* text, s32 size, MTH3D_tdstVector_* pos)
{
    static f32 sum[JFFTXT_WRAP_WORDS + 1];
    f32 narrow = inv_AGO_fn_xHudNarrow();
    f32 veilLeft;
    f32 veilRight;
    f32 start;
    f32 margin;
    f32 room;
    f32 space;
    f32 width;
    s32 advance;
    s32 lineHeight;
    s32 cellWidth;
    s32 cellHeight;
    s32 written;
    s32 glyphs;
    s32 length;
    s32 count;
    s32 lines;
    s32 words;
    s32 a;
    s32 b;
    s32 l;
    s32 n;
    u8* p;
    u8* flat = inv_JFFTXT_a_ucWrapFlat;
    u8* out = inv_JFFTXT_a_ucWrapOut;
    JFFTXT_tdstWrapWord_* word = inv_JFFTXT_a_stWrapWord;
    s16 breakAfter[JFFTXT_WRAP_LINES];

    if (narrow == 1.0f || size > JFFTXT_WRAP_SIZE) {
        return;
    }

    written = 1;
    glyphs = 0;
    for (p = text; *p != 0;) {
        if (inv_JFFTXT_fn_bIsLineBreak(p)) {
            if (glyphs == 0) {
                return;
            }
            written++;
            glyphs = 0;
            p += 3;
        } else if (*p == '/') {
            if (p[1] == 'C' || p[1] == 'c') {
                return;
            }
            p += inv_JFFTXT_fn_lCodeLength(p);
        } else {
            glyphs += *p != ' ';
            p++;
        }
    }
    length = p - text;
    if (written == 1 || glyphs == 0 || written > JFFTXT_WRAP_LINES) {
        return;
    }

    if (!inv_AGO_fn_bStretchedVeilAt(pos->xY * 0.1f, &veilLeft, &veilRight)) {
        return;
    }
    start = inv_AGO_fn_xHudPlace(pos->xX, 1000.0f, narrow);
    margin = start - veilLeft * narrow * 10.0f;
    if (margin < 0.0f) {
        margin = 0.0f;
    }
    room = ((90.0f - (90.0f - veilRight) / narrow) * 10.0f - margin - start) / narrow;
    inv_TXT_fn_vComputeCharMetrics(&advance, &lineHeight, &cellWidth, &cellHeight, pos->xZ);
    space = (f32) (s32) ((f32) advance * 0.6f);

    n = 0;
    for (p = text; *p != 0;) {
        if (inv_JFFTXT_fn_bIsLineBreak(p)) {
            while (n > 0 && flat[n - 1] == ' ') {
                n--;
            }
            flat[n++] = ' ';
            for (p += 3; *p == ' '; p++) {
            }
        } else if (*p == '/') {
            for (count = inv_JFFTXT_fn_lCodeLength(p); count > 0; count--) {
                flat[n++] = *p++;
            }
        } else {
            flat[n++] = *p++;
        }
    }
    flat[n] = 0;

    words = 0;
    for (a = 0; flat[a] != 0;) {
        if (words == JFFTXT_WRAP_WORDS) {
            return;
        }
        word[words].gapStart = a;
        word[words].gap = 0.0f;
        for (; flat[a] == ' '; a++) {
            word[words].gap += space;
        }
        if (flat[a] == 0) {
            break;
        }
        word[words].start = a;
        word[words].width = 0.0f;
        while (flat[a] != 0 && flat[a] != ' ') {
            if (flat[a] == '/') {
                a += inv_JFFTXT_fn_lCodeLength(flat + a);
            } else {
                word[words].width += (f32) advance;
                a++;
            }
        }
        word[words].end = a;
        words++;
    }
    if (words < 2) {
        return;
    }
    sum[0] = 0.0f;
    for (b = 0; b < words; b++) {
        sum[b + 1] = sum[b] + word[b].gap + word[b].width;
    }

    lines = 1;
    a = 0;
    for (b = 1; b < words; b++) {
        if (inv_JFFTXT_fn_xWrapWidth(sum, a, b + 1) > room) {
            lines++;
            a = b;
        }
    }
    if (lines >= written) {
        return;
    }

    for (l = 0; l <= lines; l++) {
        for (b = 0; b <= words; b++) {
            inv_JFFTXT_a_xWrapCost[l][b] = -1.0f;
        }
    }
    inv_JFFTXT_a_xWrapCost[0][0] = 0.0f;
    for (l = 1; l <= lines; l++) {
        for (b = l; b <= words; b++) {
            for (a = l - 1; a < b; a++) {
                f32 left;
                f32 cost;

                if (inv_JFFTXT_a_xWrapCost[l - 1][a] < 0.0f) {
                    continue;
                }
                width = inv_JFFTXT_fn_xWrapWidth(sum, a, b);
                if (width > room && b - a > 1) {
                    continue;
                }
                left = room - width;
                cost = inv_JFFTXT_a_xWrapCost[l - 1][a] + left * left;
                if (inv_JFFTXT_a_xWrapCost[l][b] < 0.0f || cost < inv_JFFTXT_a_xWrapCost[l][b]) {
                    inv_JFFTXT_a_xWrapCost[l][b] = cost;
                    inv_JFFTXT_a_wWrapFrom[l][b] = a;
                }
            }
        }
    }
    if (inv_JFFTXT_a_xWrapCost[lines][words] < 0.0f) {
        return;
    }
    b = words;
    for (l = lines; l > 0; l--) {
        b = inv_JFFTXT_a_wWrapFrom[l][b];
        breakAfter[l - 1] = b;
    }

    n = 0;
    l = 1;
    for (b = 0; b < words; b++) {
        if (l < lines && b == breakAfter[l]) {
            out[n++] = '/';
            out[n++] = 'L';
            out[n++] = ':';
            l++;
        } else {
            for (a = word[b].gapStart; a < word[b].start; a++) {
                out[n++] = flat[a];
            }
        }
        for (a = word[b].start; a < word[b].end; a++) {
            out[n++] = flat[a];
        }
        if (n > length) {
            return;
        }
    }
    for (a = word[words - 1].end; flat[a] != 0 && n < length; a++) {
        out[n++] = flat[a];
    }
    if (n > length || n >= size) {
        return;
    }
    out[n] = 0;
    memcpy(text, out, n + 1);
}
#endif

void inv_TXT_fn_vDrawStringWithTransparency(MTH3D_tdstVector_* src, const char* text, f32 scale)
{
    _JFFTXT_ record;
    f32 tail;

    record.pString = (u8*) text;
    record.ucTransparency = (u8) (s32) scale;
    record.bFrozenDisplay = 0;
    record.a3_Pos[0] = src->xX;
    record.a3_Pos[1] = src->xY;
    tail = src->xZ;
    record.a3_Pos[2] = tail;
    record.pulse = 0;
    record.bOther = 0;
    inv_TXT_fn_vComputeCharMetrics(&record.advance, &record.lineHeight, &record.cellWidth, &record.cellHeight, tail);
    JFFTXT_vDrawString(&record);
}
