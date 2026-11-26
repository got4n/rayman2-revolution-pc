#include "common.h"

#include "HIE/HIE.h"
#include "SCT/SCT.h"
#include "GLI/GLI.h"
#include "GEO/GEO.h"

extern u32 inv_GLI_ulLightCommandState;

void inv_GLI_vRecordAddLightToViewport(GLI_tdstLight_* light)
{
    if (inv_GLI_openLightCommand == NULL) {
        u32* cursor = inv_GLI_lightCommandCursor;
        u32* header = cursor;

        inv_GLI_openLightCommand = cursor;
        cursor++;
        inv_GLI_lightCommandCursor = cursor;
        *header = 0;
    }
    *inv_GLI_lightCommandCursor++ = (u32) light;
    *inv_GLI_openLightCommand =
        ((*inv_GLI_openLightCommand & LIGHT_COMMAND_COUNT) + 1) | LIGHT_COMMAND_LIGHTS;
    GLI_vAddLightToViewport(NULL, light);
}

void inv_GLI_vRecordPopLightsInViewport(s32 count)
{
    s32 remaining;

    GLI_vPopLightsInViewport(NULL, count);
    if (inv_GLI_openLightCommand != NULL) {
        remaining = (*inv_GLI_openLightCommand & LIGHT_COMMAND_COUNT) - count;
        if (remaining <= 0) {
            inv_GLI_lightCommandCursor = inv_GLI_openLightCommand;
            inv_GLI_openLightCommand = NULL;
        } else {
            inv_GLI_lightCommandCursor -= count;
            *inv_GLI_openLightCommand = remaining | LIGHT_COMMAND_LIGHTS;
        }
    } else {
        if ((inv_GLI_lightCommandCursor[-1] & LIGHT_COMMAND_TAG) == LIGHT_COMMAND_LIGHT_LIST) {
            inv_GLI_lightCommandCursor--;
            count += *inv_GLI_lightCommandCursor & LIGHT_COMMAND_COUNT;
        }
        *inv_GLI_lightCommandCursor++ = count | LIGHT_COMMAND_LIGHT_LIST;
        inv_GLI_openLightCommand = NULL;
    }
}

void _GLI_vResetTableOfActiveLightsForChar(HIE_tdstSuperObject_* _hSprObjSector)
{
    u32* cursor = inv_GLI_lightCommandCursor;
    LightCommandValue sector;

    sector.superObject = _hSprObjSector;
    *cursor = LIGHT_COMMAND_FOG;
    inv_GLI_lightCommandCursor = ++cursor;
    *cursor = sector.word;
    inv_GLI_lightCommandCursor = cursor + 1;
}

void func_0014B380(void* viewport, u8 skipFiltered)
{
    POS_tdstMatrix_* savedMatrix = g_p_stCurrentMatrix;
    u32* command = inv_GLI_lightCommandBuffer;

    GLI_vClearListOfLightInViewport(viewport);
    if (command < inv_GLI_lightCommandCursor) {
        do {
            u32 word = *command;
            u32 tag = word & LIGHT_COMMAND_TAG;

            if (tag > LIGHT_COMMAND_LAST) {
                inv_GLI_ulLightCommandState = word;
                command++;
            } else if (!skipFiltered || inv_GLI_ulLightCommandState != LIGHT_COMMAND_STATE_SKIP) {
                switch (tag) {
                case LIGHT_COMMAND_MODULE: {
                    LightCommandValue module;
                    s32 drawMask;

                    command++;
                    module.word = *command++;
                    drawMask = *command++;
                    g_p_stCurrentMatrix = module.superObject->gu_hGlobalMatrix;
                    GEO_g_xZFarTransparencyLevel = *(f32*) command++;
                    inv_RW3_fn_vSetViewportLightMask(*command++);
                    fn_vSendOneModuleToViewPort(viewport, module.superObject, drawMask);
                    break;
                }
                case LIGHT_COMMAND_LIGHTS: {
                    u32 count = word & LIGHT_COMMAND_COUNT;

                    command++;
                    while (count-- != 0) {
                        GLI_vAddLightToViewport(viewport, (GLI_tdstLight_*) *command++);
                    }
                    break;
                }
                case LIGHT_COMMAND_LIGHT_LIST:
                    GLI_vPopLightsInViewport(viewport, word & LIGHT_COMMAND_COUNT);
                    command++;
                    break;
                case LIGHT_COMMAND_FOG: {
                    LightCommandValue sector;

                    command++;
                    sector.word = *command++;
                    SECT_fn_vSendFogToViewport(NULL, sector.superObject);
                    break;
                }
                }
            } else {
                switch (tag) {
                case LIGHT_COMMAND_LIGHTS:
                    command++;
                    command += word & LIGHT_COMMAND_COUNT;
                    break;
                case LIGHT_COMMAND_LIGHT_LIST:
                    command++;
                    break;
                case LIGHT_COMMAND_MODULE:
                    command += 5;
                    break;
                case LIGHT_COMMAND_FOG:
                    command += 2;
                    break;
                }
            }
        } while (command < inv_GLI_lightCommandCursor);
    }
    g_p_stCurrentMatrix = savedMatrix;
}
