#include "common.h"

#include "GAM/GAM.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "GLI/GLI.h"
#include "SCT/SCT.h"

extern s32 dword_355518[];

void inv_SHW_fn_vClearShadowData(Data3dShadow_inv* flags)
{
    memset(flags, 0, sizeof(Data3dShadow_inv));
}

void inv_SHW_fn_vInitShadowMode1(Data3dShadow_inv* flags)
{
    memset(flags, 0, sizeof(Data3dShadow_inv));
    flags->control.flags = 1;
}

void inv_SHW_fn_vInitShadowMode2(Data3dShadow_inv* flags)
{
    flags->control.flags = 2;
    flags->scaleX = 5.0f;
    flags->scaleY = 6.0f;
}

void inv_SHW_fn_vSetShadowIncluded(Data3dShadow_inv* flags, s32 enabled)
{
    if (enabled) {
        flags->control.flags |= 0x100;
    } else {
        flags->control.flags &= 0xFFFFFEFF;
    }
}

s32 inv_SHW_fn_bHasShadowMode(Data3dShadow_inv* flags)
{
    return flags->control.mode != 0 ? 1 : 0;
}

s32 inv_SHW_fn_bIsShadowMode2(Data3dShadow_inv* flags)
{
    return flags->control.mode == 2;
}

s32 inv_SHW_fn_bIsShadowMode1(Data3dShadow_inv* flags)
{
    return flags->control.mode == 1;
}

void inv_SHW_fn_vSetShadowScaleX(Data3dShadow_inv* flags, f32 value)
{
    if (inv_SHW_fn_bIsShadowMode2(flags) != 0) {
        flags->scaleX = value;
    }
}

void inv_SHW_fn_vSetShadowScaleY(Data3dShadow_inv* flags, f32 value)
{
    if (inv_SHW_fn_bIsShadowMode2(flags) != 0) {
        flags->scaleY = value;
    }
}

f32 inv_SHW_fn_xGetShadowScaleX(Data3dShadow_inv* flags)
{
    if (inv_SHW_fn_bIsShadowMode2(flags) != 0) {
        return flags->scaleX;
    }
    return 0.0f;
}

f32 inv_SHW_fn_xGetShadowScaleY(Data3dShadow_inv* flags)
{
    if (inv_SHW_fn_bIsShadowMode2(flags) != 0) {
        return flags->scaleY;
    }
    return 0.0f;
}

s32 inv_SHW_fn_ucGetShadowListKind(Data3dShadow_inv* flags)
{
    if (inv_SHW_fn_bIsShadowMode2(flags)) {
        return flags->source.listKind;
    }
    return 0;
}

void inv_SHW_fn_vSetShadowListKind(Data3dShadow_inv* flags, u8 value)
{
    if (inv_SHW_fn_bIsShadowMode2(flags) != 0) {
        flags->source.listKind = value;
    }
}

void inv_SHW_fn_vSetShadowOn(Data3dShadow_inv* flags, s32 enabled)
{
    if (enabled) {
        flags->control.flags |= 0x200;
    } else {
        flags->control.flags &= 0xFFFFFDFF;
    }
}

s32 inv_SHW_fn_bIsShadowOn(Data3dShadow_inv* flags)
{
    return (flags->control.flags >> 9) & 1;
}

void inv_SHW_fn_vSetFirstShadowMaterialCount(u32 count)
{
    if (count < 20) {
        dword_355518[0] = count;
    } else {
        dword_355518[0] = 20;
    }
}

void inv_SHW_fn_vSetSecondShadowMaterialCount(u32 count)
{
    if (count < 20) {
        dword_355570[0] = count;
    } else {
        dword_355570[0] = 20;
    }
}

void* inv_SHW_fn_hCreateShadowMaterial(void)
{
    GliMaterialData* material;
    GEO_tdstColor_ color;

    GLI_xCreateMaterial(&material);
    GLI_xSetMaterialType(material, 0x45D803F6);
    color.xR = 0.0f;
    color.xG = 0.0f;
    color.xB = 0.0f;
    color.xA = 0.0f;
    GLI_xSetMaterialColor(material, &color);
    color.xA = 255.0f;
    GLI_xSetMaterialAmbientCoef(material, &color);
    return material;
}
