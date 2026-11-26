#include "common.h"

#include "HIE/HIE.h"
#include "COL/COL.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"
#include "ps2/r5900.h"

#include "include_asm.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "GEO/GEO.h"

extern f32 D_00392B34;
extern f32 flt_3934D0[];
f32 sqrtf(f32 value);
extern f32 D_00392B38;

extern int rand(void);

static inline f32 inv_MTH3D_fn_xRandomSpan(f32 span, f32 lo)
{
    return (f32) rand() * span * RAND_SCALE_inv + lo;
}

static inline f32 inv_MTH3D_fn_xRandomInRange(f32 lo, f32 hi)
{
    return inv_MTH3D_fn_xRandomSpan(hi - lo, lo);
}

static __inline__ f32 inv_MTH3D_fn_xVectorLength(f32 x, f32 y, f32 z)
{
    f32 len_sq;
    f32 len;
    len_sq = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    len = MTH_fn_xSqrtOpt2(len_sq);
    return len;
}

extern f32 D_00392B1C;
extern f32 D_00392B20;
extern f32 D_00392B24;
extern f32 D_00392B28;
extern f32 D_00392B2C;
extern f32 D_00392B30;
extern f32 D_00393F28;
extern f32 flt_3934C8[];
extern f32 flt_3934D4[];

f32 acosf(f32);
f32 cosf(f32);
f32 powf(f32, f32);
s32 inv_MEC_fn_bMovesTowardPlane(EffectParticle_inv* particle, MTH3D_tdstVector_* planeNormal, f32 time);
void inv_MEC_fn_vInitParticleSpin(EffectParticle_inv* dst, EffectSystem_inv* src);
void inv_MEC_fn_vEmitFromPoint(EffectSystem_inv*, MTH3D_tdstVector_*, MTH3D_tdstVector_*);
void inv_MEC_fn_vEmitFromRectangle(EffectSystem_inv*, MTH3D_tdstVector_*, MTH3D_tdstVector_*);
void inv_MEC_fn_vEmitFromSphere(EffectSystem_inv*, MTH3D_tdstVector_*, MTH3D_tdstVector_*);
void inv_MEC_fn_vUpdateEffectSystemsActivity(struct GLD_tdstViewportAttributes_* p_stVpt);
void inv_MEC_fn_vUpdateEffectSystem(EffectSystem_inv* system, f32 dt);
void inv_MEC_fn_vRenderRuntimeObject(EffectSystem_inv* owner, struct GLD_tdstViewportAttributes_* p_stVpt,
                                     s32 lDrawModeMask);

void inv_MEC_fn_vVectorToSpherical(MTH3D_tdstVector_* p_stVector, f32* p_xElevation, f32* p_xAzimuth,
                   f32* p_xLength)
{
    MTH3D_tdstVector_ horizontal;
    f32 zero;
    f32 length;

    MTH3D_M_vSetXYZ_inv(&horizontal, p_stVector->xX, 0.0f, p_stVector->xZ);
    length = inv_MTH3D_fn_xNormXYZ(&horizontal);
    *p_xLength = inv_MTH3D_fn_xNormXYZ(p_stVector);
    zero = horizontal.xY;
    if (length <= zero) {
        f32 vertical;
        *p_xAzimuth = zero;
        vertical = (-MTH_HALF_PI_inv);
        if (zero < p_stVector->xY) {
            vertical = MTH_HALF_PI_inv;
        }
        *p_xElevation = vertical;
    } else {
        f32 azimuth;
        f32 elevation;
        azimuth = acosf((horizontal.xX / length));
        if (p_stVector->xZ < zero) {
            azimuth = (-azimuth);
        }
        *p_xAzimuth = azimuth;
        elevation = acosf((length / *p_xLength));
        if (p_stVector->xY < zero) {
            elevation = (-elevation);
        }
        *p_xElevation = elevation;
    }
}

extern f32 cosf(f32 x);

void inv_MEC_fn_vSphericalToVector(f32* dst, f32 angle1, f32 angle2, f32 radius)
{
    f32 scale1;
    f32 period1;
    f32 xTurns1;
    f32 xWholeTurns1;
    f32 reduced1;
    f32 cosine1;
    f32 sign1;
    f32 zero1;
    f32 root1;
    f32 sine1;
    u32 bits1;
    s32 ni1;

    f32 scale2;
    f32 period2;
    f32 xTurns2;
    f32 xWholeTurns2;
    f32 reduced2;
    f32 cosine2;
    f32 sign2;
    f32 zero2;
    f32 root2;
    f32 sine2;
    u32 bits2;
    s32 ni2;

    f32 base;

    scale1 = MTH_INV_TWO_PI_inv;
    period1 = MTH_TWO_PI_inv;
    xTurns1 = angle1 * scale1;
    ni1 = inv_MTH_fn_lToInteger(xTurns1);
    xWholeTurns1 = (f32) ni1;
    xWholeTurns1 = xWholeTurns1 * period1;
    reduced1 = angle1 - xWholeTurns1;
    cosine1 = cosf(angle1);

    if (__builtin_fabsf(reduced1) < MTH_PI_inv) {
        sign1 = inv_MTH_fn_xSign(reduced1);
    } else {
        sign1 = -inv_MTH_fn_xSign(reduced1);
    }

    root1 = 1.0f - MTH_fn_xSqrOpt1(cosine1);
    root1 = MTH_fn_xSqrtOpt2(root1);
    sine1 = root1 * sign1;

    scale2 = MTH_INV_TWO_PI_inv;
    period2 = MTH_TWO_PI_inv;
    xTurns2 = angle2 * scale2;
    ni2 = inv_MTH_fn_lToInteger(xTurns2);
    xWholeTurns2 = (f32) ni2;
    xWholeTurns2 = xWholeTurns2 * period2;
    reduced2 = angle2 - xWholeTurns2;
    cosine2 = cosf(angle2);

    if (__builtin_fabsf(reduced2) < MTH_PI_inv) {
        sign2 = inv_MTH_fn_xSign(reduced2);
    } else {
        sign2 = -inv_MTH_fn_xSign(reduced2);
    }

    root2 = 1.0f - MTH_fn_xSqrOpt1(cosine2);
    root2 = MTH_fn_xSqrtOpt2(root2);
    sine2 = root2 * sign2;

    base = radius * cosine1;
    dst[1] = sine1 * radius;
    dst[0] = cosine2 * base;
    dst[2] = sine2 * base;
}

void inv_MEC_fn_vVectorCrossProduct(EffectParticle_inv* particle)
{
    EffectOrientation_inv* orientation;
    f32 x;
    f32 y;
    f32 z;

    orientation = particle->orientation;

    z = orientation->u.stRotation.xZ;
    y = orientation->u.stRotation.xY;
    x = orientation->u.stRotation.xX;

    MTH3D_M_vSetXYZ_inv(&orientation->rotationSpeed.stCol_0, 0.0f, -z, y);
    MTH3D_M_vSetXYZ_inv(&orientation->rotationSpeed.stCol_1, z, 0.0f, -x);
    MTH3D_M_vSetXYZ_inv(&orientation->rotationSpeed.stCol_2, -y, x, 0.0f);
}

void inv_MEC_fn_vInitParticleSpin(EffectParticle_inv* dst, EffectSystem_inv* src)
{
    f32 xAngle1;
    f32 xAngle2;
    f32 xRadius;
    EffectOrientation_inv* spin;
    EffectOrientation_inv* orientation;

    if (*(u8*) &src->flags & 1) {
        f32 base;
        f32 top;
        f32 rand_scale;

        base = src->spinBase;
        top = (base + src->spinRange) - base;
        spin = dst->orientation;
        rand_scale = RAND_SCALE_inv;
        spin->u.st2D.angularSpeed = (f32) rand() * top * rand_scale + base;
        if ((src->flags >> 6) & 1) {
            f32 two_pi;
            two_pi = MTH_TWO_PI_inv;
            spin->u.st2D.angle = (f32) rand() * two_pi * rand_scale + (0.0f);
        } else {
            spin->u.st2D.angle = 0.0f;
        }
        return;
    }

    orientation = dst->orientation;
    gu_MTH4D_M_vSetIdentityMatrix(&orientation->matrix);

    if ((src->flags >> 6) & 1) {
        f32 rand_scale;
        f32 two_pi;
        f32 zero;
        f32 rx;
        f32 ry;
        f32 rz;
        s32 lRandom;

        lRandom = rand();
        rand_scale = RAND_SCALE_inv;
        two_pi = MTH_TWO_PI_inv;
        zero = (0.0f);
        rx = (f32) lRandom * two_pi * rand_scale + zero;
        lRandom = rand();
        ry = (f32) lRandom * two_pi * rand_scale + zero;
        lRandom = rand();
        rz = (f32) lRandom * two_pi * rand_scale + zero;
        POS_fn_vGetMatrixFromAngles(&orientation->matrix, rx, ry, rz);
    }

    if ((src->flags >> 7) & 1) {
        f32 zero;
        f32 span_x;
        f32 span_y;
        f32 rand_scale;
        s32 lRandom;

        inv_MEC_fn_vVectorToSpherical(&src->spinAxis, &xAngle1, &xAngle2, &xRadius);
        zero = (0.0f);
        span_x = src->spinSpread - zero;
        rand_scale = RAND_SCALE_inv;
        lRandom = rand();
        span_y = src->spinSpread - zero;
        xAngle1 = xAngle1 + ((f32) lRandom * span_x * rand_scale + zero);
        lRandom = rand();
        xAngle2 = xAngle2 + ((f32) lRandom * span_y * rand_scale + zero);
    } else {
        f32 rand_scale;
        f32 two_pi;
        f32 zero;
        s32 lRandom;

        lRandom = rand();
        rand_scale = RAND_SCALE_inv;
        two_pi = MTH_TWO_PI_inv;
        zero = (0.0f);
        xAngle1 = (f32) lRandom * two_pi * rand_scale + zero;
        lRandom = rand();
        xAngle2 = (f32) lRandom * two_pi * rand_scale + zero;
    }

    {
        f32 base;
        f32 top;

        base = src->spinBase;
        top = (base + src->spinRange) - base;
        xRadius = (f32) rand() * top * RAND_SCALE_inv + base;
    }
    inv_MEC_fn_vSphericalToVector(&orientation->u.stRotation.xX, xAngle1, xAngle2, xRadius);
    inv_MEC_fn_vVectorCrossProduct(dst);
}

void inv_MEC_fn_vInitParticleMaxStep(EffectParticle_inv* a0, EffectSystem_inv* a1)
{
    f32 base;
    f32 cand;

    base = a0->lifetime;
    a0->maxStep = base;
    if (((a1->flags >> 14) & 1) && !((a0->flags >> 2) & 1) && !((a0->flags >> 1) & 1)) {
        cand = a0->emitTimer;
        if (cand < base) {
            a0->maxStep = cand;
        }
    }
}

void inv_MEC_fn_vEmitFromPoint(EffectSystem_inv* src, MTH3D_tdstVector_* position,
                   MTH3D_tdstVector_* velocity)
{
    MTH3D_tdstVector_ scaled;
    MTH3D_tdstMatrix_ stBasis;
    POS_tdstMatrix_ rot;
    MTH3D_tdstVector_ stVector;
    f32 lo;
    f32 rand_scale;
    s32 lRandom;

    lo = 0.0f;
    MTH3D_M_vSetXYZ_inv(position, lo, lo, lo);
    rand_scale = RAND_SCALE_inv;

    {
        f32 base;
        f32 rspan;
        f32 xSpeed;

        base = src->speedBase;
        rspan = (base + src->speedRange) - base;
        lRandom = rand();
        xSpeed = (f32) lRandom * rspan * rand_scale + base;
        if (xSpeed < lo) {
            xSpeed = lo;
        }
        velocity->xZ = xSpeed;
    }
    velocity->xX = lo;
    velocity->xY = lo;

    {
        f32 angle_x;
        f32 angle_y;
        f32 xRangeX;
        f32 xRangeY;

        if ((src->flags >> 3) & 1) {
            f32 xspan;
            f32 yspan;

            xRangeX = src->spreadX * 0.33333334f;
            xRangeY = src->spreadY * 0.33333334f;
            xspan = xRangeX * xRangeX - lo;
            lRandom = rand();
            yspan = xRangeY * xRangeY - lo;
            angle_x = (f32) lRandom * xspan * rand_scale + lo;
            lRandom = rand();
            angle_y = (f32) lRandom * yspan * rand_scale + lo;
        } else {
            f32 low_x;
            f32 low_y;
            f32 xspan;
            f32 yspan;

            xRangeX = src->spreadX * 0.5f;
            xRangeY = src->spreadY * 0.5f;
            low_x = -xRangeX;
            low_y = -xRangeY;
            xspan = xRangeX - low_x;
            lRandom = rand();
            yspan = xRangeY - low_y;
            angle_x = (f32) lRandom * xspan * rand_scale + low_x;
            lRandom = rand();
            angle_y = (f32) lRandom * yspan * rand_scale + low_y;
        }

        POS_fn_vGetMatrixFromAngles(&rot, angle_y, angle_x, 0.0f);
    }

    stBasis.stCol_0 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_0;
    stBasis.stCol_1 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_1;
    stBasis.stCol_2 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_2;
    stVector = *(MTH3D_tdstVector_*) velocity;
    MTH3D_M_vSetXYZ_inv(
        velocity,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    stBasis.stCol_0 = src->basis[0].xyz;
    stBasis.stCol_1 = src->basis[1].xyz;
    stBasis.stCol_2 = src->basis[2].xyz;
    stVector = *(MTH3D_tdstVector_*) velocity;
    MTH3D_M_vSetXYZ_inv(
        velocity,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    MTH3D_M_vKTimesXYZ_inv(&scaled, src->directionScale, &src->direction);
    MTH3D_M_vAddXYZ_inv(velocity, velocity, &scaled);

    MTH3D_M_vAddXYZ_inv(position, position, &src->position);
}

void inv_MEC_fn_vEmitFromRectangle(EffectSystem_inv* src, MTH3D_tdstVector_* position,
                   MTH3D_tdstVector_* velocity)
{
    MTH3D_tdstVector_ scaled;
    MTH3D_tdstMatrix_ stBasis;
    POS_tdstMatrix_ rot;
    MTH3D_tdstVector_ stVector;
    f32 lo;
    f32 rand_scale;
    s32 lRandom;

    lo = 0.0f;
    rand_scale = RAND_SCALE_inv;

    {
        f32 half;
        f32 xWidth;
        f32 xHeight;
        f32 xLowX;
        f32 xLowY;
        f32 xSpanX;
        f32 xSpanY;
        f32 xPosX;

        xWidth = src->emitterWidth;
        half = 0.5f;
        xLowX = -xWidth * half;
        xSpanX = xWidth * half - xLowX;
        lRandom = rand();
        xHeight = src->emitterHeight;
        xLowY = -xHeight * half;
        xSpanY = xHeight * half - xLowY;
        xPosX = (f32) lRandom * xSpanX * rand_scale + xLowX;
        lRandom = rand();
        MTH3D_M_vSetXYZ_inv(position, xPosX, (f32) lRandom * xSpanY * rand_scale + xLowY, lo);
    }

    {
        f32 base;
        f32 rspan;
        f32 xSpeed;

        base = src->speedBase;
        rspan = (base + src->speedRange) - base;
        lRandom = rand();
        xSpeed = (f32) lRandom * rspan * rand_scale + base;
        if (xSpeed < lo) {
            xSpeed = lo;
        }
        velocity->xZ = xSpeed;
    }
    velocity->xX = lo;
    velocity->xY = lo;

    {
        f32 angle_x;
        f32 angle_y;
        f32 xRangeX;
        f32 xRangeY;

        if ((src->flags >> 3) & 1) {
            f32 xspan;
            f32 yspan;

            xRangeX = src->spreadX * 0.33333334f;
            xRangeY = src->spreadY * 0.33333334f;
            xspan = xRangeX * xRangeX - lo;
            lRandom = rand();
            yspan = xRangeY * xRangeY - lo;
            angle_x = (f32) lRandom * xspan * rand_scale + lo;
            lRandom = rand();
            angle_y = (f32) lRandom * yspan * rand_scale + lo;
        } else {
            f32 low_x;
            f32 low_y;
            f32 xspan;
            f32 yspan;

            xRangeX = src->spreadX * 0.5f;
            xRangeY = src->spreadY * 0.5f;
            low_x = -xRangeX;
            low_y = -xRangeY;
            xspan = xRangeX - low_x;
            lRandom = rand();
            yspan = xRangeY - low_y;
            angle_x = (f32) lRandom * xspan * rand_scale + low_x;
            lRandom = rand();
            angle_y = (f32) lRandom * yspan * rand_scale + low_y;
        }

        POS_fn_vGetMatrixFromAngles(&rot, angle_y, angle_x, 0.0f);
    }

    stBasis.stCol_0 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_0;
    stBasis.stCol_1 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_1;
    stBasis.stCol_2 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_2;
    stVector = *(MTH3D_tdstVector_*) velocity;
    MTH3D_M_vSetXYZ_inv(
        velocity,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    stBasis.stCol_0 = src->basis[0].xyz;
    stBasis.stCol_1 = src->basis[1].xyz;
    stBasis.stCol_2 = src->basis[2].xyz;
    stVector = *(MTH3D_tdstVector_*) velocity;
    MTH3D_M_vSetXYZ_inv(
        velocity,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    MTH3D_M_vKTimesXYZ_inv(&scaled, src->directionScale, &src->direction);
    MTH3D_M_vAddXYZ_inv(velocity, velocity, &scaled);

    stVector = *(MTH3D_tdstVector_*) position;
    MTH3D_M_vSetXYZ_inv(
        position,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    MTH3D_M_vAddXYZ_inv(position, position, &src->position);
}

void inv_MEC_fn_vEmitFromSphere(EffectSystem_inv* src, MTH3D_tdstVector_* position,
                   MTH3D_tdstVector_* velocity)
{
    MTH3D_tdstVector_ scaled;
    MTH3D_tdstMatrix_ stBasis;
    POS_tdstMatrix_ rot;
    MTH3D_tdstVector_ stVector;
    f32 lo;
    f32 rand_scale;
    s32 lRandom;

    lo = 0.0f;
    rand_scale = RAND_SCALE_inv;

    {
        f32 range;
        f32 span;
        f32 base;
        f32 xAngle1;
        f32 xAngle2;
        f32 xRadius;

        xAngle1 = src->emitterRadius;
        span = MTH_TWO_PI_inv;
        range = xAngle1 - lo;
        lRandom = rand();
        base = -MTH_PI_inv;
        xRadius = (f32) lRandom * range * rand_scale + lo;

        lRandom = rand();
        xAngle2 = (f32) lRandom * span * rand_scale + lo;

        lRandom = rand();
        xAngle1 = (f32) lRandom * span * rand_scale + base;
        inv_MEC_fn_vSphericalToVector((f32*) position, xAngle1, xAngle2, xRadius);
    }

    {
        f32 base;
        f32 rspan;
        f32 xSpeed;

        base = src->speedBase;
        rspan = (base + src->speedRange) - base;
        lRandom = rand();
        xSpeed = (f32) lRandom * rspan * rand_scale + base;
        if (xSpeed < lo) {
            xSpeed = lo;
        }
        velocity->xZ = xSpeed;
    }
    velocity->xX = lo;
    velocity->xY = lo;

    {
        f32 angle_x;
        f32 angle_y;
        f32 xRangeX;
        f32 xRangeY;

        if ((src->flags >> 3) & 1) {
            f32 xspan;
            f32 yspan;

            xRangeX = src->spreadX * 0.33333334f;
            xRangeY = src->spreadY * 0.33333334f;
            xspan = xRangeX * xRangeX - lo;
            lRandom = rand();
            yspan = xRangeY * xRangeY - lo;
            angle_x = (f32) lRandom * xspan * rand_scale + lo;
            lRandom = rand();
            angle_y = (f32) lRandom * yspan * rand_scale + lo;
        } else {
            f32 low_x;
            f32 low_y;
            f32 xspan;
            f32 yspan;

            xRangeX = src->spreadX * 0.5f;
            xRangeY = src->spreadY * 0.5f;
            low_x = -xRangeX;
            low_y = -xRangeY;
            xspan = xRangeX - low_x;
            lRandom = rand();
            yspan = xRangeY - low_y;
            angle_x = (f32) lRandom * xspan * rand_scale + low_x;
            lRandom = rand();
            angle_y = (f32) lRandom * yspan * rand_scale + low_y;
        }

        POS_fn_vGetMatrixFromAngles(&rot, angle_y, angle_x, 0.0f);
    }

    stBasis.stCol_0 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_0;
    stBasis.stCol_1 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_1;
    stBasis.stCol_2 = *(MTH3D_tdstVector_*) &rot.stMatrix.stCol_2;
    stVector = *(MTH3D_tdstVector_*) velocity;
    MTH3D_M_vSetXYZ_inv(
        velocity,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    stBasis.stCol_0 = src->basis[0].xyz;
    stBasis.stCol_1 = src->basis[1].xyz;
    stBasis.stCol_2 = src->basis[2].xyz;
    stVector = *(MTH3D_tdstVector_*) velocity;
    MTH3D_M_vSetXYZ_inv(
        velocity,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    MTH3D_M_vKTimesXYZ_inv(&scaled, src->directionScale, &src->direction);
    MTH3D_M_vAddXYZ_inv(velocity, velocity, &scaled);

    stVector = *(MTH3D_tdstVector_*) position;
    MTH3D_M_vSetXYZ_inv(
        position,
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xX, stBasis.stCol_1.xX, stBasis.stCol_2.xX,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xY, stBasis.stCol_1.xY, stBasis.stCol_2.xY,
                                   stVector.xX, stVector.xY, stVector.xZ),
        inv_MTH_fn_xMulAdd3Grouped(stBasis.stCol_0.xZ, stBasis.stCol_1.xZ, stBasis.stCol_2.xZ,
                                   stVector.xX, stVector.xY, stVector.xZ));

    MTH3D_M_vAddXYZ_inv(position, position, &src->position);
}

extern s32 inv_MEC_fn_bRestOnCollider();

void inv_MEC_fn_vEmitCollisionChild(EffectParticle_inv* p_stInput, EffectParticle_inv* dst, EffectSystem_inv* src,
                   void* normal, CollisionNode* node)
{
    f32 zero, xSquareLength, xLength, value;
    f32 vec0[4];
    f32 vec1[4];
    f32 vec2[3];

    dst->position = p_stInput->position;
    dst->acceleration = src->acceleration;
    dst->flags = (dst->flags & 0xFA) | 2;
    src->inv_lActiveParticleCount++;
    src->inv_lUsedParticleCount++;
    src->inv_lFreeParticleCount--;
    zero = 0.0f;
    inv_MEC_fn_vVectorToSpherical(&p_stInput->velocity, &vec2[0], &vec2[1], &vec2[2]);
    xSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stInput->velocity.xX, p_stInput->velocity.xY,
                                                p_stInput->velocity.xZ);
    xLength = MTH_fn_xSqrtOpt2(xSquareLength);
    if (xLength != zero && ((src->flags >> 13) & 1)) {
        vec2[0] += inv_MTH3D_fn_xRandomInRange(zero, src->childAngleRange);
        vec2[1] += inv_MTH3D_fn_xRandomInRange(zero, src->childAngleRange);
        xLength *= src->childSpeedScale;
        xLength += inv_MTH3D_fn_xRandomInRange(zero, src->childSpeedRange);
    } else {
        xLength = inv_MTH3D_fn_xRandomInRange(0.0f, xLength * 0.2f);
        vec2[1] = inv_MTH3D_fn_xRandomInRange(0.0f, MTH_TWO_PI_inv);
        vec2[0] = inv_MTH3D_fn_xRandomInRange(0.0f, MTH_TWO_PI_inv);
    }
    inv_MEC_fn_vSphericalToVector(&dst->velocity.xX, vec2[0], vec2[1], xLength);

    if (node != 0 && node->type != 2) {
        inv_MEC_fn_vSplitAlongNormal(&dst->velocity, (MTH3D_tdstVector_*) vec0, (MTH3D_tdstVector_*) vec1, normal);
        inv_MEC_fn_bRestOnCollider(dst, src, node, vec0, vec1, normal);
    }

    value = inv_MTH3D_fn_xRandomInRange(src->childLifetimeBase,
                                        src->childLifetimeBase + src->childLifetimeRange);
    dst->lastCollider = 0;
    dst->lifetime = value;
    dst->initialLifetime = value;
    inv_MEC_fn_vInitParticleSpin(dst, src);
    inv_MEC_fn_vInitParticleMaxStep(dst, src);
}

void inv_MEC_fn_vEmitTrailParticle(EffectParticle_inv* element, EffectParticle_inv* target,
                   EffectSystem_inv* system)
{
    f32 xZero;
    f32 xScale;

    xZero = 0.0f;
    target->position = element->position;
    MTH3D_M_vSetXYZ_inv(&target->velocity, xZero, xZero, xZero);
    MTH3D_M_vSetXYZ_inv(&target->acceleration, xZero, xZero, xZero);
    target->flags = (target->flags & 0xFC) | 4;
    system->inv_lActiveParticleCount = system->inv_lActiveParticleCount + 1;
    system->inv_lUsedParticleCount = system->inv_lUsedParticleCount + 1;
    system->inv_lFreeParticleCount = system->inv_lFreeParticleCount - 1;
    xScale = element->lifetime * system->emitLifetimeScale;
    target->lastCollider = 0;
    target->lifetime = xScale;
    target->initialLifetime = xScale;
}

void inv_MEC_fn_vEmitParticle(EffectParticle_inv* dst, EffectSystem_inv* src)
{
    f32 base;
    f32 rand_scale;
    f32 span;
    f32 total;
    f32 acc;
    s32 lRandom;

    dst->position = src->position;
    if (src->emitterShape == 0) {
        inv_MEC_fn_vEmitFromPoint(src, &dst->position, &dst->velocity);
    } else if (src->emitterShape == 1) {
        inv_MEC_fn_vEmitFromRectangle(src, &dst->position, &dst->velocity);
    } else {
        inv_MEC_fn_vEmitFromSphere(src, &dst->position, &dst->velocity);
    }
    dst->acceleration = src->acceleration;
    base = 0.0f;
    src->inv_lActiveParticleCount = src->inv_lActiveParticleCount + 1;
    src->inv_lUsedParticleCount = src->inv_lUsedParticleCount + 1;
    src->inv_lFreeParticleCount = src->inv_lFreeParticleCount - 1;
    span = src->lifetimeRange - base;
    lRandom = rand();
    rand_scale = RAND_SCALE_inv;
    total = (f32) lRandom * span * rand_scale + base;
    acc = src->lifetimeBase;
    acc = acc + total;
    dst->lifetime = acc;
    if (acc < 0.0001f) {
        dst->lifetime = 0.0001f;
    }
    dst->lastCollider = 0;
    dst->initialLifetime = dst->lifetime;
    dst->flags = dst->flags & 0xF8;
    if ((src->flags >> 14) & 1) {
        dst->emitTimer = src->emitPeriod;
    } else {
        dst->emitTimer = flt_3934C8[0];
    }
    inv_MEC_fn_vInitParticleSpin(dst, src);
    inv_MEC_fn_vInitParticleMaxStep(dst, src);
}

EffectParticle_inv* inv_MEC_fn_p_stFindFreeParticle(EffectSystem_inv* system)
{
    u32 count = system->particleCount;
    EffectParticle_inv* entry = system->particles;
    EffectParticle_inv* end = entry + count;

    while (entry < end) {
        if (entry->flags & 1) {
            return entry;
        }
        entry++;
    }
    return 0;
}

EffectParticle_inv* inv_MEC_fn_p_stFindTrailParticle(EffectSystem_inv* system)
{
    u32 count = system->particleCount;
    EffectParticle_inv* entry = system->particles;
    EffectParticle_inv* end = entry + count;

    while (entry < end) {
        if ((entry->flags >> 2) & 1) {
            return entry;
        }
        entry++;
    }
    return 0;
}

void inv_MEC_fn_vEmitCollisionChildren(EffectParticle_inv* particle, EffectSystem_inv* system, f32 time, void* normal,
                   CollisionNode* collider)
{
    s32 lMinCount;
    s32 span;
    s32 lRandom;
    s32 count;
    s32 lRemaining;
    f32 fspan;
    f32 xCount;
    EffectParticle_inv* node;

    lMinCount = system->childCountMin;
    span = (s8) ((u8) system->childCountMin + system->childCountRange) - lMinCount;
    fspan = (f32) span + 1.0f;
    lRandom = rand();
    fspan = fspan * RAND_SCALE_inv;
    xCount = (f32) lRandom * fspan;
    count = (s32) xCount + lMinCount;
    if (count <= 0) {
        return;
    }
    lRemaining = count;
    do {
        node = inv_MEC_fn_p_stFindFreeParticle(system);
        if (node != 0) {
            inv_MEC_fn_vEmitCollisionChild(particle, node, system, normal, collider);
        }
        lRemaining--;
    } while (lRemaining != 0);
}

void inv_MEC_fn_vParticleHitCollider(EffectParticle_inv* obj, EffectSystem_inv* system,
                   MTH3D_tdstVector_* v1, MTH3D_tdstVector_* v2,
                   MTH3D_tdstVector_* normal, CollisionNode* node, f32 dt)
{
    EffectParticle_inv* particle;
    EffectSystem_inv* owner;
    MTH3D_tdstVector_* from;
    MTH3D_tdstVector_* to;
    MTH3D_tdstVector_* plane;
    CollisionNode* collider;
    f32 time;
    f32 dx;
    f32 dy;
    f32 dz;
    u32 flags;
    u32 low;

    particle = obj;
    owner = system;
    from = v1;
    to = v2;
    plane = normal;
    collider = node;
    time = dt;
    if (inv_MEC_fn_bRestOnCollider(particle, owner, collider, to, from, plane) == 0) {
        dx = to->xX - from->xX;
        particle->velocity.xX = dx;
        dz = to->xY - from->xY;
        particle->velocity.xY = dz;
        dy = to->xZ - from->xZ;
        particle->velocity.xZ = dy;
        MTH3D_M_vSetXYZ_inv(&particle->velocity, collider->inv_xBounce * dx,
                            collider->inv_xBounce * dz, collider->inv_xBounce * dy);
    }
    flags = owner->flags;
    low = flags & 0xFFFF;
    if (((low >> 10) & 1) != 0 && ((particle->flags >> 1) & 1) == 0) {
        inv_MEC_fn_vEmitCollisionChildren(particle, owner, time, plane, collider);
        flags = owner->flags;
    } else if (low & 0x10000) {
        particle->flags |= low * low;
    }
    if (((flags >> 9) & 1) != 0 && ((particle->flags >> 1) & 1) == 0 &&
        (particle->flags & 1) == 0) {
        particle->flags |= 1;
        owner->inv_lFreeParticleCount = owner->inv_lFreeParticleCount + 1;
        owner->inv_lActiveParticleCount = owner->inv_lActiveParticleCount - 1;
        owner->inv_lUsedParticleCount = owner->inv_lUsedParticleCount - 1;
    }
}

static inline void inv_MTH3D_fn_vDivVector(MTH3D_tdstVector_* vector, f32 divisor)
{
    MTH3D_M_vSetXYZ_inv(vector, vector->xX / divisor, vector->xY / divisor, vector->xZ / divisor);
}

void inv_MEC_fn_vUpdateParticleOrientation(EffectParticle_inv* element, EffectSystem_inv* system, f32 elapsed)
{
    u16 flags = system->flags;
    EffectOrientation_inv* p_stOrientation;
    MTH3D_tdstMatrix_ stRotation;
    MTH3D_tdstVector_ stUp;
    MTH3D_tdstVector_ dir;
    MTH3D_tdstVector_ stAxisY;
    MTH3D_tdstVector_ stAxisX;
    MTH3D_tdstVector_ stNullVector;
    f32 xNorm;

    if ((flags >> 4) & 1) {
        if (!((element->flags >> 2) & 1)) {
            if (*(u8*) &system->flags & 1) {
                p_stOrientation = element->orientation;
                p_stOrientation->u.st2D.angle =
                    p_stOrientation->u.st2D.angle + p_stOrientation->u.st2D.angularSpeed * elapsed;
                if (MTH_TWO_PI_inv < p_stOrientation->u.st2D.angle) {
                    p_stOrientation->u.st2D.angle = p_stOrientation->u.st2D.angle - MTH_TWO_PI_inv;
                } else if (p_stOrientation->u.st2D.angle < 0.0f) {
                    p_stOrientation->u.st2D.angle = p_stOrientation->u.st2D.angle + MTH_TWO_PI_inv;
                }
            } else {
                p_stOrientation = element->orientation;
                if ((flags >> 5) & 1) {
                    MTH3D_M_vSetXYZ_inv(&stUp, 0.0f, 0.0f, 1.0f);
                    dir = element->velocity;
                    {
                        f32 sq;

                        sq = MTH_fn_xSqrAddSqrAddSqrOpt2(dir.xX, dir.xY, dir.xZ);
                        xNorm = MTH_fn_xSqrtOpt2(sq);
                    }
                    if (xNorm < 1e-4f) {
                        gu_MTH4D_M_vSetIdentityMatrix(&p_stOrientation->matrix);
                        return;
                    }
                    inv_MTH3D_fn_vDivVector(&dir, xNorm);
                    MTH3D_M_vCopyXYZ_inv(&stRotation.stCol_2, &dir);
                    if (&stAxisY == &stUp || &stAxisY == &dir) {
                        MTH3D_tdstVector_ stTemp;

                        MTH3D_M_vSetXYZ_inv(&stTemp,
                                            inv_MTH_fn_xMulSub(stUp.xY, dir.xZ, stUp.xZ, dir.xY),
                                            inv_MTH_fn_xMulSub(stUp.xZ, dir.xX, stUp.xX, dir.xZ),
                                            inv_MTH_fn_xMulSub(stUp.xX, dir.xY, stUp.xY, dir.xX));
                        stAxisY = stTemp;
                    } else {
                        MTH3D_M_vSetXYZ_inv(&stAxisY,
                                            inv_MTH_fn_xMulSub(stUp.xY, dir.xZ, stUp.xZ, dir.xY),
                                            inv_MTH_fn_xMulSub(stUp.xZ, dir.xX, stUp.xX, dir.xZ),
                                            inv_MTH_fn_xMulSub(stUp.xX, dir.xY, stUp.xY, dir.xX));
                    }
                    {
                        f32 sq;

                        sq = MTH_fn_xSqrAddSqrAddSqrOpt2(stAxisY.xX, stAxisY.xY, stAxisY.xZ);
                        xNorm = MTH_fn_xSqrtOpt2(sq);
                    }
                    if (xNorm < 1e-4f) {
                        gu_MTH4D_M_vSetIdentityMatrix(&p_stOrientation->matrix);
                        return;
                    }
                    inv_MTH3D_fn_vDivVector(&stAxisY, xNorm);
                    if (&stAxisX == &dir || &stAxisX == &stAxisY) {
                        MTH3D_tdstVector_ stTemp;

                        MTH3D_M_vSetXYZ_inv(
                            &stTemp, inv_MTH_fn_xMulSub(dir.xY, stAxisY.xZ, dir.xZ, stAxisY.xY),
                            inv_MTH_fn_xMulSub(dir.xZ, stAxisY.xX, dir.xX, stAxisY.xZ),
                            inv_MTH_fn_xMulSub(dir.xX, stAxisY.xY, dir.xY, stAxisY.xX));
                        stAxisX = stTemp;
                    } else {
                        MTH3D_M_vSetXYZ_inv(
                            &stAxisX, inv_MTH_fn_xMulSub(dir.xY, stAxisY.xZ, dir.xZ, stAxisY.xY),
                            inv_MTH_fn_xMulSub(dir.xZ, stAxisY.xX, dir.xX, stAxisY.xZ),
                            inv_MTH_fn_xMulSub(dir.xX, stAxisY.xY, dir.xY, stAxisY.xX));
                    }
                    {
                        f32 sq;

                        sq = MTH_fn_xSqrAddSqrAddSqrOpt2(stAxisX.xX, stAxisX.xY, stAxisX.xZ);
                        xNorm = MTH_fn_xSqrtOpt2(sq);
                    }
                    if (xNorm < 1e-4f) {
                        gu_MTH4D_M_vSetIdentityMatrix(&p_stOrientation->matrix);
                        return;
                    }
                    inv_MTH3D_fn_vDivVector(&stAxisX, xNorm);
                    MTH3D_M_vCopyXYZ_inv(&stRotation.stCol_1, &stAxisY);
                    MTH3D_M_vCopyXYZ_inv(&stRotation.stCol_0, &stAxisX);
                    stNullVector.xZ = stNullVector.xY = stNullVector.xX = 0.0f;
                    inv_POS_fn_vInitMatrixFromRotationAndTranslation(&p_stOrientation->matrix, &stRotation, &stNullVector);
                } else {
                    MTH3D_tdstVector_ stNullVector2;
                    MTH3D_tdstMatrix_ stDelta;
                    MTH3D_tdstMatrix_ stCurrent;

                    stCurrent.stCol_0 = *(MTH3D_tdstVector_*) &p_stOrientation->matrix.stMatrix.stCol_0;
                    stCurrent.stCol_1 = *(MTH3D_tdstVector_*) &p_stOrientation->matrix.stMatrix.stCol_1;
                    stCurrent.stCol_2 = *(MTH3D_tdstVector_*) &p_stOrientation->matrix.stMatrix.stCol_2;
                    if (&stDelta != &p_stOrientation->rotationSpeed) {
                        if (&stDelta != &stCurrent)
                            goto dir;
                    } else {
                        asm("");
                    }
                    {
                        MTH3D_tdstMatrix_ stTemp;
                        MTH3D_M_vSetXYZ_inv(
                            &stTemp.stCol_0,
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xX,
                                             p_stOrientation->rotationSpeed.stCol_1.xX,
                                             p_stOrientation->rotationSpeed.stCol_2.xX,
                                             stCurrent.stCol_0.xX, stCurrent.stCol_0.xY,
                                             stCurrent.stCol_0.xZ),
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xY,
                                             p_stOrientation->rotationSpeed.stCol_1.xY,
                                             p_stOrientation->rotationSpeed.stCol_2.xY,
                                             stCurrent.stCol_0.xX, stCurrent.stCol_0.xY,
                                             stCurrent.stCol_0.xZ),
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xZ,
                                             p_stOrientation->rotationSpeed.stCol_1.xZ,
                                             p_stOrientation->rotationSpeed.stCol_2.xZ,
                                             stCurrent.stCol_0.xX, stCurrent.stCol_0.xY,
                                             stCurrent.stCol_0.xZ));
                        MTH3D_M_vSetXYZ_inv(
                            &stTemp.stCol_1,
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xX,
                                             p_stOrientation->rotationSpeed.stCol_1.xX,
                                             p_stOrientation->rotationSpeed.stCol_2.xX,
                                             stCurrent.stCol_1.xX, stCurrent.stCol_1.xY,
                                             stCurrent.stCol_1.xZ),
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xY,
                                             p_stOrientation->rotationSpeed.stCol_1.xY,
                                             p_stOrientation->rotationSpeed.stCol_2.xY,
                                             stCurrent.stCol_1.xX, stCurrent.stCol_1.xY,
                                             stCurrent.stCol_1.xZ),
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xZ,
                                             p_stOrientation->rotationSpeed.stCol_1.xZ,
                                             p_stOrientation->rotationSpeed.stCol_2.xZ,
                                             stCurrent.stCol_1.xX, stCurrent.stCol_1.xY,
                                             stCurrent.stCol_1.xZ));
                        MTH3D_M_vSetXYZ_inv(
                            &stTemp.stCol_2,
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xX,
                                             p_stOrientation->rotationSpeed.stCol_1.xX,
                                             p_stOrientation->rotationSpeed.stCol_2.xX,
                                             stCurrent.stCol_2.xX, stCurrent.stCol_2.xY,
                                             stCurrent.stCol_2.xZ),
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xY,
                                             p_stOrientation->rotationSpeed.stCol_1.xY,
                                             p_stOrientation->rotationSpeed.stCol_2.xY,
                                             stCurrent.stCol_2.xX, stCurrent.stCol_2.xY,
                                             stCurrent.stCol_2.xZ),
                            inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xZ,
                                             p_stOrientation->rotationSpeed.stCol_1.xZ,
                                             p_stOrientation->rotationSpeed.stCol_2.xZ,
                                             stCurrent.stCol_2.xX, stCurrent.stCol_2.xY,
                                             stCurrent.stCol_2.xZ));
                        stDelta = stTemp;
                    }
                    goto done;
                dir: {
                    MTH3D_M_vSetXYZ_inv(
                        &stDelta.stCol_0,
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xX,
                                                   p_stOrientation->rotationSpeed.stCol_1.xX,
                                                   p_stOrientation->rotationSpeed.stCol_2.xX,
                                                   stCurrent.stCol_0.xX, stCurrent.stCol_0.xY,
                                                   stCurrent.stCol_0.xZ),
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xY,
                                                   p_stOrientation->rotationSpeed.stCol_1.xY,
                                                   p_stOrientation->rotationSpeed.stCol_2.xY,
                                                   stCurrent.stCol_0.xX, stCurrent.stCol_0.xY,
                                                   stCurrent.stCol_0.xZ),
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xZ,
                                                   p_stOrientation->rotationSpeed.stCol_1.xZ,
                                                   p_stOrientation->rotationSpeed.stCol_2.xZ,
                                                   stCurrent.stCol_0.xX, stCurrent.stCol_0.xY,
                                                   stCurrent.stCol_0.xZ));
                    MTH3D_M_vSetXYZ_inv(
                        &stDelta.stCol_1,
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xX,
                                                   p_stOrientation->rotationSpeed.stCol_1.xX,
                                                   p_stOrientation->rotationSpeed.stCol_2.xX,
                                                   stCurrent.stCol_1.xX, stCurrent.stCol_1.xY,
                                                   stCurrent.stCol_1.xZ),
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xY,
                                                   p_stOrientation->rotationSpeed.stCol_1.xY,
                                                   p_stOrientation->rotationSpeed.stCol_2.xY,
                                                   stCurrent.stCol_1.xX, stCurrent.stCol_1.xY,
                                                   stCurrent.stCol_1.xZ),
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xZ,
                                                   p_stOrientation->rotationSpeed.stCol_1.xZ,
                                                   p_stOrientation->rotationSpeed.stCol_2.xZ,
                                                   stCurrent.stCol_1.xX, stCurrent.stCol_1.xY,
                                                   stCurrent.stCol_1.xZ));
                    MTH3D_M_vSetXYZ_inv(
                        &stDelta.stCol_2,
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xX,
                                                   p_stOrientation->rotationSpeed.stCol_1.xX,
                                                   p_stOrientation->rotationSpeed.stCol_2.xX,
                                                   stCurrent.stCol_2.xX, stCurrent.stCol_2.xY,
                                                   stCurrent.stCol_2.xZ),
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xY,
                                                   p_stOrientation->rotationSpeed.stCol_1.xY,
                                                   p_stOrientation->rotationSpeed.stCol_2.xY,
                                                   stCurrent.stCol_2.xX, stCurrent.stCol_2.xY,
                                                   stCurrent.stCol_2.xZ),
                        inv_MTH_fn_xMulAdd3Grouped(p_stOrientation->rotationSpeed.stCol_0.xZ,
                                                   p_stOrientation->rotationSpeed.stCol_1.xZ,
                                                   p_stOrientation->rotationSpeed.stCol_2.xZ,
                                                   stCurrent.stCol_2.xX, stCurrent.stCol_2.xY,
                                                   stCurrent.stCol_2.xZ));
                }
                done:;
                    inv_MTH3D_fn_vKTimesXYZ(&stDelta.stCol_0, elapsed, &stDelta.stCol_0);
                    inv_MTH3D_fn_vKTimesXYZ(&stDelta.stCol_1, elapsed, &stDelta.stCol_1);
                    inv_MTH3D_fn_vKTimesXYZ(&stDelta.stCol_2, elapsed, &stDelta.stCol_2);
                    inv_MTH3D_fn_vAddXYZ(&stCurrent.stCol_0, &stCurrent.stCol_0, &stDelta.stCol_0);
                    inv_MTH3D_fn_vAddXYZ(&stCurrent.stCol_1, &stCurrent.stCol_1, &stDelta.stCol_1);
                    inv_MTH3D_fn_vAddXYZ(&stCurrent.stCol_2, &stCurrent.stCol_2, &stDelta.stCol_2);
                    stNullVector2.xZ = stNullVector2.xY = stNullVector2.xX = 0.0f;
                    gu_MTH4D_M_vSetIdentityMatrix(&p_stOrientation->matrix);
                    inv_POS_fn_vInitMatrixFromRotationAndTranslation(&p_stOrientation->matrix, &stCurrent, &stNullVector2);
                }
            }
        }
    }
}

s32 inv_MEC_fn_bApplyVortexMotion(EffectParticle_inv* obj, EffectSystem_inv* desc, f32* dt)
{
    MTH3D_tdstVector_ axis;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ deltaCopy;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ projection;
    MTH3D_tdstVector_ tangent;
    MTH3D_tdstVector_ cross;
    MTH3D_tdstVector_ cosinePart;
    MTH3D_tdstVector_ sinePart;
    MTH3D_tdstVector_ displacement;
    MTH3D_tdstVector_ basis[3];
    MTH3D_tdstVector_ stTemp;
    f32 distance;
    f32 angleTemp;
    f32 dot;
    f32 angle;
    f32 turns;
    f32 phase;
    f32 cosine;
    f32 sign;
    f32 sine;
    f32 speed;
    f32 tangentLength;
    f32 lengthSq;
    f32 scale;
    f32 preScale;
    u32 bits;
    s32 turnInt;

    if (((obj->flags >> 2) & 1) != 0) {
        *dt = 0.0f;
        return 1;
    }

    MTH3D_M_vSetXYZ_inv(&axis, 0.0f, 0.0f, 1.0f);

    basis[0] = desc->basis[0].xyz;
    basis[1] = desc->basis[1].xyz;
    basis[2] = desc->basis[2].xyz;

    {
        stTemp = axis;
        MTH3D_M_vSetXYZ_inv(&axis,
                            inv_MTH_fn_xMulAdd3(basis[0].xX, stTemp.xX, basis[1].xX, stTemp.xY,
                                                basis[2].xX, stTemp.xZ),
                            inv_MTH_fn_xMulAdd3(basis[0].xY, stTemp.xX, basis[1].xY, stTemp.xY,
                                                basis[2].xY, stTemp.xZ),
                            inv_MTH_fn_xMulAdd3(basis[0].xZ, stTemp.xX, basis[1].xZ, stTemp.xY,
                                                basis[2].xZ, stTemp.xZ));
    }

    angleTemp = inv_MTH_fn_xMultiply(desc->forceStrength, *dt);

    MTH3D_M_vSetXYZ_inv(&delta, obj->position.xX - desc->position.xX,
                        obj->position.xY - desc->position.xY,
                        *(volatile f32*) &obj->position.xZ - *(volatile f32*) &desc->position.xZ);
    deltaCopy = delta;

    lengthSq = MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ);
    distance = MTH_fn_xSqrtOpt2(lengthSq);
    if (distance < D_00392B1C) {
        *dt = 0.0f;
        return 1;
    }

    MTH3D_M_vSetXYZ_inv(&direction, delta.xX / distance, delta.xY / distance, delta.xZ / distance);

    dot = inv_MTH_fn_xMulAdd3(direction.xX, axis.xX, direction.xY, axis.xY, direction.xZ, axis.xZ);

    inv_MTH3D_fn_vKTimesXYZ(&projection, dot, &axis);

    inv_MTH3D_fn_vSubXYZ(&tangent, &direction, &projection);

    {
        if (&cross == &axis || &cross == &tangent) {
            DNM_p_stVectorCrossProduct(&stTemp, &axis, &tangent);
            *(&cross) = (stTemp);
        } else {
            DNM_p_stVectorCrossProduct(&cross, &axis, &tangent);
        }
    };

    angle = angleTemp / powf(distance, desc->forceExponent);
    preScale = D_00392B24;
    turns = angle * D_00392B20;
    turnInt = inv_MTH_fn_lToIntegerOrdered(turns);
    phase = (f32) turnInt;
    angleTemp = angle - phase * preScale;
    phase = angleTemp;
    cosine = cosf(angle);

    if (__builtin_fabsf(phase) < D_00392B28) {
        sign = inv_MTH_fn_xSign(phase);
    } else {
        bits = *(u32*) &phase;
        if ((bits & 0x7FFFFFFF) == 0) {
            sign = 0.0f;
            sign = -sign;
        } else {
            bits = (bits & 0x80000000) | 0x3F800000;
            sign = -(*(f32*) &bits);
        }
    }

    sine = MTH_fn_xSqrtOpt2(1.0f - MTH_fn_xSqrOpt1(cosine));
    sine = sine * sign;

    inv_MTH3D_fn_vKTimesXYZ(&cosinePart, cosine, &tangent);

    inv_MTH3D_fn_vKTimesXYZ(&sinePart, sine, &cross);

    MTH3D_M_vSetXYZ_inv(&delta, distance * (projection.xX + cosinePart.xX + sinePart.xX),
                        distance * (projection.xY + cosinePart.xY + sinePart.xY),
                        distance * (projection.xZ + cosinePart.xZ + sinePart.xZ));

    MTH3D_M_vAddXYZ_inv(&obj->position, &delta, &desc->position);

    speed = inv_MTH3D_fn_xVectorLength(obj->velocity.xX, obj->velocity.xY, obj->velocity.xZ);
    tangentLength = inv_MTH3D_fn_xVectorLength(tangent.xX, tangent.xY, tangent.xZ);

    MTH3D_M_vKTimesXYZ_inv(&displacement, speed * *dt, &axis);

    if (tangentLength != 0.0f) {
        f32 invLength;
        invLength = 1.0f / tangentLength;
        inv_MTH3D_fn_vKTimesXYZ(&tangent, invLength, &tangent);
    }

    scale = speed * desc->forceTangentScale * *dt;
    inv_MTH3D_fn_vKTimesXYZ(&tangent, scale, &tangent);
    inv_MTH3D_fn_vAddXYZ(&displacement, &displacement, &tangent);

    MTH3D_M_vAddXYZ_inv(&obj->position, &obj->position, &displacement);

    *dt = 0.0f;
    return 1;
}

void inv_MEC_fn_vApplyColliderFriction(EffectParticle_inv* obj, EffectSystem_inv* system, f32 dt)
{
    MTH3D_tdstVector_ step;
    MTH3D_tdstVector_ diff;
    MTH3D_tdstVector_ dir;
    CollisionNode* node;
    f32 len;
    f32 xDistanceFactor;
    u8 type;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 ex;
    f32 ey;
    f32 ez;

    step = obj->acceleration;
    node = obj->lastCollider;
    if (node != NULL) {
        dx = system->acceleration.xX - obj->acceleration.xX;
        dy = system->acceleration.xY - obj->acceleration.xY;
        dz = system->acceleration.xZ - obj->acceleration.xZ;
        MTH3D_M_vSetXYZ_inv(&diff, dx, dy, dz);
        dx = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
        dx = MTH_fn_xSqrtOpt2(dx);
        xDistanceFactor = dx * D_00392B2C;
        dir = obj->velocity;
        len = MTH_fn_xSqrAddSqrAddSqrOpt2(dir.xX, dir.xY, dir.xZ);
        len = MTH_fn_xSqrtOpt2(len);
        if (len > D_00392B30) {
            ex = dir.xX / len;
            ey = dir.xY / len;
            ez = dir.xZ / len;
            MTH3D_M_vSetXYZ_inv(&dir, ex, ey, ez);
            type = node->type;
            if (type & 0x100) {
                dir.xX = xDistanceFactor * xDistanceFactor;
                dir.xY = xDistanceFactor;
            }
            if (type == 1 || type == 4 || type == 8) {
                MTH3D_M_vSetXYZ_inv(
                    &dir, ((CollisionShape*) node->data)->inv_xFriction * xDistanceFactor * ex,
                    ((CollisionShape*) node->data)->inv_xFriction * xDistanceFactor * ey,
                    ((CollisionShape*) node->data)->inv_xFriction * xDistanceFactor * ez);
            }
            MTH3D_M_vAddXYZ_inv(&step, &step, &dir);
        }
    }
    MTH3D_M_vKTimesXYZ_inv(&step, dt, &step);
    MTH3D_M_vAddXYZ_inv(&obj->velocity, &obj->velocity, &step);
}

extern s32 func_0013F5C0(EffectParticle_inv* particle, EffectSystem_inv* system,
                         CollisionNode** outNode, f32* outDistance, MTH3D_tdstVector_* segment,
                         MTH3D_tdstVector_* outHit);

s32 inv_MEC_fn_bMoveParticle(EffectParticle_inv* obj, EffectSystem_inv* system, f32* p_dt)
{
    MTH3D_tdstVector_ new_pos;
    MTH3D_tdstVector_ normal;
    MTH3D_tdstVector_ velocityStep;
    MTH3D_tdstVector_ accelerationStep;
    MTH3D_tdstVector_ hitAccelerationStep;
    MTH3D_tdstVector_ sp_50;
    MTH3D_tdstVector_ sp_60;
    CollisionNode* node;
    f32 sub_dt;
    f32 dt;
    f32 half_dt2;
    f32 half_subdt2;

    node = NULL;

    dt = *p_dt;
    half_dt2 = dt * 0.5f;

    velocityStep.xX = dt * obj->velocity.xX;
    velocityStep.xY = dt * obj->velocity.xY;

    half_dt2 *= dt;
    dt *= obj->velocity.xZ;
    velocityStep.xZ = dt;

    MTH3D_M_vAddXYZ_inv(&new_pos, &obj->position, &velocityStep);

    MTH3D_M_vKTimesXYZ_inv(&accelerationStep, half_dt2, &obj->acceleration);

    MTH3D_M_vAddXYZ_inv(&new_pos, &new_pos, &accelerationStep);

    if (func_0013F5C0(obj, system, &node, &sub_dt, &new_pos, &normal) != 0) {
        *p_dt -= sub_dt;
        half_subdt2 = sub_dt * 0.5f;
        half_subdt2 *= sub_dt;

        obj->position.xX += sub_dt * obj->velocity.xX;
        obj->position.xY += sub_dt * obj->velocity.xY;
        obj->position.xZ += sub_dt * obj->velocity.xZ;

        MTH3D_M_vKTimesXYZ_inv(&hitAccelerationStep, half_subdt2, &obj->acceleration);

        MTH3D_M_vAddXYZ_inv(&obj->position, &obj->position, &hitAccelerationStep);

        inv_MEC_fn_vApplyColliderFriction(obj, system, sub_dt);
        inv_MEC_fn_vSplitAlongNormal(&obj->velocity, &sp_50, &sp_60, &normal);
        inv_MEC_fn_vParticleHitCollider(obj, system, &sp_60, &sp_50, &normal, node, *p_dt);
    } else {
        obj->position = new_pos;
        inv_MEC_fn_vApplyColliderFriction(obj, system, *p_dt);
        *p_dt = 0.0f;
    }

    if (obj->lastCollider != NULL) {
        func_0013F788(obj, system);
    }

    return (obj->flags & 1) ^ 1;
}

void inv_MEC_fn_vUpdateParticle(EffectParticle_inv* element, EffectSystem_inv* system, f32 time)
{
    f32 origin[3];
    f32 step;
    f32 elapsed;
    f32 zero;
    EffectParticle_inv* target;

    step = time < element->maxStep ? time : element->maxStep;
    elapsed = step;
    time -= elapsed;

    if (system->flags >> 15) {
        while (0.0f < step) {
            if (!inv_MEC_fn_bApplyVortexMotion(element,
                               system, &step)) {
                return;
            }
        }
    } else if (0.0f < elapsed) {
        if (!((element->flags >> 2) & 1)) {
            do {
                if (!inv_MEC_fn_bMoveParticle(element,
                                   system, &step)) {
                    return;
                }
            } while (0.0f < step && !((element->flags >> 2) & 1));
        }
    }

    zero = 0.0f;
    element->lifetime -= elapsed;
    if (element->lifetime <= zero) {
        if ((system->flags >> 11) & 1) {
            if (!((element->flags >> 1) & 1) && !((element->flags >> 2) & 1)) {
                origin[0] = zero;
                origin[1] = zero;
                origin[2] = zero;
                inv_MEC_fn_vEmitCollisionChildren(element, system, time, origin, NULL);
            }
        }
        element->flags |= 1;
        system->inv_lFreeParticleCount++;
        system->inv_lActiveParticleCount--;
        system->inv_lUsedParticleCount--;
        return;
    }

    inv_MEC_fn_vUpdateParticleOrientation(element, system, elapsed);
    if ((system->flags >> 14) & 1) {
        if (!((element->flags >> 2) & 1)) {
            if (!((element->flags >> 1) & 1)) {
                element->emitTimer -= elapsed;
                if (element->emitTimer <= zero) {
                    target = inv_MEC_fn_p_stFindFreeParticle(system);
                    if (target != NULL || (target = inv_MEC_fn_p_stFindTrailParticle(system)) != NULL) {
                        inv_MEC_fn_vEmitTrailParticle(element, target,
                                      system);
                    }
                    element->emitTimer = system->emitPeriod;
                }
            }
        }
    }
    inv_MEC_fn_vInitParticleMaxStep(element, system);
}

void inv_MEC_fn_vUpdateParticles(EffectSystem_inv* system, f32 dt)
{
    EffectParticle_inv* ptr;
    EffectParticle_inv* end;

    ptr = system->particles;
    end = ptr + system->particleCount;
    if (ptr < end) {
        do {
            if ((ptr->flags & 1) == 0) {
                inv_MEC_fn_vUpdateParticle(ptr, system, dt);
            }
            ptr++;
        } while (ptr < end);
    }
}

void inv_MEC_fn_vUpdateEffectSystemsActivity(struct GLD_tdstViewportAttributes_* p_stVpt)
{
    HIE_tdstSuperObject_* root;
    EffectSystem_inv* system;
    SECT_tdstSector_* sector;
    HIE_tdstSuperObject_* sectorSO;
    SECT_tdstListOfSOSectorsInActivityInteraction_* border;
    COL_tdstParallelBox_* box;
    COL_tdstBoundingSphere_* sphere;
    s32 found;
    s16 si;
    u8 flags;

    root = GAM_fn_hGetCurrentSector((void*) (*(u32*) &g_stEngineStructure.activeSuperObject));
    sector = root->hLinkedObject;
    system = *&inv_MEC_p_stFirstEffectSystem;
    if (system != 0) {
        do {
            found = 0;
            if (*(u8*) &root->flags & 0x10) {
                box = root->hVisualBoundingVolume;
                if (box != 0) {
                    if (INT_fn_bIntersectBoxWithBox(&box->stMinPoint, &box->stMaxPoint, &system->boundsMin,
                                                    &system->boundsMax) != 0) {
                        found = 1;
                    }
                }
            } else {
                sphere = root->hVisualBoundingVolume;
                if (sphere != 0) {
                    if (INT_fn_bIntersectSphereWithBox(&sphere->stCenterPoint, &system->boundsMin,
                                                       &system->boundsMax, sphere->xRadius) != 0) {
                        found = 1;
                    }
                }
            }
            border = sector->stListOfSectorsInActivityInteraction.hFirstElementOpt;
            if (sector->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt > 0) {
                si = 0;
                do {
                    sectorSO = border->hPointerOfSectorSO;
                    if (*(u8*) &sectorSO->flags & 0x10) {
                        box = sectorSO->hVisualBoundingVolume;
                        if (box != 0) {
                            if (INT_fn_bIntersectBoxWithBox(&box->stMinPoint, &box->stMaxPoint, &system->boundsMin,
                                                             &system->boundsMax) != 0) {
                                found = 1;
                            }
                        }
                    } else {
                        sphere = sectorSO->hVisualBoundingVolume;
                        if (sphere != 0) {
                            if (INT_fn_bIntersectSphereWithBox(&sphere->stCenterPoint, &system->boundsMin,
                                                                &system->boundsMax, sphere->xRadius) != 0) {
                                found = 1;
                            }
                        }
                    }
                    border = border + 1;
                } while (++si < sector->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt);
            }
            if (found == 0) {
                system->runFlags = system->runFlags | 4;
            } else if (GEO_lCullingBox(p_stVpt, &system->boundsMin, &system->boundsMax, 0) == 0) {
                system->runFlags = system->runFlags | 4;
            } else {
                system->runFlags = system->runFlags & 0xFB;
            }
            system = system->next;
        } while (system != 0);
    }
}

static inline u8 inv_MTH3D_fn_bIsSuspended(EffectSystem_inv* system)
{
    return system->runFlags & 1;
}

void inv_MEC_fn_vUpdateEffectSystem(EffectSystem_inv* system, f32 dt)
{
    EffectParticle_inv* particle;
    f32 remaining;
    s32 stop;

    if (((system->runFlags >> 2) & 1) || dt <= 0.0f) {
        return;
    }
    remaining = dt;
    system->age += remaining;
    inv_MEC_fn_vUpdateParticles(system, dt);
    stop = 0;
    while (system->spawnDelay <= remaining && !inv_MTH3D_fn_bIsSuspended(system) && !stop) {
        particle = inv_MEC_fn_p_stFindFreeParticle(system);
        if (particle == NULL && system->inv_lUsedParticleCount - system->inv_lFreeParticleCount >= system->particleCount) {
            particle = inv_MEC_fn_p_stFindTrailParticle(system);
        }
        if (particle != NULL) {
            inv_MEC_fn_vEmitParticle(particle, system);
            remaining -= system->spawnDelay;
            inv_MEC_fn_vUpdateParticle(particle, system, remaining);
            system->spawnDelay = 1.0f / system->spawnRate;
        }
        if (particle == NULL) {
            stop = 1;
        } else if ((system->flags >> 2) & 1) {
            stop = !(system->inv_lActiveParticleCount < system->spawnLimit);
        } else {
            stop = 0;
        }
    }
    if (!inv_MTH3D_fn_bIsSuspended(system)) {
        system->spawnDelay -= remaining;
    }
    if (system->spawnDelay < 0.0f) {
        system->spawnDelay = 0.0f;
    }
}

void inv_MEC_fn_vRenderRuntimeObject(EffectSystem_inv* owner, struct GLD_tdstViewportAttributes_* p_stVpt,
                                     s32 lDrawModeMask)
{
    EffectParticle_inv* it;
    EffectParticle_inv* end;
    if (*(u8*) &owner->flags & 1) {
        GeometricObjectView* buffer = owner->geometricObject;
        s16 count = 0;
        SpriteElementList* header;
        it = owner->particles;
        end = it + owner->particleCount;
        header = (SpriteElementList*) *buffer->elements;
        if ((u32) it < (u32) end) {
            do {
                if (!(it->flags & 1)) {
                    buffer->vertices[count] = it->position;
                    ++count;
                }
                ++it;
            } while ((u32) it < (u32) end);
        }
        header->count = count;
        HIE_fn_vPushSuperObjectMatrix(inv_MEC_hParticleSuperObject);
        GLI_xSendObjectToViewportWithLights(p_stVpt, owner->geometricObject, lDrawModeMask);
        HIE_fn_vPopMatrix();
    } else {
        it = owner->particles;
        end = it + owner->particleCount;
        while ((u32) it < (u32) end) {
            if (!(it->flags & 1)) {
                EffectOrientation_inv* node = it->orientation;
                HIE_fn_vPushSuperObjectMatrix(inv_MEC_hParticleSuperObject);
                if (node->object)
                    GLI_xSendObjectToViewportWithLights(p_stVpt, node->object, lDrawModeMask);
                HIE_fn_vPopMatrix();
            }
            ++it;
        }
    }
}

void inv_MEC_fn_vSetParticlesObject(EffectSystem_inv* system, void* object)
{
    u32 count = system->particleCount;
    EffectParticle_inv* entry = system->particles;
    EffectParticle_inv* end = entry + count;

    for (; entry < end; entry++) {
        entry->orientation->object = object;
    }
}

#ifdef PC_PORT
extern s32 inv_GAM_bDrawingInBetween;

#endif
void inv_MEC_fn_vUpdateAllEffectSystems(struct GLD_tdstViewportAttributes_* p_stVpt, s32 lDrawModeMask)
{
    f32 value;
    EffectSystem_inv* cur;
    value = inv_fn_xGetEngineClockDelta();
    inv_MEC_fn_vUpdateEffectSystemsActivity(p_stVpt);
    cur = inv_MEC_p_stFirstEffectSystem;
    while (cur != 0) {
        if ((cur->runFlags & 1) == 0 && ((cur->runFlags >> 2) & 1) == 0) {
#ifdef PC_PORT
            if (!inv_GAM_bDrawingInBetween) {
                inv_MEC_fn_vUpdateEffectSystem(cur, value);
            }
#else
            inv_MEC_fn_vUpdateEffectSystem(cur, value);
#endif
            inv_MEC_fn_vRenderRuntimeObject(cur, p_stVpt, lDrawModeMask);
        }
        cur = cur->next;
    }
}

void inv_MEC_fn_vResumeAllEffectSystems(void)
{
    EffectSystem_inv* node = inv_MEC_p_stFirstEffectSystem;

    while (node != 0) {
        node->runFlags = (u8) (node->runFlags & 0xFE);
        node = node->next;
    }
}

void inv_MEC_fn_vPauseAllEffectSystems(void)
{
    EffectSystem_inv* node = inv_MEC_p_stFirstEffectSystem;

    while (node != 0) {
        node->runFlags = (u8) (node->runFlags | 1);
        node = node->next;
    }
}

void inv_fn_vClearLevelRuntimeList(void)
{
    inv_MEC_p_stFirstEffectSystem = 0;
}

f32 inv_MEC_fn_xGetParticlePlaneDistance(EffectParticle_inv* particle, MTH3D_tdstVector_* planePoint,
                  MTH3D_tdstVector_* planeNormal)
{
    MTH3D_tdstVector_ normal;
    MTH3D_tdstVector_ point;
    MTH3D_tdstVector_ offset;
    f32 dx, dy, dz;
    f32 distance, speed, accel, negSpeed, negAccel, half, time;
    f32 zero;

    normal = *(MTH3D_tdstVector_*) planeNormal;
    dz = particle->position.xZ - planePoint->xZ;
    dx = particle->position.xX - planePoint->xX;
    dy = particle->position.xY - planePoint->xY;
    distance = inv_MTH_fn_xMulAdd3(dx, planeNormal->xX, dy, planeNormal->xY, dz, planeNormal->xZ);
    distance = distance - D_00392B34;
    point = *(MTH3D_tdstVector_*) planePoint;
    speed = inv_MTH_fn_xMulAdd3(particle->velocity.xX, normal.xX, particle->velocity.xY, normal.xY,
                     particle->velocity.xZ, normal.xZ);
    accel = inv_MTH_fn_xMulAdd3(particle->acceleration.xX, normal.xX, particle->acceleration.xY,
                     normal.xY, particle->acceleration.xZ, normal.xZ);
    negSpeed = -speed;
    negAccel = -accel;
    MTH3D_M_vSetXYZ_inv(&offset, dx, dy, dz);
    zero = 0.0f;
    if (distance < zero) {
        distance = zero;
    }
    if (particle->flags & 0x100) {
        return flt_3934D0[0];
    }
    if (negAccel != zero) {
        half = negAccel * 0.5f;
        time = (speed + sqrtf(negSpeed * negSpeed - half * 4.0f * -distance)) / (half + half);
    } else {
        if (negSpeed == zero) {
            return flt_3934D0[0];
        }
        time = distance / negSpeed;
    }
    if (time < zero) {
        return flt_3934D0[0];
    }
    return time;
}

s32 inv_MEC_fn_bRestOnCollider(EffectParticle_inv* p_stDest, EffectSystem_inv* p_stSrc, CollisionNode* p_stCollider, MTH3D_tdstVector_* p_stNormal, MTH3D_tdstVector_* p_stFirst, MTH3D_tdstVector_* p_stSecond)
{
    MTH3D_tdstVector_ stEnd;
    MTH3D_tdstVector_ stOffset;
    MTH3D_tdstVector_ stPoint;
    f32 xLength;
    f32 xEndLength;
    f32 xOffsetLength;
    s32 bTest;
    CollisionShape* p_stShape;

    bTest = 1;
    stEnd = *p_stSecond;
    if (p_stCollider->type == 2) {
        return 0;
    }
    xLength = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(p_stFirst->xX, p_stFirst->xY, p_stFirst->xZ));
    if (p_stCollider->type == 1 || p_stCollider->type == 4 || p_stCollider->type == 8) {
        p_stShape = p_stCollider->data;
        if (p_stShape->inv_xRestSpeed < xLength) {
            bTest = 0;
        }
    }
    if (bTest != 0) {
        inv_MEC_fn_vSplitAlongNormal(&p_stSrc->acceleration, &stPoint, &stOffset, &stEnd);
        MTH3D_M_vAddXYZ_inv(&stEnd, &stEnd, &stOffset);
        xOffsetLength = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(stOffset.xX, stOffset.xY, stOffset.xZ));
        xEndLength = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(stEnd.xX, stEnd.xY, stEnd.xZ));
        if (xEndLength < xOffsetLength) {
            p_stDest->acceleration = stPoint;
            p_stDest->velocity = *p_stNormal;
            p_stDest->lastCollider = p_stCollider;
            return 1;
        }
        p_stDest->acceleration = p_stSrc->acceleration;
        p_stDest->lastCollider = 0;
        return 0;
    }
    if (p_stDest->lastCollider != p_stCollider) {
        p_stDest->lastCollider = 0;
        p_stDest->acceleration = p_stSrc->acceleration;
        return 0;
    }
    return 0;
}

s32 inv_MEC_fn_bMovesTowardPlane(EffectParticle_inv* particle, MTH3D_tdstVector_* planeNormal, f32 time)
{
    f32 direction[3];
    f32 zero;
    f32 x, y, z;
    f32 length;
    f32 dot;
    s32 result;

    zero = 0.0f;
    x = particle->acceleration.xX + zero;
    y = particle->acceleration.xY + zero;
    z = particle->acceleration.xZ + zero;
    direction[0] = time * x + particle->velocity.xX;
    direction[1] = time * y + particle->velocity.xY;
    direction[2] = time * z + particle->velocity.xZ;
    length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(direction[0], direction[1], direction[2]));
    if (length < D_00392B38) {
        result = 0;
    } else {
        direction[0] = direction[0] / length;
        direction[1] = direction[1] / length;
        direction[2] = direction[2] / length;
        result = 1;
        dot = inv_MTH_fn_xMulAdd3TiedY(planeNormal->xX, direction[0], planeNormal->xY, direction[1],
                                       planeNormal->xZ, direction[2]);
        if (zero < dot) {
            result = 0;
        }
    }
    return result;
}

s32 inv_MEC_fn_bIsPointBehindPlane(MTH3D_tdstVector_* p_stNormal, MTH3D_tdstVector_* p_stOrigin,
                  MTH3D_tdstVector_* p_stPoint)
{
    f32 xDelta[3];
    f32 dot;
    s32 result;

    xDelta[0] = p_stPoint->xX - p_stOrigin->xX;
    xDelta[1] = p_stPoint->xY - p_stOrigin->xY;
    xDelta[2] = p_stPoint->xZ - p_stOrigin->xZ;
    dot = inv_MTH_fn_xMulAdd3(xDelta[0], p_stNormal->xX, xDelta[1], p_stNormal->xY, xDelta[2],
                              p_stNormal->xZ);
    result = 1;
    if (dot < 0.0f) {
    } else {
        result = 0;
    }
    return result;
}

s32 inv_MEC_fn_bIntersectParticleWithPlane(EffectParticle_inv* particle, MTH3D_tdstVector_* segment, CollisionShape* shape,
                  f32* distance, MTH3D_tdstVector_* hit)
{
    f32 value;

    if (inv_MEC_fn_bIsPointBehindPlane(&shape->points[1], &shape->points[0], segment) == 0) {
        return 0;
    }

    value = inv_MEC_fn_xGetParticlePlaneDistance(particle, &shape->points[0], &shape->points[1]);
    D_00393F28 = value;
    if (value == flt_3934D4[0]) {
        return 0;
    }

    if (inv_MEC_fn_bMovesTowardPlane(particle, &shape->points[1], value) == 0) {
        return 0;
    }

    *hit = shape->points[1];
    *distance = D_00393F28;
    return 1;
}
