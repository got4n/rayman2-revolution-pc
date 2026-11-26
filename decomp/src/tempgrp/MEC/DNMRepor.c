#include "common.h"

#include "MEC/MEC.h"

#include "ps2/libc.h"

DNM_stReport_* DNM_p_stReportDefaultInit(DNM_stReport_* _p_stReport)
{
    memset(_p_stReport, 0, sizeof(DNM_stReport_));
    return _p_stReport;
}
