#ifndef REVO_SVG_SVG_H
#define REVO_SVG_SVG_H

#include "ps2/types.h"

typedef struct SaveIconEntry {
    char* path;
    u32 size;
    void* data;
} SaveIconEntry;

#define SAVE_ICON_COUNT 3

extern SaveIconEntry off_3313D8[SAVE_ICON_COUNT];

typedef struct SaveSlotInfo {
    u32 gameDuration;
    s32 modificationTime;
    u16 slotIndex;
    u8 valid;
    char playerName[7];
    char name[10];
    char comment[12];
} SaveSlotInfo;

typedef struct SaveSlot {
    struct SaveSlot* next;
    u8 unk04[0x3C];
    SaveSlotInfo info;
    s16 id;
    u8 unk6A[0x16];
} __attribute__((aligned(8))) SaveSlot;

typedef struct SaveRecordEntry {
    u16 key;
    u16 offset;
} SaveRecordEntry;

typedef struct SaveGameData {
    u8 unk0000[0x10C];
    s32 volumes[3];
    SaveRecordEntry records[(0x2058 - 0x118) / 4];
    u16 recordCount;
    u16 streamSize;
    u8 header[0x20];
    u8 stream[0x9D7C - 0x207C];
} SaveGameData;

extern s32 func_00140840(s32 unused0, s32 unused1, s32 saveBytes);
extern s32 SVG_fn_s32DestroySaveGame(s32 _iPort, s32 _iSlot, char* _szSonyCompliantDirName);
extern u32 SVG_fn_u32SaveGame(s32 port, s32 slot, char* directory, char* playerName, char* comment,
                              u16 completion, u32 gameDuration, void* saveData, s32 saveSize);
extern SaveSlot* inv_SVG_fn_p_stScanAndLoadSaveDirectory(void);

typedef struct sceMcStDateTime {
    u8 Resv2;
    u8 Sec;
    u8 Min;
    u8 Hour;
    u8 Day;
    u8 Month;
    u16 Year;
} sceMcStDateTime;

typedef struct sceMcTblGetDir {
    sceMcStDateTime _Create;
    sceMcStDateTime _Modify;
    u32 FileSizeByte;
    u16 AttrFile;
    u16 Reserve1;
    u32 Reserve2;
    u32 PdaAplNo;
    char EntryName[0x20];
} sceMcTblGetDir;

extern SaveSlot inv_SAV_stCurrentSlot[];

extern s32 SVG_fn_s32Format(s32 _iPort, s32 _iSlot);
extern s32 SVG_fn_s32Init(void);
extern s32 SVG_fn_s32LoadFromFile(s32 port, s32 slot, char* path, void* buffer, s32 size);
extern s32 SVG_fn_s32WriteToFile(s32 _iPort, s32 _iSlot, char* _szFileName, void* _pData,
                                 s32 _u32DataSize);
extern s32 func_0013F998(void);
extern s32 SVG_fn_s32Query(s32 _iPort, s32 _iSlot, s32* _p_iFreeClusters);
extern void SVG_fn_vAsciiToSJIS(u8* sjis, u16 length, char* ascii);
extern s32 inv_SVG_fn_lGetModificationTimeInSeconds(sceMcTblGetDir* times);

extern u8 inv_SVG_bDirectoryValid;
extern u8 gs_bSceMcOpIsSuccessful;

#endif
