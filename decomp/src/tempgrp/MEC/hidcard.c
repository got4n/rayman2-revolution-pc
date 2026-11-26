#include "common.h"

#include "MEC/MEC.h"

s32 DNM_fn_eIdentityCardGetType(DNM_stMecBaseIdCard* _hCurrent)
{
    return _hCurrent->m_eIdentity;
}
