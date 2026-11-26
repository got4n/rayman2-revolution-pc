#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"
#include "FMN/FMN.h"

#define SND_BANK_SIGNATURE 0x32787370
#define SND_BIN_ID_INVALID 0xFFFFFFFF

#define SND_BANK_LOADED 1
#define SND_BANK_RESOLVE 2
#define SND_BANK_UNLOAD 3
#define SND_BANK_KEEP 4

typedef struct SndBank SndBank;

typedef struct SndBankList {
    SndBank* first;
    SndBank* last;
    s32 count;
} SndBankList;

typedef struct SndBankFiles {
    s32 hasVb : 1;
    s32 hasVsb : 1;
    s32 hasVsc : 1;
} SndBankFiles;

typedef struct SndBankData {
    u32 signature;
    s32 id;
    _SND_tdstBlockEvent* events;
    u32 eventCount;
    SndBinResource* resources;
    u32 resourceCount;
    u32 unk18[2];
    SndBankFiles files;
} SndBankData;

struct SndBank {
    s32 id;
    u32 pad04;
    u64 slotMask;
    u64 signature;
    s32 state;
    SndBankData* data;
    u8* scratch;
    u32 vbBase;
    u32 vsbBase;
    u32 vscBase;
    SndBank* next;
    SndBank* prev;
    SndBankList* owner;
    u32 pad3C;
};

typedef struct SndEditorIdRecord {
    u32 editorKey;
    s32 entryCount;
    s32 entryOffset;
} SndEditorIdRecord;

typedef struct SndEditorIdEntry {
    u64 editorSub;
    u64 binId;
} SndEditorIdEntry;

extern SndBankList SND_g_dBankDescriptionList;
extern SndEditorIdRecord* SND_gs_pEventGroupData;
extern s64 SND_gs_ulEventGroupCount;
extern char unk_393870[];
extern s32 D_003938A0;
extern s32 D_003938A4;
extern s32 D_00393F6C;
extern s32 D_00393F70;

s32 SND_fn_bUnLoadBinaryBank(s32 bankId);
s32 fn_bInitBinEvent(_SND_tdstBlockEvent* event);

static inline s32 inv_SND_fn_lMakeBankName(s32 bankId, char* extension, s32 capacity, char* output)
{
    char filename[0x100];
    s32 length;
    s32 result = -1;

    length = sprintf(filename, "BNK_%d.%s", bankId, extension);
    if (length + 1 < capacity) {
        strcpy(output, filename);
        result = length;
    }
    return result;
}

static inline s64 inv_SND_fn_lSlotBit(s32 slot)
{
    s32 one = 1;
    do {
        if (one == 1) {
            break;
        }
    } while (one == 0);
    return one << slot;
}

s32 fn_bInitBinEvent(_SND_tdstBlockEvent* event)
{
    switch (event->eType) {
    case 1:
    case 6:
    case 7:
    case 8:
        if ((u32) event->uParams.stPlay.uResource != SND_BIN_ID_INVALID) {
            event->uParams.stPlay.uResource = SND_fn_pGetBinRes((u32) event->uParams.stPlay.uResource);
        } else {
            event->uParams.stPlay.uResource = NULL;
        }
        break;
    case 2:
        if ((u32) event->uParams.stStop.uEvt.lIndex != SND_BIN_ID_INVALID) {
            event->uParams.stStop.uEvt.pstEvent = SND_fn_pGetBinEvent(event->uParams.stStop.uEvt.lIndex);
        } else {
            event->uParams.stStop.uEvt.pstEvent = NULL;
        }
        break;
    case 3:
    case 9:
        break;
    case 4:
        if ((u32) event->uParams.stStopNGo.uEvtStop.lIndex != SND_BIN_ID_INVALID) {
            event->uParams.stStopNGo.uEvtStop.pstEvent = SND_fn_pGetBinEvent(event->uParams.stStopNGo.uEvtStop.lIndex);
        } else {
            event->uParams.stStopNGo.uEvtStop.pstEvent = NULL;
        }
        if ((u32) event->uParams.stStopNGo.uEvtGo.lIndex != SND_BIN_ID_INVALID) {
            event->uParams.stStopNGo.uEvtGo.pstEvent = SND_fn_pGetBinEvent(event->uParams.stStopNGo.uEvtGo.lIndex);
        } else {
            event->uParams.stStopNGo.uEvtGo.pstEvent = NULL;
        }
        break;
    case 5:
        if ((u32) event->uParams.stStopNGoFade.uEvtStop.lIndex != SND_BIN_ID_INVALID) {
            event->uParams.stStopNGoFade.uEvtStop.pstEvent = SND_fn_pGetBinEvent(event->uParams.stStopNGoFade.uEvtStop.lIndex);
        } else {
            event->uParams.stStopNGoFade.uEvtStop.pstEvent = NULL;
        }
        if ((u32) event->uParams.stStopNGoFade.uEvtGo.lIndex != SND_BIN_ID_INVALID) {
            event->uParams.stStopNGoFade.uEvtGo.pstEvent = SND_fn_pGetBinEvent(event->uParams.stStopNGoFade.uEvtGo.lIndex);
        } else {
            event->uParams.stStopNGoFade.uEvtGo.pstEvent = NULL;
        }
        break;
    case 10:
        if ((u32) event->uParams.stExtraAll.uResource != SND_BIN_ID_INVALID) {
            event->uParams.stExtraAll.uResource = SND_fn_pGetBinRes((u32) event->uParams.stExtraAll.uResource);
        } else {
            event->uParams.stExtraAll.uResource = NULL;
        }
        break;
    default:
        SND_fn_vDisplayError(0x13, unk_393870);
        return 0;
    }
    return 1;
}

SndBank* fn_pstGetBankDescriptor(s32 bankId)
{
    SndBank* bank = SND_g_dBankDescriptionList.first;
    SndBank* result = NULL;

    while (bank != NULL) {
        if (bank->id == bankId) {
            result = bank;
            break;
        }
        bank = bank->next;
    }
    return result;
}

SndBank* fn_pstAddBankDescriptor(s32 bankId, s32 slot, SndBankData* data, s32 unused)
{
    SndBank* bank = NULL;
    SndBank* last;
    SndBankList* owner;

    if (data->signature == SND_BANK_SIGNATURE) {
        bank = SND_fn_pvMallocSndEx(0, sizeof(SndBank));
        if (bank != NULL) {
            memset(bank, 0, sizeof(SndBank));
            bank->owner = NULL;
            bank->next = NULL;
            owner = *(SndBankList* volatile*) &bank->owner;
            bank->prev = NULL;
            if (owner != NULL) {
                owner->count--;
                owner = bank->owner;
                if (owner->first == bank) {
                    owner->first = bank->next;
                    owner = bank->owner;
                }
                if (owner->last == bank) {
                    owner->last = bank->prev;
                }
            }
            bank->prev = NULL;
            bank->owner = NULL;
            bank->next = NULL;
            last = SND_g_dBankDescriptionList.last;
            bank->owner = &SND_g_dBankDescriptionList;
            bank->prev = last;
            if (SND_g_dBankDescriptionList.first == NULL) {
                SND_g_dBankDescriptionList.first = bank;
            } else {
                SND_g_dBankDescriptionList.last->next = bank;
            }
            SND_g_dBankDescriptionList.last = bank;
            SND_g_dBankDescriptionList.count++;
            bank->id = bankId;
            bank->data = data;
            bank->state = 0;
            bank->scratch = NULL;
            bank->vbBase = 0;
            bank->vsbBase = 0;
            bank->vscBase = 0;
            if (slot == -1) {
                bank->slotMask = 0xFFFFFFFF;
            } else {
                bank->slotMask = (u64) (1 << slot);
            }
        }
    }
    return bank;
}

s32 fn_bResolveBankPointer(SndBank* bank)
{
    SndBankData* data;
    _SND_tdstBlockEvent* event;
    SndBinResource* resource;
    SndBinResourceDisk_inv copy;
    u32 offset;
    s32 i;

    bank->signature = SND_BANK_SIGNATURE;
    data = bank->data;
    offset = (u32) data->events;
    event = (_SND_tdstBlockEvent*) ((u8*) data + offset);
    data->events = event;
    offset = (u32) bank->data->resources;
    resource = (SndBinResource*) ((u8*) bank->data + offset);
    bank->data->resources = resource;
    bank->scratch = (u8*) (bank->data->resources + bank->data->resourceCount);

    event = data->events;
    for (i = 0; i < (s32) data->eventCount; i++) {
        fn_bInitBinEvent(event);
        event++;
    }

    resource = data->resources;
    for (i = 0; i < (s32) data->resourceCount; i++) {
        u32 base;

        copy.header = resource->header;
        if (copy.header.kind == 1) {
            u8 flags = *(volatile u64*) &copy.header.sampleFlags;

            if ((s32) (flags << 25) >= 0) {
                if ((s32) (flags << 26) >= 0) {
                    base = bank->vbBase;
                } else {
                    base = bank->vsbBase;
                }
            } else {
                base = bank->vscBase;
            }
            SND_fn_bLoadResBinarySnd(&copy, resource, base);
        } else {
            SND_fn_bLoadResBinarySnd(&copy, resource, (u32) bank->scratch);
        }
        resource++;
    }

    bank->scratch = NULL;
    return 1;
}

SndEditorIdRecord* SND_fn_pstGetEventGroupData(void)
{
    return SND_gs_pEventGroupData;
}

void SND_fn_vSetEventGroupData(SndEditorIdRecord* map)
{
    SND_gs_pEventGroupData = map;
}

s32 SND_fn_ulGetEventGroupCount(void)
{
    return SND_gs_ulEventGroupCount;
}

void SND_fn_vSetEventGroupCount(u32 count)
{
    SND_gs_ulEventGroupCount = count;
}

s32 SND_fn_iGetNumberOfBanks(void)
{
    return SND_g_dBankDescriptionList.count;
}

u32 SND_fn_iGetNumberOfEventInBank(s32 bankId)
{
    SndBank* bank = fn_pstGetBankDescriptor(bankId);

    if (bank != NULL) {
        return bank->data->eventCount;
    }
    return 0;
}

void SND_fn_vInitBankSnd(char* name)
{
    SND_g_dBankDescriptionList.first = NULL;
    SND_g_dBankDescriptionList.last = NULL;
    SND_g_dBankDescriptionList.count = 0;
}

void SND_fn_vDesInitBankSnd(void)
{
    SndBank* bank;
    SndBank* next;

    for (bank = SND_g_dBankDescriptionList.first, next = bank != NULL ? bank->next : NULL;
         bank != NULL; bank = next, next = bank != NULL ? bank->next : NULL) {
        SND_fn_bUnLoadBinaryBank(bank->id);
    }
}

SndBinResource* SND_fn_pGetBinRes(u32 reference)
{
    SndBank* bank;
    SndBinResource* result = NULL;
    s32 index;

    if (reference != SND_BIN_ID_INVALID) {
        index = reference & 0xFFFF;
        bank = fn_pstGetBankDescriptor(reference >> 16);
        if (bank != NULL) {
            result = &bank->data->resources[index];
        }
    }
    return result;
}

s32 SND_fn_bStartLoadBanks2(s32 slot)
{
    SndBank* bank = SND_g_dBankDescriptionList.first;
    s64 mask;

    if (bank != NULL) {
        mask = ~((s64) (1 << slot));
        do {
            if (slot == -1) {
                bank->slotMask = 0;
            } else {
                bank->slotMask &= mask;
            }
            if (bank->slotMask == 0) {
                bank->state = SND_BANK_UNLOAD;
            }
            bank = bank->next;
        } while (bank != NULL);
    }
    return 1;
}

s32 SND_fn_bLoadBinaryBank(s32 bankId)
{
    char name[0x20];
    SndBank* selected;
    SndBank** selectedAddress;
    SndBank* bank;
    void* header;
    u32 vb;
    u32 vsb;
    u32 vsc;
    s32 file;
    s32 resourceFile;
    s32 dataId;
    s32 result;
    s32 loaded;

    selectedAddress = &selected;
    loaded = 0;
    bank = fn_pstGetBankDescriptor(bankId);
    if (bank == NULL) {
        if (inv_SND_fn_lMakeBankName(bankId, "BNM", 0x11, name) >= 0) {
            file = inv_FMN_fn_ulOpenVirtualOrFallback(name);
            if ((u32) file < 0xF) {
                if (fn_iLoadFileEE(file, &header) >= 0) {
                    bank = fn_pstAddBankDescriptor(bankId, -1, header, -1);
                    dataId = bank->data->id;
                    if (bank != NULL) {
                        if (bank->data->files.hasVb != 0) {
                            selected = bank;
                            if (inv_SND_fn_lMakeBankName(dataId, "VB", 0x11, name) >= 0) {
                                resourceFile = inv_FMN_fn_ulOpenVirtualOrFallback(name);
                                if ((u32) resourceFile < 0xF) {
                                    if (fn_iLoadFileSPU(resourceFile, &vb) >= 0) {
                                        loaded = 1;
                                        bank->vbBase = vb;
                                    }
                                    FMN_Close(resourceFile);
                                }
                            }
                        }
                        if (bank != NULL) {
                            if (bank->data->files.hasVsb != 0) {
                                *selectedAddress = bank;
                                if (inv_SND_fn_lMakeBankName(dataId, "VSB", 0x11, name) >= 0) {
                                    resourceFile = inv_FMN_fn_ulOpenVirtualOrFallback(name);
                                    if ((u32) resourceFile < 0xF) {
                                        if (fn_iLoadFileIOP(resourceFile, &vsb) >= 0) {
                                            loaded = 1;
                                            bank->vsbBase = vsb;
                                        }
                                        FMN_Close(resourceFile);
                                    }
                                }
                            }
                            if (bank != NULL) {
                                if (bank->data->files.hasVsc != 0) {
                                    *selectedAddress = bank;
                                    if (inv_SND_fn_lMakeBankName(dataId, "VSC", 0x11, name) >= 0) {
                                        resourceFile = inv_FMN_fn_ulOpenVirtualOrFallback(name);
                                        if ((u32) resourceFile < 0xF) {
                                            if (fn_iLoadFileCD(resourceFile, &vsc) >= 0) {
                                                loaded = 1;
                                                bank->vscBase = vsc;
                                            }
                                            FMN_Close(resourceFile);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                FMN_Close(file);
            }
        }
    } else {
        selected = bank;
    }
    result = loaded;
    if (selected != NULL) {
        if (selected->state != SND_BANK_LOADED) {
            selected->state = SND_BANK_KEEP;
            selected->state = SND_BANK_RESOLVE;
            if (fn_bResolveBankPointer(selected) != 0) {
                selected->state = SND_BANK_LOADED;
                result = 1;
            } else {
                SND_fn_bUnLoadBinaryBank(bankId);
            }
        }
    }
    return result;
}

s32 SND_fn_bInitBank2(s32 bankId, s32 slot)
{
    char name[0x20];
    SndBank* bank;
    SndBank* selected;
    SndBank** selectedAddress;
    void* header;
    u32 vb;
    u32 vsb;
    u32 vsc;
    s32 resourceFile;
    s32 loaded;
    u32 file;
    s32 state;
    s64 mask;

    selectedAddress = &selected;
    loaded = 0;
    bank = fn_pstGetBankDescriptor(bankId);
    if (bank == NULL) {
        if (inv_SND_fn_lMakeBankName(bankId, "BNM", 0x11, name) >= 0) {
            file = inv_FMN_fn_ulOpenVirtualOrFallback(name);
            if (file < 0xF) {
                if (fn_iLoadFileEE(file, &header) >= 0) {
                    bank = fn_pstAddBankDescriptor(bankId, slot, header, -1);
                    bankId = bank->data->id;
                    if (bank != NULL) {
                        if (bank->data->files.hasVb != 0) {
                            selected = bank;
                            if (inv_SND_fn_lMakeBankName(bankId, "VB", 0x11, name) >= 0) {
                                resourceFile = inv_FMN_fn_ulOpenVirtualOrFallback(name);
                                if ((u32) resourceFile < 0xF) {
                                    if (fn_iLoadFileSPU(resourceFile, &vb) >= 0) {
                                        loaded = 1;
                                        bank->vbBase = vb;
                                    }
                                    FMN_Close(resourceFile);
                                }
                            }
                        }
                        if (bank != NULL) {
                            if (bank->data->files.hasVsb != 0) {
                                *selectedAddress = bank;
                                if (inv_SND_fn_lMakeBankName(bankId, "VSB", 0x11, name) >= 0) {
                                    resourceFile = inv_FMN_fn_ulOpenVirtualOrFallback(name);
                                    if ((u32) resourceFile < 0xF) {
                                        if (fn_iLoadFileIOP(resourceFile, &vsb) >= 0) {
                                            loaded = 1;
                                            bank->vsbBase = vsb;
                                        }
                                        FMN_Close(resourceFile);
                                    }
                                }
                            }
                            if (bank != NULL) {
                                if (bank->data->files.hasVsc != 0) {
                                    *selectedAddress = bank;
                                    if (inv_SND_fn_lMakeBankName(bankId, "VSC", 0x11, name) >= 0) {
                                        resourceFile = inv_FMN_fn_ulOpenVirtualOrFallback(name);
                                        if ((u32) resourceFile < 0xF) {
                                            if (fn_iLoadFileCD(resourceFile, &vsc) >= 0) {
                                                loaded = 1;
                                                bank->vscBase = vsc;
                                            }
                                            FMN_Close(resourceFile);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                FMN_Close(file);
            }
        }
    } else {
        selected = bank;
    }

    state = loaded;
    if (state == 0) {
        if (slot == -1) {
            selected->slotMask = 0xFFFFFFFF;
        } else {
            mask = inv_SND_fn_lSlotBit(slot);
            selected->slotMask &= mask;
            selected->slotMask |= mask;
        }
    }

    state = selected->state;
    if (state == SND_BANK_LOADED) {
        selected->state = SND_BANK_LOADED;
    } else if (state != 0) {
        if (state == SND_BANK_UNLOAD) {
            selected->state = SND_BANK_LOADED;
        }
    } else {
        selected->state = SND_BANK_KEEP;
    }
    return 1;
}

s32 SND_fn_bEndLoadBanks(void)
{
    SndBank* bank;
    SndBank* next;

    g_bWorkingTheme = 0;
    D_003938A8 = 0;
    bank = SND_g_dBankDescriptionList.first;
    if (bank != NULL) {
        next = bank->next;
        do {
            switch ((u32) bank->state) {
            case SND_BANK_LOADED:
            case SND_BANK_RESOLVE:
                break;
            case SND_BANK_UNLOAD:
                SND_fn_bUnLoadBinaryBank(bank->id);
                break;
            case SND_BANK_KEEP:
                bank->state = SND_BANK_RESOLVE;
                break;
            }
            bank = next;
            if (bank == NULL) {
                break;
            }
            next = bank->next;
        } while (1);
    }

    for (bank = SND_g_dBankDescriptionList.first; bank != NULL; bank = bank->next) {
        switch (bank->state) {
        case SND_BANK_LOADED:
            break;
        case SND_BANK_RESOLVE:
            fn_bResolveBankPointer(bank);
            bank->state = SND_BANK_LOADED;
            break;
        }
    }
    return 1;
}

s32 SND_fn_bUnLoadBinaryBankAndData(s32 bankId)
{
    return SND_fn_bUnLoadBinaryBank(bankId);
}

s32 SND_fn_bUnLoadBinaryBank(s32 bankId)
{
    SndBank* bank;
    SndBinResource* resource;
    s32 i;

    bank = fn_pstGetBankDescriptor(bankId);
    if (bank != NULL) {
        if (bank->data != NULL) {
            resource = bank->data->resources;
            i = 0;
            if ((s32) bank->data->resourceCount > 0) {
                do {
                    SND_fn_vUnLoadResSnd(resource);
                    i++;
                    resource++;
                } while (i < (s32) bank->data->resourceCount);
            }
            if (bank->vbBase != 0) {
                fn_vUnLoadFileSPU(bank->vbBase);
                bank->vbBase = 0;
            }
            if (bank->vsbBase != 0) {
                fn_vUnLoadFileIOP(bank->vsbBase);
                bank->vsbBase = 0;
            }
            if (bank->vscBase != 0) {
                bank->vscBase = 0;
            }
            SND_fn_vFreeSnd(bank->data);
            bank->data = NULL;
        }

        if (bank->next != NULL) {
            bank->next->prev = bank->prev;
        }
        if (bank->prev != NULL) {
            bank->prev->next = bank->next;
        }
        if (bank->owner != NULL) {
            bank->owner->count--;
            if (bank->owner->first == bank) {
                bank->owner->first = bank->next;
            }
            if (bank->owner->last == bank) {
                bank->owner->last = bank->prev;
            }
        }
        bank->next = NULL;
        bank->prev = NULL;
        bank->owner = NULL;
        SND_fn_vFreeSndEx(0, bank);
    }
    return 0;
}

u64 SND_fn_ulGetBinIdFromEditorId(u64 editorId)
{
    SndEditorIdRecord* record;
    SndEditorIdRecord* recordEnd;
    SndEditorIdEntry* entry;
    SndEditorIdEntry* entryEnd;
    u64 editorSub;
    u64 key;

    if (SND_gs_pEventGroupData == NULL) {
        return SND_BIN_ID_INVALID;
    }
    key = editorId >> 16;
    editorSub = editorId & 0xFF;
    record = SND_gs_pEventGroupData;
    recordEnd = record + (s32) SND_gs_ulEventGroupCount;
    while (record < recordEnd) {
        if (record->editorKey == key) {
            entry = (SndEditorIdEntry*) ((u8*) record + record->entryOffset);
            entryEnd = entry + record->entryCount;
            while (entry < entryEnd) {
                if (entry->editorSub == editorSub) {
                    return entry->binId;
                }
                entry++;
            }
            return SND_BIN_ID_INVALID;
        }
        record++;
    }
    return SND_BIN_ID_INVALID;
}

void* func_0015F418(u32 editorId)
{
    return SND_fn_pGetBinEvent(SND_fn_ulGetBinIdFromEditorId(editorId));
}

u32 func_0015F448(void)
{
    return SND_BIN_ID_INVALID;
}

s32 func_0015F458(void)
{
    return 0;
}

void* SND_fn_pGetBinEvent(u32 reference)
{
    _SND_tdstBlockEvent* result = NULL;

    if (reference != SND_BIN_ID_INVALID) {
        u32 index = reference & 0xFFFF;
        SndBank* bank = fn_pstGetBankDescriptor(reference >> 16);

        if (bank != NULL) {
            SndBankData* data = bank->data;

            if (index < data->eventCount) {
                result = &data->events[index];
            }
        }
    }
    return result;
}

s32 func_0015F4D0(void)
{
    return 1;
}

u32 SND_fn_ulGetEventBinIdFromPointer(_SND_tdstBlockEvent* event)
{
    SndBank* bank;
    SndBankData* data;
    _SND_tdstBlockEvent* events;
    s32 count;
    s32 index;
    u64 handle;

    if (event == NULL) {
        return SND_BIN_ID_INVALID;
    }
    for (bank = SND_g_dBankDescriptionList.first; bank != NULL; bank = bank->next) {
        data = bank->data;
        index = event - data->events;
        count = data->eventCount;
        if (index >= 0 && index < count) {
            handle = ((u64) bank->id << 16) + (u64) index;
            return handle;
        }
    }
    return SND_BIN_ID_INVALID;
}

s32 fn_iGetDataFileName(s32 bankId, char* extension, s32 capacity, char* output)
{
    return inv_SND_fn_lMakeBankName(bankId, extension, capacity, output);
}

s32 fn_iRemoteEEInit(void)
{
    SndRpcEndpoint_inv server;
    SndRpcEndpoint_inv client;
    s32 result = -1;

    if (fn_iInitEECmdModule() >= 0) {
        server.rpcId = 0x696F3265;
        server.handler = func_001656B0;
        client.rpcId = 0x65653269;
        client.handler = NULL;
        result = func_00165D18(&D_003938A0, &server, &client);
    }
    return result;
}

s32 fn_iRemoteEEDesinit(void)
{
    s32 result = -1;
    s32 recovered = 0;
    s32 secondRecovered;

    if (D_003938A0 >= 0) {
        recovered = fn_iRecover(D_003938A0) >= 0;
        D_003938A0 = -1;
    }
    if (D_003938A4 >= 0) {
        secondRecovered = 0;
        if (recovered != 0) {
            secondRecovered = 0;
            if (fn_iRecover(D_003938A4) >= 0) {
                secondRecovered = 1;
            }
        }
        D_003938A4 = result;
        recovered = secondRecovered;
    }
    if (recovered != 0) {
        result = 0;
    }
    return result;
}

void func_0015F6C0(void)
{
    s32 cursor = D_00393F6C + 8;

    D_00393F6C = cursor;
    if (cursor >= D_00393F70) {
        D_003938A8 = 0;
    }
}

void func_0015F6E8(void)
{
    s32 cursor = D_00393F6C + 8;

    D_00393F6C = cursor;
    if (cursor >= D_00393F70) {
        D_003938A8 = 0;
    }
}
