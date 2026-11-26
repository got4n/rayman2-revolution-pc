#include "common.h"
#include "snd/snd.h"

#include "sdk.h"

#include "include_asm.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#define SND_INI_STATUS_ERROR -1
#define SND_INI_STATUS_PENDING 1
#define SND_INI_STATUS_DESINIT_DONE 2
#define SND_INI_STATUS_INIT_DONE 3

typedef struct SndIniData {
    s32 initPending : 1;
    s32 desinitPending : 1;
    s32 initDone : 1;
    s32 desinitDone : 1;
    s32 : 28;
    s32 reply;
    s32 command;
    s32 environment;
} SndIniData;

typedef struct SndIniMngr {
    SndIniData data[2];
    s32 threadId;
    u32 options[4];
} SndIniMngr;

typedef s32 (*SndIniCallback)(SndIniData* data);

extern SndIniMngr inv_SND_optionsManager;

s32 fn_iDestructTdstSndIniMngr(SndIniMngr* manager);
s32 fn_iConstructTdstSndIniMngr(SndIniMngr* manager, u32** options);

s32 fn_iConstructTdstSndIniData(SndIniData* data)
{
    data->initPending = 0;
    data->desinitPending = 0;
    data->environment = 0;
    data->initDone = 0;
    data->command = 0;
    data->desinitDone = 0;
    return 0;
}

#ifdef PC_PORT
s32 fn_iTreatFlagTdstSndIniData(SndIniData* data, void* unused)
#else
s32 fn_iTreatFlagTdstSndIniData(SndIniData* data)
#endif
{
    if (data->desinitPending && !data->desinitDone) {
        data->initDone = 0;
        data->desinitDone = 1;
    }
    if (data->initPending && !data->initDone) {
        data->initDone = 1;
        data->desinitDone = 0;
    }
    fn_u32DesallocateVoiceBACKEnv(data->environment);
    data->environment = 0;
    data->initPending = 0;
    data->desinitPending = 0;
    return 0;
}

s32 fn_iLaunchTdstSndIniData(SndIniData* data, s32 init, s32 command)
{
    tdstBACKCall call;
    s32 reply;
    s32 request;
    s32 environment;
    s32 result;

    call.args[0] = (s32) data;
    call.callback = (BACKCallback) fn_iTreatFlagTdstSndIniData;
    call.args[1] = 0;
    result = -1;
    environment = fn_u32AllocateVoiceBACKEnv(&call);
    if (environment != 0) {
        data->initPending = init;
        data->desinitPending = !init;
        data->environment = environment;
        data->command = command;
        request = environment;
        result = fn_iRemoteCall(command, 0, &reply, 4, &request, 4);
        data->reply = reply;
    }
    return result;
}

extern u32* inv_SND_p_ulIniOptions;

s32 fn_iConstructTdstSndIniMngr(SndIniMngr* manager, u32** options)
{
    SndRollOff range;
    s32 result;
    s32 ok;
    u32* source;
    u32* copy;

    result = -1;
    fn_iConstructTdstSndIniData(&manager->data[0]);
    fn_iConstructTdstSndIniData(&manager->data[1]);
#ifdef PC_PORT
    manager->threadId = pc_thread_self();
#else
    manager->threadId = GetThreadId();
#endif
    source = *options;
    if (source != NULL) {
        copy = manager->options;
        copy[0] = source[0];
        copy[1] = source[1];
        copy[2] = source[2];
        copy[3] = source[3];
        inv_SND_p_ulIniOptions = copy;
    } else {
        u32* cleared = manager->options;

        cleared[0] = 0;
        cleared[1] = 0;
        cleared[2] = 0;
        cleared[3] = 0;
    }
    ok = 0;
    if (fn_iInitBACKEnv() >= 0) {
        ok = fn_iRemoteEEInit() >= 0;
        ok = fn_iLaunchTdstSndIniData(&manager->data[1], 1, 0x10000) >= 0 && ok;
        ok = fn_iLaunchTdstSndIniData(&manager->data[0], 1, 0x20000) >= 0 && ok;
    }
    if (ok) {
        result = 1;
    }
    range.nearDistance = 0x7FFFFFFE;
    range.farDistance = 0x7FFFFFFE;
    SND_fn_vSetDefaultRollOff(&range);
    return result;
}

s32 fn_iDestructTdstSndIniMngr(SndIniMngr* manager)
{
    s32 result;

    {
        s32 init = 0;
        s32 command = 0x10001;

        __asm__("" : "+r"(manager), "+r"(command) : "r"(init));
        __asm__("" : "+r"(manager), "+r"(init));
        fn_iLaunchTdstSndIniData(&manager->data[1], init, command);
    }
    fn_iLaunchTdstSndIniData(&manager->data[0], 0, 0x20001);
    result = fn_iRemoteEEDesinit();
    {
        s32 status = 1;
        s32 failed = -1;
        u32 ok = (u32) ~result >> 31;

        if (!ok) {
            status = failed;
        }
        return status;
    }
}

s32 fn_iStatusInitTdstSndIniMngr(SndIniMngr* manager)
{
    s32 status = SND_INI_STATUS_INIT_DONE;

    if (!manager->data[0].initPending && !manager->data[0].initDone &&
        !manager->data[1].initPending && !manager->data[1].initDone) {
        status = SND_INI_STATUS_ERROR;
    } else if (manager->data[0].initPending || manager->data[1].initPending) {
        status = SND_INI_STATUS_PENDING;
    }
    return status;
}

s32 fn_iStatusDesinitTdstSndIniMngr(SndIniMngr* manager)
{
    s32 status = SND_INI_STATUS_DESINIT_DONE;

    if (!manager->data[0].desinitPending && !manager->data[0].desinitDone &&
        !manager->data[1].desinitPending && !manager->data[1].desinitDone) {
        status = SND_INI_STATUS_ERROR;
    } else if (manager->data[0].desinitPending || manager->data[1].desinitPending) {
        status = SND_INI_STATUS_PENDING;
    }
    return status;
}

s32 SND_fn_iInitOptions(u32** options)
{
    return fn_iConstructTdstSndIniMngr(&inv_SND_optionsManager, options);
}

s32 SND_fn_iDesInitOptions(void)
{
    return fn_iDestructTdstSndIniMngr(&inv_SND_optionsManager);
}

s32 SND_fn_iTestDesInitOptions(void)
{
    return fn_iStatusDesinitTdstSndIniMngr(&inv_SND_optionsManager);
}

s32 SND_fn_iTestInitOptions(void)
{
    return fn_iStatusInitTdstSndIniMngr(&inv_SND_optionsManager);
}

s32 fn_bIsInitRunningTdstSndIniData(SndIniData* data)
{
    return data->initPending;
}

s32 fn_bIsInitFailsTdstSndIniData(SndIniData* data)
{
    return !data->initPending && !data->initDone;
}

s32 fn_bIsDesinitRunningTdstSndIniData(SndIniData* data)
{
    return data->desinitPending;
}

s32 fn_bIsDesinitFailsTdstSndIniData(SndIniData* data)
{
    return !data->desinitPending && !data->desinitDone;
}
