#include "common.h"

#include "ps2/libc.h"

s32 func_00178D28(void* material, void* pipeline);

void func_001518F0(void* material)
{
    if (func_00178D28(material, NULL) == 0)
        printf("ERROR --> Set Pipeline for Material[0x%X] FAILED!!\n", material);
    else
        printf("DEBUG --> Set Material[0x%X] back to DEFAULT PIPELINE\n", material);
}

void* func_00151940(void* material, void* pipeline)
{
    if (func_00178D28(material, pipeline) == 0)
        printf("ERROR --> Set Pipeline for Material[0x%X] FAILED!!\n", material);
    return material;
}
