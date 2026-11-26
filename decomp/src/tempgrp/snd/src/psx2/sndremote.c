#include "common.h"

#include "ps2/libc.h"
#include "sdk.h"
#include "snd/snd.h"

#include "include_asm.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#define SND_RPC_BUFFER_SIZE 0x400
#define SND_THREAD_BUFFER_SIZE 0x4000
#define SND_CLIENT_BIND_DELAY 10000

typedef struct gu_CommandChunkHeader {
    s32 command;
    s32 flags;
    u32 payloadSize;
} gu_CommandChunkHeader;

typedef struct SoundRpcRequest {
    u32 outputSize;
    u32 inputSize;
    u8 payload[SND_RPC_BUFFER_SIZE - 8];
} SoundRpcRequest;

typedef struct SoundRpcReply {
    s32 result;
    u8 payload[SND_RPC_BUFFER_SIZE - 4];
} SoundRpcReply;

typedef struct SoundThreadSetup {
    SndRpcEndpoint_inv endpoint;
    void* buffer;
    u32 bufferSize;
} SoundThreadSetup;

typedef struct SoundClientThreadArgs {
    SoundThreadSetup* setup;
    s32 threadId;
    s32 running;
} SoundClientThreadArgs;

typedef s32 (*SoundThreadEntry)(SoundClientThreadArgs* args);

extern sceSifClientData inv_SND_rpcClient;
extern SoundRpcRequest inv_SND_stRpcRequest;
extern SoundRpcReply inv_SND_stRpcReply;
extern u8 inv_SND_a_ucServerThreadBuffer[];
extern u8 inv_SND_a_ucClientThreadBuffer[];
extern u8 dword_39A6F0[] __attribute__((section(".sdata")));

s32 DeleteThread(s32 threadId);
s32 GetThreadId(void);
s32 fn_iServerLoop(SoundClientThreadArgs* args);
s32 fn_iLaunch(SoundThreadSetup* setup, SoundThreadEntry entry);
s32 sceSifBindRpc(sceSifClientData* client, s32 rpcId, s32 mode);
s32 sceSifCallRpc(sceSifClientData* client, s32 rpcId, s32 mode, void* send, s32 sendSize, void* receive,
                  s32 receiveSize, void* endFunction, void* endParameter);
s32 sceSifCheckStatRpc(sceSifRpcData* rpcd);
void WakeupThread(s32 threadId);

s32 fn_u32FormatCommandChunk(s32 chunk, gu_CommandOutput* out, const gu_CommandInput* in, s32 flags)
{
    u32 need = in->size + sizeof(gu_CommandChunkHeader);
    s32 written = 0;

    if (out->remaining >= need) {
        gu_CommandChunkHeader* header = (gu_CommandChunkHeader*) out->cursor;

        header->command = chunk;
        header->flags = flags;
        header->payloadSize = in->size;
        written = in->size;
        if (written != 0) {
            memcpy(header + 1, in->data, written);
            written = in->size;
        }
        written += sizeof(gu_CommandChunkHeader);
        out->remaining -= written;
        out->cursor += written;
    }
    return written;
}

#ifdef PC_PORT
s32 fn_iRemoteCall(s32 rpcId, s32 mode, void* output, s32 outputSize, void* input, s32 inputSize)
{
    u32 inputAligned = (inputSize + 3) & ~3;
    s32 result = -1;
    u32 outputAligned = (outputSize + 3) & ~3;
    u32 requestSize = inputAligned + 8;
    u32 replySize = outputAligned + 4;

    if (requestSize <= SND_RPC_BUFFER_SIZE && replySize <= SND_RPC_BUFFER_SIZE) {
        inv_SND_stRpcRequest.outputSize = outputSize;
        inv_SND_stRpcRequest.inputSize = inputSize;
        if (inputSize != 0) {
            memcpy(inv_SND_stRpcRequest.payload, input, inputSize);
            if (inputSize != inputAligned) {
                memset(&inv_SND_stRpcRequest.payload[inputSize], 0, inputAligned - inputSize);
            }
        }

        if (outputSize != 0) {
            memset(inv_SND_stRpcReply.payload, 0, outputAligned);
        }

        if (mode == 1) {
            pc_sound_command(rpcId, (PcAddress) &inv_SND_stRpcRequest, requestSize, (PcAddress) &inv_SND_stRpcReply, replySize);
        }
        result = inv_SND_stRpcReply.result;
        if (outputSize != 0 && output != NULL) {
            memcpy(output, inv_SND_stRpcReply.payload, outputSize);
        }
        if (mode != 1) {
            pc_sound_command(rpcId, (PcAddress) &inv_SND_stRpcRequest, requestSize, (PcAddress) &inv_SND_stRpcReply, replySize);
        }
    }

    return result;
}
#else
s32 fn_iRemoteCall(s32 rpcId, s32 mode, void* output, s32 outputSize, void* input, s32 inputSize)
{
    u32 inputAligned = (inputSize + 3) & ~3;
    s32 result = -1;
    u32 outputAligned = (outputSize + 3) & ~3;
    u32 requestSize = inputAligned + 8;
    u32 replySize = outputAligned + 4;

    if (requestSize <= SND_RPC_BUFFER_SIZE && replySize <= SND_RPC_BUFFER_SIZE) {
        while (sceSifCheckStatRpc(&inv_SND_rpcClient.rpcd) != 0) {
        }

        inv_SND_stRpcRequest.outputSize = outputSize;
        inv_SND_stRpcRequest.inputSize = inputSize;
        if (inputSize != 0) {
            memcpy(inv_SND_stRpcRequest.payload, input, inputSize);
            if (inputSize != inputAligned) {
                memset(&inv_SND_stRpcRequest.payload[inputSize], 0, inputAligned - inputSize);
            }
        }

        if (outputSize != 0) {
            memset(inv_SND_stRpcReply.payload, 0, outputAligned);
        }

        if (sceSifCallRpc(&inv_SND_rpcClient, rpcId, (mode ^ 1) != 0, &inv_SND_stRpcRequest, requestSize,
                          &inv_SND_stRpcReply, replySize, NULL, NULL) >= 0) {
            result = inv_SND_stRpcReply.result;
            if (outputSize != 0 && output != NULL) {
                memcpy(output, inv_SND_stRpcReply.payload, outputSize);
            }
        }
    }

    return result;
}
#endif

typedef struct SoundRpcServerSetup {
    u8 queue[0x20];
    u8 server[0x50];
} SoundRpcServerSetup;

extern s32 inv_SND_ulRpcRequestCount;
extern s32 inv_SND_lRpcRequestsInProgress;
extern u8 inv_SND_a_ucRpcServerBuffer[];

void sceSifSetRpcQueue(void* queue, s32 threadId);
void sceSifRegisterRpc(void* server, s32 rpcId, s32 function, void* buffer, s32 a4, s32 a5,
                       void* queue);
void* sceSifGetNextRequest(void* queue);
void sceSifExecRequest(void* request);
void SleepThread(void);
s32 StartThread(s32 threadId, void* argument);

#ifdef PC_PORT
typedef void* (*SoundMessageHandler)(u32 function, void* message, u32 size);

s32 fn_iServerLoop(SoundClientThreadArgs* args)
{
    s32 clientThread = args->threadId;
    SoundMessageHandler handler = (SoundMessageHandler) args->setup->endpoint.handler;
    u32 function;
    u32 size;

    args->running = 0;
    pc_thread_wake(clientThread);
    pc_thread_set_priority(clientThread, 0x69);
    for (;;) {
        while (pc_sound_receive(&function, inv_SND_a_ucRpcServerBuffer, SND_RPC_BUFFER_SIZE, &size)) {
            inv_SND_lRpcRequestsInProgress++;
            inv_SND_ulRpcRequestCount++;
            handler(function, inv_SND_a_ucRpcServerBuffer, size);
            inv_SND_lRpcRequestsInProgress--;
        }
        pc_sound_wait();
    }
}

s32 fn_iClientLoop(SoundClientThreadArgs* args)
{
    pc_thread_wake(args->threadId);
    args->running = 0;
    return 0;
}

s32 fn_iGetCurrentThreadPriority(void)
{
    return pc_thread_priority(pc_thread_self());
}

s32 fn_iLaunch(SoundThreadSetup* setup, SoundThreadEntry entry)
{
    SoundClientThreadArgs args;
    s32 thread = pc_thread_create((PcAddress) entry, (PcAddress) setup->buffer, setup->bufferSize,
                                  fn_iGetCurrentThreadPriority() - 1);

    if (thread < 0) {
        return -1;
    }
    args.setup = setup;
    args.threadId = pc_thread_self();
    args.running = -1;
    pc_thread_start(thread, (u32) &args);
    pc_thread_sleep();
    return args.running;
}
#else
s32 fn_iServerLoop(SoundClientThreadArgs* args)
{
    SoundRpcServerSetup setup;
    SoundThreadSetup* thread = args->setup;
    s32 clientThread = args->threadId;
    s32 rpcId = thread->endpoint.rpcId;
    void* request;

    sceSifSetRpcQueue(setup.queue, GetThreadId());
    sceSifRegisterRpc(setup.server, rpcId, (s32) thread->endpoint.handler,
                      inv_SND_a_ucRpcServerBuffer, 0, 0, setup.queue);
    args->running = 0;
    WakeupThread(clientThread);
    ChangeThreadPriority(clientThread, 0x69);
    for (;;) {
        while ((request = sceSifGetNextRequest(setup.queue)) != NULL) {
            inv_SND_lRpcRequestsInProgress++;
            inv_SND_ulRpcRequestCount++;
            sceSifExecRequest(request);
            inv_SND_lRpcRequestsInProgress--;
        }
        SleepThread();
    }
}

s32 fn_iClientLoop(SoundClientThreadArgs* args)
{
    volatile s32 delay;
    s32 rpcId = args->setup->endpoint.rpcId;
    s32 threadId = args->threadId;

    do {
        if (sceSifBindRpc(&inv_SND_rpcClient, rpcId, 0) < 0) {
            while (1) {
            }
        }
        delay = SND_CLIENT_BIND_DELAY;
        do {
            delay--;
        } while (delay != -1);
    } while (inv_SND_rpcClient.serve == NULL);
    WakeupThread(threadId);
    args->running = 0;
    return 0;
}

s32 fn_iGetCurrentThreadPriority(void)
{
    ThreadParam status;

    ReferThreadStatus(GetThreadId(), &status);
    return status.currentPriority;
}

s32 fn_iLaunch(SoundThreadSetup* setup, SoundThreadEntry entry)
{
    ThreadParam param;
    SoundClientThreadArgs args;
    s32 thread;
    s32 priority;
    s32 result;

    result = -1;
    priority = fn_iGetCurrentThreadPriority() - 1;
    param.status = 1;
    param.entry = entry;
    param.stack = setup->buffer;
    param.stackSize = setup->bufferSize;
    param.gpReg = dword_39A6F0;
    param.initPriority = priority;
    param.currentPriority = priority;
    param.attr = 0x2000000;
    param.option = 0;
    param.waitType = 0;
    param.waitId = 0;
    param.wakeupCount = 0;
    thread = CreateThread(&param);
    if (thread >= 0) {
        args = (SoundClientThreadArgs){ setup, GetThreadId(), result };
        StartThread(thread, &args);
        SleepThread();
        result = args.running;
    }
    return result;
}
#endif

s32 func_00165C88(const SndRpcEndpoint_inv* endpoint)
{
    SoundThreadSetup setup;

    setup.endpoint.rpcId = endpoint->rpcId;
    setup.endpoint.handler = endpoint->handler;
    setup.buffer = inv_SND_a_ucServerThreadBuffer;
    setup.bufferSize = SND_THREAD_BUFFER_SIZE;
    return fn_iLaunch(&setup, fn_iServerLoop);
}

s32 func_00165CD0(const SndRpcEndpoint_inv* endpoint)
{
    SoundThreadSetup setup;

    setup.endpoint.rpcId = endpoint->rpcId;
    setup.endpoint.handler = endpoint->handler;
    setup.buffer = inv_SND_a_ucClientThreadBuffer;
    setup.bufferSize = SND_THREAD_BUFFER_SIZE;
    return fn_iLaunch(&setup, fn_iClientLoop);
}

s32 func_00165D18(s32* threads, const SndRpcEndpoint_inv* server,
                  const SndRpcEndpoint_inv* client)
{
    s32 result = -1;

    threads[1] = func_00165C88(server);
    threads[0] = func_00165CD0(client);
    if (threads[1] >= 0 && threads[0] >= 0) {
        result = 0;
    }
    return result;
}

s32 fn_iRecover(s32 threadId)
{
#ifdef PC_PORT
    pc_thread_delete(threadId);
#else
    DeleteThread(threadId);
#endif
    return 0;
}

s32 fn_iRecoverServerClient(s32* threads)
{
    fn_iRecover(threads[0]);
    fn_iRecover(threads[1]);
    return 0;
}
