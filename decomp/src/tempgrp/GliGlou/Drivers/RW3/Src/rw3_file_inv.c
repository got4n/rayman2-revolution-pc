#include "common.h"

#include "ps2/libc.h"
#include "renderware.h"

#include "include_asm.h"

#define inv_RW_FILE_BUFFER_SIZE 0x2800

typedef struct FileContext {
    s32 fd;
    s32 position;
    s32 size;
    u8 buffer[inv_RW_FILE_BUFFER_SIZE];
    u32 bufferPosition;
} FileContext;

typedef struct RwFileFunctions {
    void* rwfexist;
    void* rwfopen;
    void* rwfclose;
    void* rwfread;
    void* rwfwrite;
    void* rwfgets;
    void* rwfputs;
    void* rwfeof;
    void* rwfseek;
    void* rwfflush;
} RwFileFunctions;

typedef struct {
    RwFileFunctions table;
    RwFreeList* freeList;
} FileModuleGlobals;

extern s32 inv_RW_lOpenFileCount;
extern s32 D_00393454;
extern s32 inv_RW3_lFileModuleGlobalsOffset;
extern s32 inv_RW_lFileSystemPluginRefCount[1];

char *strchr(const char *s, s32 c);
char *strrchr(const char *s, s32 c);
char *strcat(char *dst, const char *src);
char *strupr(char *s);
s32 sceOpen(const char *path, s32 flags);
void sceClose(s32 fd);
s32 sceLseek(s32 fd, s32 offset, s32 whence);
s32 sceRead(s32 fd, void *buf, s32 n);
s32 sceWrite(s32 fd, const void *buf, s32 n);
RwFileFunctions* RwOsGetFileInterface(void);
void *inv_RW_fn_pFileSystemPluginOpen(void *instance, s32 offset, s32 size);

u32 inv_RW_fn_ulParseFileOpenMode(const char *path)
{
    char *pn;
    char *pplus;
    char *pa;
    char *pw;
    char *pr;
    char *pd;
    u32 mode;

    pr = strrchr(path, 'r');
    pw = strrchr(path, 'w');
    pa = strrchr(path, 'a');
    pplus = strrchr(path, '+');
    pn = strrchr(path, 'n');
    pd = strrchr(path, 'd');
    if (pplus != 0)
        mode = 3;
    else if (pr != 0)
        mode = 1;
    else if (pw != 0)
        mode = 2;
    else if (pa != 0)
        mode = 2;
    else
        return 0;
    mode = (pw != 0) ? (mode | 0x600) : mode;
    mode = (pa != 0) ? (mode | 0x200) : mode;
    mode = (pn != 0) ? (mode | 0x8000) : mode;
    if (pd != 0)
        mode |= 0x20000000;
    return mode;
}

void *inv_RW_fn_pFileOpen(const char *filename, const char *mode)
{
    char path[0x100];
    FileContext *file;
    s32 flags;
    RwGlobals_inv *globals;
    char *upper;
    char *scan;
    void *(*allocCallback)(void *);
    void *(*freeCallback)(void *, void *);
    s32 freeOffset;

    if (strchr(filename, ':') != NULL) {
        strncpy(path, filename, 0xFF);
    } else {
        *(u64 *)path = *(u64 *)"cdrom0:";
        upper = path + 7;
        strncpy(upper, filename, 0xF6);
        path[0xFD] = '\0';
        if (strrchr(path, ';') == NULL) {
            strcat(path, ";1");
        }
        strupr(upper);
    }

    path[0xFF] = '\0';
    scan = path;
    while (*scan != '\0') {
        if (*scan == '/') {
            *scan = '\\';
        }
        scan++;
    }

    globals = &ourGlobals;
    allocCallback = globals->freeListAlloc_inv;
    file = allocCallback(
        ((FileModuleGlobals *)((u8 *)globals + inv_RW3_lFileModuleGlobalsOffset))->freeList);
    if (file == NULL) {
        return NULL;
    }

    flags = inv_RW_fn_ulParseFileOpenMode(mode);
    if (flags == 0) {
        return NULL;
    }

    file->fd = sceOpen(path, flags);
    if (!(file->fd < 0)) {
        file->position = sceLseek(file->fd, 0, 2);
        file->size = file->position;
        if (!(file->position < 0)) {
            if ((flags & 0x600) == 0x200) {
                file->bufferPosition = inv_RW_FILE_BUFFER_SIZE;
                inv_RW_lOpenFileCount++;
                return file;
            }

            file->position = sceLseek(file->fd, 0, 0);
            if (file->position >= 0) {
                goto success;
            }
        }
        sceClose(file->fd);
    }
    freeOffset = inv_RW3_lFileModuleGlobalsOffset;
    freeCallback = globals->freeListFree_inv;
    freeOffset += (s32)globals;
    freeCallback(((FileModuleGlobals *)freeOffset)->freeList, file);
    return NULL;

success:
    file->bufferPosition = inv_RW_FILE_BUFFER_SIZE;
    inv_RW_lOpenFileCount++;
    return file;
}

void *inv_RW_fn_pFileOpenCallback(const char *filename, const char *mode)
{
    return inv_RW_fn_pFileOpen(filename, mode);
}

typedef s32 (*FileFreeFn)(void *freeList, FileContext *file, void *moduleGlobals);

s32 inv_RW_fn_iFileClose(FileContext *file)
{
    u8 *moduleGlobals;
    FileFreeFn freeCallback;

    if (file != NULL && inv_RW_lOpenFileCount != 0) {
        sceClose(file->fd);
        moduleGlobals = (u8 *)&ourGlobals + inv_RW3_lFileModuleGlobalsOffset;
        freeCallback = (FileFreeFn)ourGlobals.freeListFree_inv;
        freeCallback(((FileModuleGlobals *)moduleGlobals)->freeList, file, moduleGlobals);
        inv_RW_lOpenFileCount--;
        return 0;
    }
    return -1;
}

s32 inv_RW_fn_bFileExists(s32 fileName)
{
    s32 fp;

    fp = ((s32 (*)(s32, void *))((void **)RwOsGetFileInterface())[1])(fileName, "r");
    if (fp != 0) {
        fp = ((s32 (*)(s32))((void **)RwOsGetFileInterface())[2])(fp);
        return 1;
    }
    return 0;
}

u32 inv_RW_fn_iFileRead(void *addr, u32 size, u32 count, FileContext *fp)
{
    s32 numBytes;
    u32 bytesRead;
    u32 rest;
    s32 lRead;

    numBytes = size * count;
    bytesRead = 0;

    if (fp->position + numBytes > fp->size) {
        numBytes = fp->size - fp->position;
    }

    if (fp->bufferPosition < inv_RW_FILE_BUFFER_SIZE && numBytes > 0) {
        bytesRead = inv_RW_FILE_BUFFER_SIZE - fp->bufferPosition;
        if ((u32)numBytes < bytesRead) {
            bytesRead = numBytes;
        }
        memcpy(addr, &fp->buffer[fp->bufferPosition], bytesRead);
        addr = (u8 *)addr + bytesRead;
        fp->bufferPosition += bytesRead;
        fp->position += bytesRead;
    }

    if (numBytes != bytesRead) {
        rest = numBytes - bytesRead;
        if (rest >= inv_RW_FILE_BUFFER_SIZE) {
            lRead = sceRead(fp->fd, addr, rest);
            if (lRead < 0) {
                lRead = 0;
            }
            rest = lRead;
        } else {
            sceRead(fp->fd, fp->buffer, inv_RW_FILE_BUFFER_SIZE);
            memcpy(addr, fp->buffer, rest);
            fp->bufferPosition = rest;
        }
        bytesRead += rest;
        fp->position += rest;
    }

    return bytesRead / size;
}

s32 inv_RW_fn_iFileWrite(s32 buffer, s32 size, s32 count, FileContext *fp)
{
    s32 lWritten;
    s32 lPosition;

    lWritten = sceWrite(fp->fd, (void *) buffer, size * count);
    if (lWritten == -1) {
        return 0;
    }
    lPosition = fp->position + lWritten;
    fp->position = lPosition;
    if (fp->size < lPosition) {
        fp->size = lPosition;
    }
    if (size != 0) {
        return (s32)((u32)lWritten / (u32)size);
    }
    return 0;
}

s32 inv_RW_fn_iFileSeek(FileContext *fp, long offset, s32 origin)
{
    s32 oldPos;
    s32 bufStart;
    s32 noBuffer;

    oldPos = fp->position;
    bufStart = oldPos - fp->bufferPosition;
    noBuffer = 0;
    if (fp->bufferPosition == inv_RW_FILE_BUFFER_SIZE) {
        noBuffer = 1;
    }
    fp->bufferPosition = inv_RW_FILE_BUFFER_SIZE;

    switch (origin) {
    case 1:
        if (oldPos + offset >= bufStart && oldPos + offset < bufStart + inv_RW_FILE_BUFFER_SIZE) {
            fp->position += offset;
            fp->bufferPosition = fp->position - bufStart;
        } else {
            fp->position = sceLseek(fp->fd, oldPos + offset, 0);
        }
        break;
    case 2:
        fp->position = sceLseek(fp->fd, offset, 2);
        break;
    case 0:
        fp->position = sceLseek(fp->fd, offset, 0);
        break;
    default:
        return -1;
    }

    if (noBuffer) {
        fp->bufferPosition = inv_RW_FILE_BUFFER_SIZE;
    }

    if (fp->position == -1) {
        fp->position = oldPos;
        fp->bufferPosition = inv_RW_FILE_BUFFER_SIZE;
        return -1;
    }
    return 0;
}

s8 *inv_RW_fn_pFileGets(s8 *buf, s32 len, s32 ctx)
{
    s32 n;
    s32 i = 0;
    s8 *p;
    s32 nl;
    s32 cr;
    s32 left;

    n = inv_RW_fn_iFileRead(buf, 1, len - 1, (FileContext *) ctx);
    if (n == 0) {
        return (s8 *)0;
    }

    if (n > 0) {
        nl = 10;
        cr = 13;
        p = buf;

        while (i < n) {
            if (*p == nl) {
                ++i;
                buf[i] = 0;
                inv_RW_fn_iFileSeek((FileContext *) ctx, i - n, 1);
                return buf;
            }

            if (*p != cr) {
                ++p;
                ++i;
                goto test;
            }

            left = n - 1;
            {
                s32 v = i < left;
                s8 *next;

                if (!v) {
                    ++p;
                    ++i;
                    goto test;
                }

                next = p + ((v ^ 1) ^ v);
                v = p[1];

                if (v == nl) {
                    memcpy(p, next, (u32)(n - i - 1));
                    n = left;
                    goto test;
                } else {
                    ++p;
                    ++i;
                }
            }

test:
            ;
        }
    }

    return buf;
}

s32 inv_RW_fn_iFilePuts(char *string, FileContext *fp)
{
    s32 lLength;
    s32 lWritten;
    s32 lValue;
    s32 lPosition;

    lLength = strlen(string);
    lWritten = sceWrite(fp->fd, string, lLength);
    lValue = -1;
    if (lWritten == lValue) {
        return lValue;
    }
    lPosition = fp->position;
    lValue = fp->size;
    lPosition = lPosition + lWritten;
    fp->position = lPosition;
    if (lValue < lPosition) {
        fp->size = lPosition;
    }
    if (lLength != lWritten) {
        lValue = -1;
        return lValue;
    }
    lValue = lWritten;
    return lValue;
}

s32 func_0011D848(FileContext *fp)
{
    s32 bInRange;

    bInRange = fp->position < fp->size;
    return bInRange ^ 1;
}

s32 func_0011D860(void)
{
    return 0;
}

void *inv_RW_fn_pFileSystemPluginOpen(void *instance, s32 offset, s32 size)
{
    RwFileFunctions* table;
    RwFreeList* freeList;

    inv_RW3_lFileModuleGlobalsOffset = offset;
    freeList = RwFreeListCreate(0x2814, 5, 0);
    ((FileModuleGlobals *)((u8 *)&ourGlobals + inv_RW3_lFileModuleGlobalsOffset))->freeList =
        freeList;
    if (freeList == NULL) {
        return NULL;
    }
    table = RwOsGetFileInterface();
    ((FileModuleGlobals *)((u8 *)&ourGlobals + inv_RW3_lFileModuleGlobalsOffset))->table = *table;
    table->rwfexist = inv_RW_fn_bFileExists;
    table->rwfopen = inv_RW_fn_pFileOpenCallback;
    table->rwfclose = inv_RW_fn_iFileClose;
    table->rwfread = inv_RW_fn_iFileRead;
    table->rwfwrite = inv_RW_fn_iFileWrite;
    table->rwfgets = inv_RW_fn_pFileGets;
    table->rwfputs = inv_RW_fn_iFilePuts;
    table->rwfeof = func_0011D848;
    table->rwfseek = inv_RW_fn_iFileSeek;
    table->rwfflush = func_0011D860;
    inv_RW_lFileSystemPluginRefCount[0]++;
    return instance;
}

void *inv_RW_fn_pFileSystemPluginClose(void *object, s32 offset, s32 size)
{
    RwFileFunctions* table;
    s32 base;
    void *pObject;

    pObject = object;
    table = RwOsGetFileInterface();
    base = (s32)&ourGlobals;
    *table = *(RwFileFunctions*) (base + inv_RW3_lFileModuleGlobalsOffset);
    RwFreeListDestroy(((FileModuleGlobals *)(base + inv_RW3_lFileModuleGlobalsOffset))->freeList);
    inv_RW_lFileSystemPluginRefCount[0] = inv_RW_lFileSystemPluginRefCount[0] - 1;
    return pObject;
}

s32 inv_RW_fn_bFileSystemPluginAttach(void)
{
    s32 lOffset;
    s32 lInvalid;

    lInvalid = -1;
    lOffset = RwEngineRegisterPlugin(0x2C, D_00393454, inv_RW_fn_pFileSystemPluginOpen, inv_RW_fn_pFileSystemPluginClose);
    if (lInvalid < lOffset) {
        return 1;
    }
    return 0;
}
