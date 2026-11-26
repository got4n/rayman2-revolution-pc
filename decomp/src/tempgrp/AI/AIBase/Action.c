#include "common.h"

#include "AI/AI.h"
#include "HIE/HIE.h"
#include "ps2/libc.h"

#include "MTH/MTH.h"

s32 strlen(char* string);

static inline s8 inv_AI_fn_cUpperCase(s8 c)
{
    s32 upper = c - 0x20;

    if ((inv_FMN_pathCharacterFlags[c] & 2) == 0)
        upper = c;
    return upper;
}

tdstNodeInterpret_* fn_p_stDefaultActionInit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    s8* szTypeParam;
    u8 ucNbParam;
    u8 ucParam;
    s32 value;
    f32 real;

    szTypeParam = func_001E6F40(AI_NODE_ID(p_stTree));
    ucNbParam = strlen(szTypeParam);
    if (ucNbParam == 0)
        return p_stTree;

    param = inv_AI_parameterPoolCursor++;
    for (ucParam = 0; ucParam < ucNbParam; ucParam++) {
        s8 cType;

        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        cType = inv_AI_fn_cUpperCase(szTypeParam[ucParam]);
        if (cType == 'L') {
            if (param->ucType == E_vt_Float)
                value = inv_MTH_fn_lToIntegerOrdered(param->u.xValue);
            else
                value = param->u.lValue;
            fn_vSetActionParamlValue(ucParam, p_SuperObjPerso, value);
        } else if (cType == 'X') {
            if (param->ucType == E_vt_Float)
                real = param->u.xValue;
            else
                real = param->u.lValue;
            fn_vSetActionParamxValue(ucParam, p_SuperObjPerso, real);
        } else if (cType == 'V') {
            fn_vSetActionParamp_stVertex(ucParam, p_SuperObjPerso, &param->u.stVector);
        } else if (cType == 'A') {
            fn_vSetActionParamStateValue(ucParam, p_SuperObjPerso, param->u.tdxstHandleToState);
        }
    }
    inv_AI_parameterPoolCursor = param;
    return p_stTree;
}
