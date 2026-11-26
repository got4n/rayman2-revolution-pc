#include "common.h"

#include "snd/snd.h"

typedef struct SndCmdInterface {
    SndCmdModule* modules;
    SndCmdModule* end;
} SndCmdInterface;

extern SndCmdInterface inv_SND_stCmdInterface;
extern u8 inv_SND_a_ucCmdOutputBuffer[];

s32 fn_iInitServerFunc(SndCmdModule* modules, SndCmdModule* end)
{
    inv_SND_stCmdInterface.modules = modules;
    inv_SND_stCmdInterface.end = end;
    return 0;
}

s32 func_00165600(u32 command, s32* output, s32 outputSize, s32* message, s32 messageSize)
{
    s16 index = command;
    u32 moduleId = command >> 16;
    s32 result = -1;
    SndCmdHandler handler = NULL;
    SndCmdModule* module;
    SndCmdModule* end;

    if (index >= 0) {
        end = inv_SND_stCmdInterface.end;
        for (module = inv_SND_stCmdInterface.modules; module < end; module++) {
            if (module->id == moduleId && module->lastCommand >= index) {
                handler = module->handlers[index];
                break;
            }
        }
    }
    if (handler != NULL) {
        s32 requested = message[0];
        s32 inputSize = message[1];

        if (requested <= outputSize - 8) {
            *output = handler(output + 1, requested, message + 2, inputSize);
            result = 0;
        }
    }
    return result;
}

u8* func_001656B0(u32 command, u8* message, s32 messageSize)
{
    s32 isBatch;
    u8* output;
    u32 count;
    s32 room;
    u8* result;
    u32* header;
    s32 ok;

    result = NULL;
    header = NULL;
    room = 0x400;
    output = inv_SND_a_ucCmdOutputBuffer;
    count = command & 0xFFFF;
    isBatch = (u16) (command >> 16) == 0xFFFF;

    do {
        if (isBatch) {
            message += 8;
            messageSize -= 12;
            header = (u32*) message;
            command = *(u32*) message;
            message += 4;
        }
        ok = func_00165600(command, (s32*) output, room, (s32*) message, messageSize) >= 0;
        if (!isBatch) {
            break;
        }
        count--;
        room -= header[1];
        output += header[1];
        messageSize -= header[2];
        message += header[2];
        if (count == 0) {
            break;
        }
    } while (ok);
    if (ok) {
        result = inv_SND_a_ucCmdOutputBuffer;
    }
    return result;
}
