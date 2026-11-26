#include "common.h"

#include "SVG/SVG.h"
#include "FMN/FMN.h"
#include "AI/AI.h"
#include "TMR/TMR.h"
#include "GAM/GAM.h"

#define MC_TRIES 3

#define MC_RETRY(call)                                                                             \
    for (tries = MC_TRIES, gs_bSceMcOpIsSuccessful = 0; tries != 0 && !(gs_bSceMcOpIsSuccessful = (call) == 0);) {           \
        if (--tries != 0) {                                                                        \
            fn_vTimerWait(1);                                                                      \
        }                                                                                          \
    }

typedef struct SjisRange {
    u16 sjisFirst;
    u16 asciiFirst;
} SjisRange;

extern SjisRange off_331400[];
extern u16 word_331410[];

s32 FMN_Read(s32 file, void* buffer, s32 length, s32 count);
#ifdef PC_PORT
#include "PC/PC.h"

#define PC_CARD_FREE_KB 8000
#else
s32 gu_sceMcWrite(s32 fd, void* buffer, s32 size);
s32 sceMcClose(s32 fd);
s32 sceMcDelete(s32 port, s32 slot, char* path);
s32 sceMcFormat(s32 port, s32 slot);
s32 sceMcGetDir(s32 port, s32 slot, char* path, s32 mode, s32 maxEntries, sceMcTblGetDir* entries);
s32 sceMcInit(void);
s32 sceMcOpen(s32 port, s32 slot, char* path, s32 mode);
s32 sceMcRead(s32 fd, void* buffer, s32 size);
s32 sceMcSync(s32 mode, s32* command, s32* result);
s32 sceMcUnformat(s32 port, s32 slot);
#endif

s32 inv_SVG_fn_lGetModificationTimeInSeconds(sceMcTblGetDir* times)
{
    s32 months = times->_Modify.Month + (times->_Modify.Year - 2000) * 12;
    s32 days = times->_Modify.Day + months * 31;
    s32 hours = times->_Modify.Hour + days * 24;
    s32 minutes = times->_Modify.Min + hours * 60;

    return times->_Modify.Sec + minutes * 60;
}

s32 func_0013F998(void)
{
    s32 count = 0;
    s32 i;

    for (i = 0; i < SAVE_ICON_COUNT; i++) {
        if (off_3313D8[i].size != 0 && off_3313D8[i].data != NULL) {
            count++;
        }
    }
    return count;
}

#ifdef PC_PORT
s32 SVG_fn_s32Format(s32 _iPort, s32 _iSlot)
{
    return 1;
}

s32 inv_SVG_fn_bUnformatMemoryCard(s32 _iPort, s32 _iSlot)
{
    return 1;
}

s32 SVG_fn_s32LoadFromFile(s32 _iPort, s32 _iSlot, char* path, void* buffer, s32 size)
{
    return pc_save_read(path, buffer, size);
}

s32 SVG_fn_s32WriteToFile(s32 _iPort, s32 _iSlot, char* path, void* buffer, s32 size)
{
    return pc_save_write(path, buffer, size);
}
#else
s32 SVG_fn_s32Format(s32 _iPort, s32 _iSlot)
{
    s32 tries;
    s32 result;

    MC_RETRY(sceMcFormat(_iPort, _iSlot));
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &result);
        return result == 0;
    }
    return 0;
}

s32 inv_SVG_fn_bUnformatMemoryCard(s32 port, s32 slot)
{
    s32 tries;
    s32 result;

    MC_RETRY(sceMcUnformat(port, slot));
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &result);
        return result == 0;
    }
    return 0;
}

s32 SVG_fn_s32LoadFromFile(s32 port, s32 slot, char* path, void* buffer, s32 size)
{
    s32 fd = -1;
    s32 result;
    s32 tries;
    s32 ok = 0;
    sceMcTblGetDir* entry;

    entry = (sceMcTblGetDir*) (((u32) __builtin_alloca(0x80) + 0x3F) & 0xFFFFFFC0);

    MC_RETRY(sceMcGetDir(port, slot, path, 0, 1, entry));
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &result);
        if (result == 1 && size == entry->FileSizeByte) {
            MC_RETRY(sceMcOpen(port, slot, path, 1));
            if (gs_bSceMcOpIsSuccessful != 0) {
                sceMcSync(0, NULL, &fd);
                if (fd >= 0) {
                    MC_RETRY(sceMcRead(fd, buffer, entry->FileSizeByte));
                    if (gs_bSceMcOpIsSuccessful != 0) {
                        sceMcSync(0, NULL, &result);
                        ok = result == entry->FileSizeByte;
                    }
                    MC_RETRY(sceMcClose(fd));
                    if (gs_bSceMcOpIsSuccessful != 0) {
                        sceMcSync(0, NULL, &result);
                    }
                }
            }
        }
    }
    return ok;
}

s32 SVG_fn_s32WriteToFile(s32 _iPort, s32 _iSlot, char* _szFileName, void* _pData, s32 _u32DataSize)
{
    s32 iFd = -1;
    s32 result;
    s32 tries;

    MC_RETRY(sceMcOpen(_iPort, _iSlot, _szFileName, 0x202));
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &iFd);
        if (iFd < 0) {
            return 0;
        }

        MC_RETRY(gu_sceMcWrite(iFd, _pData, _u32DataSize));
        if (gs_bSceMcOpIsSuccessful != 0) {
            sceMcSync(0, NULL, &result);
            if (result == _u32DataSize) {
                MC_RETRY(sceMcClose(iFd));
                if (gs_bSceMcOpIsSuccessful != 0) {
                    sceMcSync(0, NULL, &result);
                    if (result == 0) {
                        return 1;
                    }
                }
            }
        }
    }

    if (iFd < 0) {
        return 0;
    }
    MC_RETRY(sceMcClose(iFd));
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &result);
    }
    MC_RETRY(sceMcDelete(_iPort, _iSlot, _szFileName));
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &result);
    }
    return 0;
}
#endif

#ifdef PC_PORT
s32 SVG_fn_s32Query(s32 port, s32 slot, s32* info1)
{
    u8 saved = inv_SVG_bDirectoryValid;

    inv_SVG_bDirectoryValid = 0;
    if (inv_DEMO_bPlaying || port != 0 || slot != 0) {
        return 0;
    }
    if (info1 != NULL) {
        *info1 = PC_CARD_FREE_KB;
    }
    inv_SVG_bDirectoryValid = saved;
    return 2;
}
#else
s32 sceMcGetInfo(s32 port, s32 slot, s32* info0, s32* info1, s32* info2);

s32 SVG_fn_s32Query(s32 _iPort, s32 _iSlot, s32* _p_iFreeClusters)
{
    s32 tries;
    s32 info0;
    s32 info2;
    s32 status;
    u8 saved = inv_SVG_bDirectoryValid;

    inv_SVG_bDirectoryValid = 0;
    if (inv_DEMO_bPlaying)
        return 0;
    MC_RETRY(sceMcGetInfo(_iPort, _iSlot, &info0, _p_iFreeClusters, &info2));
    if (!gs_bSceMcOpIsSuccessful)
        return 0;
wait:
    if (sceMcSync(1, NULL, &status) != 1)
        goto wait;
    if (info0 != 2)
        return 0;
    if (status >= -9) {
        if (status == -2)
            return 3;
        if (status == -1)
            return 2;
        if (status == 0) {
            inv_SVG_bDirectoryValid = saved;
            if (info2 != 0)
                return 2;
            return 3;
        }
    }
    return 0;
}
#endif

s32 SVG_fn_s32Init(void)
{
    void* buffer;
    s32 fd;
    s32 length;
    s32 i;

#ifndef PC_PORT
    if (sceMcInit() != 0) {
        return 0;
    }
#endif

    for (i = 0; i < SAVE_ICON_COUNT; i++) {
        fd = inv_FMN_fn_uOpenVirtualOrCD(off_3313D8[i].path, 1);
        if ((u32) fd < 15) {
            length = FMN_Length(fd);
            off_3313D8[i].data = AI_fn_p_vTrueAlloc(gp_6F74, length);
            if (off_3313D8[i].data != NULL) {
                off_3313D8[i].size = length;
                FMN_Read(fd, off_3313D8[i].data, length, 1);
            }
            FMN_Close(fd);
        }
    }
    return 1;
}

void SVG_fn_vAsciiToSJIS(u8* sjis, u16 length, char* ascii)
{
    u16 i;
    s32 c;
    s32 punctuationShift;
    s32 range;
    s32 first;
    u16 code;
    u8* p;

    p = sjis;
    for (i = 0; i < length; i++) {
        *p++ = 0x81;
        *p++ = 0x40;
    }

    i = 0;
    while (*ascii != '\0' && i < length) {
        range = 0x30;
        c = *ascii;
        punctuationShift = 0;
        ascii++;
        if (c >= ' ' && c <= '/') {
            punctuationShift = 1;
        } else if (c >= '0' && c <= '9') {
            range = 0;
        } else if (c >= ':' && c <= '@') {
            punctuationShift = 0xB;
        } else if (c >= 'A' && c <= 'Z') {
            range = 1;
        } else if (c >= '[' && c <= '`') {
            punctuationShift = 0x25;
        } else if (c >= 'a' && c <= 'z') {
            range = 2;
        } else if (c >= '{' && c <= '~') {
            punctuationShift = 0x3F;
        } else {
            return;
        }
        if (punctuationShift != 0) {
            first = punctuationShift + 0x1F;
            code = word_331410[c - first];
        } else {
            code = off_331400[range].sjisFirst + c - off_331400[range].asciiFirst;
        }
        sjis[1] = code;
        sjis[0] = code >> 8;
        i++;
        sjis += 2;
    }
}
