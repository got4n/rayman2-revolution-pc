#include "common.h"
#include "snd/snd.h"

s32 fn_iConstructTdstBACKData(tdstBACKData* data)
{
    tdstBACKCall* call = &data->call;

    call->args[0] = 0;
    data->flags &= ~1;
    call->args[1] = 0;
    call->callback = NULL;
    return 0;
}

tdstBACKData* fn_u32AllocateTdstBACKData(tdstBACKData* data, tdstBACKCall* source)
{
    tdstBACKCall* call = &data->call;

    data->flags |= 1;
    call->args[0] = source->args[0];
    call->args[1] = source->args[1];
    call->callback = source->callback;
    return data;
}

s32 fn_u32DesallocateTdstBACKData(tdstBACKData* data)
{
    fn_iConstructTdstBACKData(data);
    return 0;
}

s32 fn_iOperateCallTdstBACKData(tdstBACKData* data)
{
    BACKCallback callback = data->call.callback;
    s32 result = -1;

    if (callback != NULL)
        result = callback(data->call.args[0], data->call.args[1]);
    return result;
}
