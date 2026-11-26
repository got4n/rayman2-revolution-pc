#ifndef REVO_FMN_FMN_H
#define REVO_FMN_FMN_H

#include "ps2/types.h"

typedef struct Bigfile Bigfile;
typedef struct UbiDirectory UbiDirectory;

typedef struct UbiLink {
    void* value;
    struct UbiLink* next;
} UbiLink;

struct UbiDirectory {
    char* name;
    UbiDirectory* parent;
    UbiLink* directories;
    UbiLink* files;
};

typedef struct UbiFile {
    UbiDirectory* directory;
    char* name;
    s64 offset;
    u64 length;
    s64 unk18;
    u64 position;
    Bigfile* bigfile;
} UbiFile;

struct Bigfile {
    UbiDirectory* directory;
    UbiFile* current;
    s32 handle;
};

typedef struct CdFileLoc {
    u32 lsn;
    u32 size;
} CdFileLoc;

typedef struct FmnFileDescriptor {
    union {
        CdFileLoc* cd;
        UbiFile* archive;
    } handle;

    s32 position;
    s32 size;
    s32 kind;
} FmnFileDescriptor;

#define FMN_FILE_COUNT 15

extern FmnFileDescriptor inv_FMN_virtualFileTable[FMN_FILE_COUNT];

typedef void* (*FmnAllocFn)(u32 size);
typedef void (*FmnErrorFn)(void* buffer);

extern s32 FMN_Tell(s32 slot);
extern s32 inv_FMN_fn_uOpenVirtualOrCD(char* path, s32 mode);
extern void inv_FMN_fn_lOpenGameArchive(void);
extern s32 FMN_Length(s32 slot);
extern s32 inv_FMN_fn_lMountSoundBanks(void);
extern u8* FMN_ConvertPathUnix2Dos(char* path);
extern s32 FMN_Close(s32 slot);
extern u32 inv_FMN_fn_ulOpenVirtualOrFallback(char* path);
extern void* inv_FMN_fn_lOpenFileOrVirtual(char* path, FmnAllocFn allocate, FmnErrorFn onError);

extern void* inv_FMN_fn_lLoadFileFromCdWithCallback(char* path, FmnAllocFn allocate, FmnErrorFn onError);
extern s32 FMN_SCE_CD_Close(s32 slot);
extern s32 FMN_SCE_CD_Length(s32 slot);
extern s32 FMN_SCE_CD_Open(char* path, s32 mode);
extern s32 FMN_SCE_CD_Tell(s32 slot);
extern s32 gu_FMN_iFindVirtualHandle(void);
extern s32 inv_FMN_fn_lGetBigfileHandleSize(s32 slot);
extern s32 inv_FMN_fn_lOpenCdFallback(char* path);
extern s32 inv_FMN_fn_lFreeBigfileSlot(u32 slot);
extern u32 inv_FMN_fn_pGetBigfileHandlePtr(s32 slot);

extern void* inv_ubi_bigfile_p_vLoadFile(Bigfile* bigfile, char* path, FmnAllocFn allocate, FmnErrorFn onError);
extern UbiFile* ubi_bigfile_fopen(Bigfile* bigfile, char* path);
extern s32 ubi_bigfile_fclose(UbiFile* file);
extern s32 ubi_bigfile_flength(UbiFile* file);
extern s32 ubi_bigfile_fread(void* buffer, s64 size, s64 count, UbiFile* file);
extern s32 ubi_bigfile_ftell(UbiFile* file);
extern void inv_UBI_FILE_fn_vUnmountBigfileIfNotNull(Bigfile* bigfile);
extern s32 inv_ubi_bigfile_parseDescriptor(Bigfile* bigfile, char* path);
extern s32 ubi_bigfile_fseek(UbiFile* file, u64 offset, s32 origin);

extern UbiDirectory* ubi_directory_Factory(UbiDirectory* parent, char* name);
extern UbiFile* ubi_directory_findFile(UbiDirectory* directory, char* path);
extern void ubi_directory_destroy(UbiDirectory* directory);
extern void ubi_directory_insertDirectory(UbiDirectory* directory, UbiDirectory* child);
extern void ubi_directory_insertFile(UbiDirectory* directory, UbiFile* file);

struct COL_tdstCollideObject_;
struct POS_tdstMatrix_;
extern void ubi_file_destroy(UbiFile* record);
extern UbiFile* ubi_file_Factory(UbiDirectory* directory, char* name, u64 offset, u64 length,
                                 u64 unk18);
extern s32 inv_COL_fn_bSphereTouchesBoundingVolume(void* sphere, struct POS_tdstMatrix_* sphereMatrix, void* other,
                         struct POS_tdstMatrix_* otherMatrix, u8 translationOnly, u8 unused,
                         u8 otherIsBox);
extern void COL_fn_vCollideDynamicGeomObjWithStaticGeomObj(struct COL_tdstCollideObject_* _hGeometricObject1, struct POS_tdstMatrix_* _p_stT1DynamicGeomObjMatrix, struct POS_tdstMatrix_* _p_stT0DynamicGeomObjMatrix, struct COL_tdstCollideObject_* _hGeometricObject2, struct POS_tdstMatrix_* _p_stGeometricObjectMatrix2, void* _p_vParameter1, s16 _sParameter2, u8 _bStaticGeomObjHasNoTransformationMatrix, u8 _bDynamicGeomObjHasZoomInsteadOfScale);

extern s8 inv_FMN_bCdReadRetrying;

#endif
