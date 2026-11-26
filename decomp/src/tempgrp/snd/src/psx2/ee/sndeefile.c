#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"
#include "FMN/FMN.h"

s32 FMN_Read(s32 file, void* data, s32 count, s32 size);
s32 scePrintf(char* format, ...);
#ifdef PC_PORT
#define scePrintf printf
#endif
s32 fn_iLoadFileRemote(s32 file, s32 command, s32* output);

s32 fn_iLoadFileEE(s32 file, void** output)
{
    s32 size;
    void* buffer;
    s32 result;

    scePrintf("START fn_iLoadFileEE\n");
    result = -1;
    if (output != NULL) {
        size = FMN_Length(file);
        buffer = SND_fn_pvMallocSnd(size);
        if (buffer != NULL) {
            if (size == FMN_Read(file, buffer, 1, size)) {
                *output = buffer;
                result = 0;
            } else {
                SND_fn_vFreeSnd(buffer);
            }
        }
    }
    scePrintf("END fn_iLoadFileEE\n");
    return result;
}

s32 fn_iLoadFileIOP(s32 file, s32* output)
{
    scePrintf("START fn_iLoadFileIOP\n");
    return fn_iLoadFileRemote(file, 4, output);
}

void fn_vUnLoadFileIOP(s32 handle)
{
    s32 request[1];

    printf("START fn_vUnLoadFileIOP(%d)\n", handle);
    request[0] = handle;
    fn_iRemoteCall(0x20005, 1, NULL, 0, request, 4);
    printf("END fn_vUnLoadFileIOP(%d)\n", handle);
}

s32 fn_iLoadFileSPU(s32 file, s32* output)
{
    scePrintf("START fn_iLoadFileIOP\n");
    return fn_iLoadFileRemote(file, 2, output);
}

void fn_vUnLoadFileSPU(s32 handle)
{
    s32 request[1];

    request[0] = handle;
    fn_iRemoteCall(0x20003, 1, NULL, 0, request, 4);
}

s32 fn_iLoadFileCD(s32 file, s32* output)
{
    scePrintf("START fn_iLoadFileCD\n");
    return fn_iLoadFileRemote(file, 6, output);
}

s32 fn_iLoadFileRemote(s32 file, s32 command, s32* output)
{
    s32 query[2];
    s32 reply[2];
    s32 size;
    s32 result;

    result = -1;
    scePrintf("START fn_iLoadFileRemote\n");
    query[0] = FMN_Tell(file);
    size = FMN_Length(file);
    query[1] = size;
    scePrintf("fn_iLoadFileRemote does RPC : lsn = %d, size = %d\n", query[0], size);
    if (fn_iRemoteCall((command & 0xFFFF) | 0x20000, 1, reply, 8, query, 8) >= 0) {
        result = 0;
        printf("The file is loaded at 0x%.8x size %d\n", reply[0], reply[1]);
        *output = reply[0];
    }
    scePrintf("END fn_iLoadFileRemote\n");
    return result;
}
