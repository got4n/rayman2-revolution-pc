#include "common.h"

#include "ps2/libc.h"

#include "GLI/GLI.h"
#include "renderware.h"

extern RwSky2DVertexFields dword_395530[4];
extern RwSky2DVertexFields dword_395570;
extern RwSky2DVertexFields flt_3955B0;
extern RwSky2DVertexFields flt_3955F0;

f32 RwCameraGetNearClipPlane(void* camera);

s32 RwIm2DRenderPrimitive(s32 type, RwSky2DVertexFields* vertices, s32 count);

void func_00153560(void)
{
    f32 farclip, far_z, rhw;
    u32 w, h;
    s32 dbg;
    RwSky2DVertexFields *p1, *p2, *p3, *p4;

    dbg = 0;
    farclip = RwCameraGetFarClipPlane(inv_GLI_primaryCamera);
    far_z = RwIm2DGetFarScreenZ();
    if (farclip != 0.0f)
        rhw = 1.0f / farclip;
    else
        rhw = 0.0f;

    w = *(u32*) &g_lVIDEO_WantedWidth;
    if (dbg)
        *(u32*) &dword_395530[0].pad1 = w;
    {
        RwSky2DVertexFields* v = &dword_395530[0];

        *(s32*) &dword_395530[0].scrVertex.x = 0;
        *(s32*) &v->scrVertex.y = 0;
        v->scrVertex.z = far_z;
        v->recipZ = rhw;
        *(s32*) &v->u = 0;
        *(s32*) &v->v = 0;
        do {
            *(s32*) &v->color.red = 0;
            *(s32*) &v->color.green = 0;
            *(s32*) &v->color.blue = 0;
            v->color.alpha = 255.0f;
        } while (0);
        *(s32*) &dword_395570.scrVertex.x = 0;
        h = g_lVIDEO_WantedHeight;
        p1 = v + 1;
    }
    {
        f32 t;
        RwSky2DVertexFields* v;

        do {
            t = h;
            v = &dword_395570;
        } while (0);
        p1->scrVertex.y = t;
        v->scrVertex.z = far_z;
        v->recipZ = rhw;
        *(s32*) &v->u = 0;
        *(s32*) &v->v = 0;
        do {
            *(s32*) &v->color.red = 0;
            *(s32*) &v->color.green = 0;
            *(s32*) &v->color.blue = 0;
            v->color.alpha = 255.0f;
        } while (0);
        p2 = v + 1;
    }
    {
        f32 t = g_lVIDEO_WantedWidth;
        RwSky2DVertexFields* v = &flt_3955B0;

        p2->scrVertex.x = t;
        *(s32*) &v->scrVertex.y = 0;
        v->scrVertex.z = far_z;
        v->recipZ = rhw;
        *(s32*) &v->u = 0;
        *(s32*) &v->v = 0;
        do {
            *(s32*) &v->color.red = 0;
            *(s32*) &v->color.green = 0;
            *(s32*) &v->color.blue = 0;
            v->color.alpha = 255.0f;
        } while (0);
        do {
            RwSky2DVertexFields* q = &flt_3955F0;

            if (dbg)
                *(s32*) &q->pad1 = 0;
        } while (0);
        do {
            p3 = v + 1;
        } while (0);
    }
    do {
        p3->scrVertex.x = g_lVIDEO_WantedWidth;
    } while (0);
    p4 = &flt_3955F0;
    {
        f32 t = g_lVIDEO_WantedHeight;
        RwSky2DVertexFields* v = &flt_3955F0;

        p4->scrVertex.y = t;
        v->recipZ = rhw;
        v->scrVertex.z = far_z;
        *(s32*) &v->u = 0;
        *(s32*) &v->v = 0;
        do {
            *(s32*) &v->color.red = 0;
            *(s32*) &v->color.green = 0;
            *(s32*) &v->color.blue = 0;
            v->color.alpha = 255.0f;
        } while (0);
    }

    printf("INFO --> pCamera_Game = 0x%X\n", inv_GLI_primaryCamera);
    printf("INFO --> FarClip = %f, FarScreen = %f\n", (double) farclip, (double) far_z);
    printf("INFO --> NearClip = %f, NearScreen = %f\n", (double) RwCameraGetNearClipPlane(inv_GLI_primaryCamera), (double) RwIm2DGetNearScreenZ());
}

void func_00153800(u8* color)
{
    s32 saved0C;
    s32 saved0E;
    RwSky2DVertexFields* vertex;
    u8 i = 0;

    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &saved0C);
    RwRenderStateGet(rwRENDERSTATEFOGENABLE, &saved0E);
    if (inv_GLI_zTestState == 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        inv_GLI_zTestState = 1;
    }
    if (inv_GLI_zWriteState != 0) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
        inv_GLI_zWriteState = 0;
    }
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*) 0);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);

    do {
        do {
            vertex = &dword_395530[i];
            vertex->color.red = color[0];
            vertex->color.green = color[1];
            vertex->color.blue = color[2];
            vertex->color.alpha = color[3];
        } while (0);
        i++;
    } while (i < 4);
    RwIm2DRenderPrimitive(4, dword_395530, 4);

    if (inv_GLI_zTestState == 0) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        inv_GLI_zTestState = 1;
    }
    if (inv_GLI_zWriteState == 0) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
    }
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) saved0C);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) saved0E);
}
