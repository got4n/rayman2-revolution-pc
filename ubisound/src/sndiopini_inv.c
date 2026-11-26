#include "include_asm.h"
#include "snditbase_inv.h"

extern char a_stIOPCmdModule_inv[];
extern int fn_iGetPlayInterface_inv();
extern int fn_iGetLoadInterface_inv();
extern int fn_iInitServerFunc();
extern int fn_iInitSpu_inv();

int fn_iInitIOPCmdModule_inv(void)
{
    int r = -1;

#ifdef PC_PORT
    pc_memset(a_stIOPCmdModule_inv, 0, 0x80);
#else
    sysclib_14_inv(a_stIOPCmdModule_inv, 0, 0x80);
#endif
    if (fn_iGetPlayInterface_inv(a_stIOPCmdModule_inv) >= 0 && fn_iGetLoadInterface_inv(a_stIOPCmdModule_inv + 0x10) >= 0
        && fn_iInitServerFunc(a_stIOPCmdModule_inv, a_stIOPCmdModule_inv + 0x80) >= 0 && fn_iInitSpu_inv() >= 0)
        r = 0;
    return r;
}
