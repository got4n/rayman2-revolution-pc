#include "include_asm.h"
#include "snditbase_inv.h"

int g_bStereo_inv = 1;

void fn_vSplitTdstDesc_inv(Desc_inv *left, Desc_inv *right, Desc_inv *src)
{
    if (!(src->h.f.bFromCD < 0)) {
        if (left != 0) {
            CopyDesc_inv(left, src);
            left->h.cNbChannels = 1;
            left->u.b.stRight.u32Size = 0;
            left->u.b.stRight.pvVag = 0;
        }
        if (right != 0) {
            CopyDesc_inv(right, src);
            right->h.cNbChannels = 1;
            {
                SampleChan_inv *p = &right->u.b.stRight;
                right->u.b.stLeft.u32Size = p->u32Size;
                right->u.b.stLeft.pvVag = p->pvVag;
                p->u32Size = 0;
                p->pvVag = 0;
            }
        }
    } else {
        if (left != 0) {
            CopyDesc_inv(left, src);
            left->h.cNbChannels = 1;
            left->h.f.bInterleaved = 0;
        }
        if (right != 0) {
            CopyDesc_inv(right, src);
            right->h.cNbChannels = 1;
            right->h.f.bInterleaved = 0;
        }
    }
}

void fn_vGetOutParam_inv(tdstOutParam_inv *out, tdstPlayParam_inv *mix, DescHead_inv *src)
{
    DescHead_inv h;

    CopyHead_inv(&h, src);
    if (src->cNbChannels == 2) {
        h.f.bParamPan = 0;
        h.cNbChannels = 1;
    }
    if (h.cNbChannels == 1) {
        if (h.f.bParamPitch)
            out->pitch = (mix->lPitch * h.uwRate) >> 16;
        else
            out->pitch = h.uwRate;
        out->left = h.lVolume;
        out->right = h.lVolume;
        if (h.f.bParamVolume < 0) {
            out->left *= mix->ucVolume;
            out->right *= mix->ucVolume;
        } else {
            out->left <<= 7;
            out->right <<= 7;
        }
        if (h.f.bParamPan < 0) {
            out->left *= 127 - mix->ucPan;
            out->right *= mix->ucPan;
        } else {
            out->left <<= 8;
            out->right <<= 8;
        }
        if (h.f.bFromCD < 0) {
            out->left >>= 1;
            out->right >>= 1;
        }
    }
    out->left >>= 7;
    out->right >>= 7;
    if (out->left > 16000)
        out->left = 16000;
    if (out->right > 16000)
        out->right = 16000;
}

void fn_vSetStereo_inv(int x)
{
    g_bStereo_inv = x;
}

int fn_bGetStereo_inv(void)
{
    return g_bStereo_inv;
}

void fn_vMarkLoopBlocks_inv(unsigned char *buf, int size)
{
    unsigned char *p;
    unsigned char *end;

    buf[1] = 6;
    end = buf + (size - 15);
    for (p = buf + 0x11; p <= end; p += 16)
        *p = 2;
}

void fn_vMarkLastBlocks_inv(unsigned char *buf, int size)
{
    unsigned char *p;
    unsigned char *end;

    end = buf + (size - 15);
    for (p = buf + 1; p < end; p += 16)
        *p = 2;
    *end = 3;
}

int fn_u32ClearBlocks_inv(char *buf, int size)
{
    char *p = buf;
    char *end = buf + ((size + 15) & ~15);

    for (; p < end; p += 16) {
        ((int *)p)[0] = 12;
        ((int *)p)[1] = 0;
        ((int *)p)[2] = 0;
        ((int *)p)[3] = 0;
    }
    return p - buf;
}

unsigned int fn_u32GetFadeLength_inv(int which, Desc_inv *d)
{
    int x = (which * 1000) >> 16;
    unsigned int r = 0;

    if (d->h.lFormat == 4)
        r = (unsigned int)(x * d->h.uwRate) / 7000;
    return r;
}
