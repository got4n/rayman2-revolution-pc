#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"

typedef struct AssociationList_inv {
    struct _Association* first;
    struct _Association* last;
    s32 count;
} AssociationList_inv;

typedef struct _Association {
    s32 channelActive;
    s32 channel;
    s32 object;
    s32 type;
    _SND_tdstBlockEvent* event;
    s32 unk14;
    s32 snd;
    SndBinResource* resource;
    s32 active;
    s32 detune;
    s32 handle;
    s32 unk2C;
    s32 unk30;
    struct _Association* next;
    struct _Association* prev;
    AssociationList_inv* list;
} Association;
extern s32 g_bInitDoneSound;
extern s32 gi_NbObjectType;
extern s32 inv_SND_objectSoundTypes;

struct VLNode;
void SND_fn_vDestroyAllMicroSound(void);
void erase_association(s32*);
void fn_vResetVolumeForObject(s32, s32);
void fn_vSetDefaultRetChannelSound(s32);
void fn_vUpdateTypeObjectSoundInfos(SND_tdstTypeObjectSoundInfos*, SND_tdstTypeObjectSoundInfos*);
void stop_un_son_objet(void*, s32, void*);
void desinit_object_type(void);
void inv_SND_fn_vFreeAllVolumeLines(void);
void fn_vChangeVolumeForType(s32, s32);
void stop_son_type_all(s32);
void inv_MOD_fn_vNoOp(void);

extern AssociationList_inv association;
extern gu_tdstMicroSoundHead micros_sound;
extern s32 gu_SND_fn_bReleaseDriverSound(void);
extern s32 gu_SND_fn_bRestoreDriverSound(void);

typedef struct SndObjectVolumeNodeView SndObjectVolumeNodeView;

typedef struct SndObjectVolumeListView {
    SndObjectVolumeNodeView* first;
    SndObjectVolumeNodeView* last;
    s32 count;
} SndObjectVolumeListView;

struct SndObjectVolumeNodeView {
    s32 volume;
    s32 objectKey;
    SndObjectVolumeNodeView* next;
    SndObjectVolumeNodeView* previous;
    SndObjectVolumeListView* volatile owner;
};

typedef s32 (*FillFn2)(s32, s32);

typedef struct FillObjectSoundInfoBlock {
    FillFn2 getRelP0X;
    FillFn2 getRelP0Y;
    FillFn2 getRelP0Z;
    FillFn2 getRelP1X;
    FillFn2 getRelP1Y;
    FillFn2 getRelP1Z;
    u32 unk18;
    s32 useDirectPosition;
} FillObjectSoundInfoBlock;

typedef union SND_tduObjectSoundInfo {
    SND_tstObjectSoundInfoBlock block;
    FillObjectSoundInfoBlock get;
} SND_tduObjectSoundInfo;

typedef struct VLList {
    struct VLNode* first;
    struct VLNode* last;
    s32 count;
} VLList;

typedef struct VLNode {
    s32 vol;
    s32 inv_volBeforeFade;
    struct VLNode* next;
    struct VLNode* prev;
    VLList* list;
} VLNode;

typedef struct SND_tdstObjectSoundRecord {
    SND_tdstTypeObjectSoundInfos callbacks;
    SND_tduObjectSoundInfo info;
    VLNode* volumeLine;
    SndObjectInfoCallback_inv infoCallback;
    SndRollOffCallback_inv rollOffHandler;
    s32 lastEvent;
    s32 volume;
    SndObjectVolumeListView volumes;
} SND_tdstObjectSoundRecord;

extern s32 dword_3D80E8[];

extern char unk_393858[];

extern Association* get_association_object(void* object, s32 typeIndex, void* channel);

extern void erase_association(s32* node);

extern s32 g_bDopplerActive;

extern s8 g_bUseNearestMicro;

void GetMicroParam(s32 obj, SxdMicroParams_inv* params);

SndFixedVector3* fn_vComputeRelToAbsCoord(SndFixedVector3* out, SndFixedVector3 base,
                                          SndFixedVector3 relative, SndFixedVector3 axisX,
                                          SndFixedVector3 axisY, SndFixedVector3 axisZ);

void get_RollOff(void* object, s32 type, _SoundParam* param);

void set_SwitchParam(s32 object, s32 type, u8* param);

extern s32 g_iNbActiveMicros;

void fill_pre_filled_param(s32 a0, s32 a1, s32 a2, _SoundParam* buf);

extern u8 g_VolumeLines[];

extern s32 nb_volume_lines;

extern s32 g_ulNbFadeOutTicks;
#ifdef PC_PORT
u32 fptoui(f32 x);
#endif


extern s32 SND_fn_lSendRequestChannelSound(s32 a0, s32 a1, s32 a2, _SND_tdstBlockEvent* req,
                                           s32 a4, SndEndCallback_inv a5);

extern s32 HandleCallback;

s32* get_association_channel(s32 channel, s32 object, s32 type, s32 event);

void inv_SND_fn_vNoOpSound3D(s32 lIndice);

void stop_all_son_objet(s32 a0, s32 a1);

void fn_vResetVolumeForObject(s32 a0, s32 a1);

void stop_son_type_but_all(s32 a0);

extern u32 qword_3704F0[8];

void fn_vUpdateTypeObjectSoundInfos(SND_tdstTypeObjectSoundInfos* dst,
                                    SND_tdstTypeObjectSoundInfos* src);

void fn_vRajAllVolumeForLine(s32* a0);

extern void fn_vChangeVolumeForObject(s32 a0, s32 a1, s32 a2);

s32 fn_lGetVolumeForObject(s32 a0, s32 a1);

void fn_vChangeVolumeForType(s32 a0, s32 a1);

s32 fn_lGetVolumeForType(s32 a0);

void fn_vResetVolumeForType(s32 a0);

extern s32 D_00393830;

void fnSetLastEventForSoundObjectType(s32 typeIndex, s32 event)
{
    if (typeIndex < gi_NbObjectType) {
        if (typeIndex >= 0) {
            ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].lastEvent = event;
        }
    }
}

s32 SND_fn_pstGetLastEventForObjectTypeSound(s32 typeIndex)
{
    if (typeIndex < gi_NbObjectType) {
        if (typeIndex >= 0) {
            return ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].lastEvent;
        }
    }
    return 0;
}

void fn_vResetVolumeForObject(s32 a0, s32 a1)
{
    s32 scale;
    s32 lValue;
    s32* p_lNode;
    s32 bFound;

    scale = 0x74;
    p_lNode = (s32*) inv_SND_objectSoundTypes;
    a1 = a1 * scale;
    bFound = 0;
    a1 = a1 + (s32) p_lNode;
    p_lNode = *(s32**) (a1 + 0x68);

L_loop:
    if (p_lNode != 0) {
        lValue = p_lNode[1];
        if (lValue == a0) {
            bFound = 1;
        } else {
            p_lNode = (s32*) p_lNode[2];
            goto L_loop;
        }
    }
    if (bFound == 0) {
        return;
    }

    a0 = p_lNode[2];
    if (a0 != 0) {
        lValue = p_lNode[3];
        *(s32*) (a0 + 0xC) = lValue;
        a0 = p_lNode[3];
    } else {
        a0 = p_lNode[3];
    }
    if (a0 != 0) {
        lValue = p_lNode[2];
        *(s32*) (a0 + 8) = lValue;
        a0 = p_lNode[4];
    } else {
        a0 = p_lNode[4];
    }
    if (a0 != 0) {
        {
            s32* tmp = (s32*) a0;
            lValue = tmp[2];
            lValue = lValue - 1;
            tmp[2] = lValue;
            a0 = p_lNode[4];
            lValue = *(s32*) a0;
            if (lValue == (s32) p_lNode) {
                lValue = p_lNode[2];
                *(s32*) a0 = lValue;
                a0 = p_lNode[4];
                lValue = *(s32*) (a0 + 4);
            } else {
                lValue = *(s32*) (a0 + 4);
            }
            if (lValue == (s32) p_lNode) {
                lValue = p_lNode[3];
                *(s32*) (a0 + 4) = lValue;
            }
        }
    }
    p_lNode[2] = 0;
    p_lNode[3] = 0;
    a0 = 0;
    p_lNode[4] = 0;
    SND_fn_vFreeSndEx(a0, p_lNode);
}

void fn_vChangeVolumeForType(s32 typeIndex, s32 volume)
{
    printf("change volume for type %d with vol %d\n", typeIndex, volume);
    ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].volume = volume;
}

void fn_vResetVolumeForType(s32 typeIndex)
{
    ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].volume = 0x7F;
    printf("reset volume for type %d with vol max %d\n", typeIndex, 0x7F);
}

void fn_vChangeVolumeForObject(s32 objectArg, s32 type, s32 volume)
{
    void* object = (void*) objectArg;

    SndObjectVolumeNodeView* node;
    SndObjectVolumeListView* list;
    SND_tdstObjectSoundRecord* table;
    s32 found;

    found = 0;
    if (volume == 0x7F) {
        fn_vResetVolumeForObject(objectArg, type);
        return;
    }

    {
        s32 addr;
        addr = type * 0x74;
        addr = addr + (s32) inv_SND_objectSoundTypes;
        node = ((SND_tdstObjectSoundRecord*) addr)->volumes.first;
    }
    while (node != 0) {
        if (node->objectKey == (s32) object) {
            found = 1;
            break;
        }
        node = node->next;
    }

    if (found == 0) {
        SndObjectVolumeListView* owner;
        node = SND_fn_pvMallocSndEx(0, 0x14);
        node->owner = 0;
        node->next = 0;
        node->previous = 0;
        owner = node->owner;
        if (owner != 0) {
            owner->count = owner->count - 1;
            owner = node->owner;
            if (owner->first == node) {
                owner->first = node->next;
                owner = node->owner;
            }
            if (owner->last == node) {
                owner->last = node->previous;
            }
        }

        node->previous = 0;
        node->owner = 0;
        node->next = 0;
        {
            SndObjectVolumeNodeView* tail1;
            SndObjectVolumeNodeView* head;
            SND_tdstObjectSoundRecord* entry;
            s32 offset;
            offset = type * 0x74;
            entry = (SND_tdstObjectSoundRecord*) (offset + (s32) inv_SND_objectSoundTypes);
            list = &entry->volumes;
            tail1 = list->last;
            node->owner = list;
            node->previous = tail1;
            head = entry->volumes.first;
            if (head == 0) {
                entry->volumes.first = node;
            } else {
                SndObjectVolumeNodeView* tail2;
                tail2 = list->last;
                tail2->next = node;
            }
        }

        {
            s32 addr;
            SndObjectVolumeListView* p;
            addr = type * 0x74;
            addr = addr + (s32) inv_SND_objectSoundTypes;
            addr = addr + 0x68;
            p = (SndObjectVolumeListView*) addr;
            p->last = node;
            p->count = p->count + 1;
        }
        node->objectKey = (s32) object;
    }
    node->volume = volume;
}

s32 fn_lGetVolumeForType(s32 typeIndex)
{
    return ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].volume;
}

s32 fn_lGetVolumeForObject(s32 object, s32 typeIndex)
{
    SndObjectVolumeNodeView* node;
    s32 found = 0;
    s32 volume;

    node = ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].volumes.first;
    while (node != 0) {
        if (node->objectKey == object) {
            found = 1;
            break;
        }
        node = node->next;
    }
    volume = 0x7F;
    if (found != 0) {
        volume = node->volume;
    }
    return volume;
}

void GetMicroParam(s32 obj, SxdMicroParams_inv* params)
{
    params->position.x = g_st_pfnRetMicroSound.X(obj);
    params->position.y = g_st_pfnRetMicroSound.Y(obj);
    params->position.z = g_st_pfnRetMicroSound.Z(obj);
    params->velocity.x = g_st_pfnRetMicroSound.VX(obj);
    params->velocity.y = g_st_pfnRetMicroSound.VY(obj);
    params->velocity.z = g_st_pfnRetMicroSound.VZ(obj);
    params->inv_stFrontAxis.x = g_st_pfnRetMicroSound.DNX(obj);
    params->inv_stFrontAxis.y = g_st_pfnRetMicroSound.DNY(obj);
    params->inv_stFrontAxis.z = g_st_pfnRetMicroSound.DNZ(obj);
    params->inv_stRightAxis.x = g_st_pfnRetMicroSound.DTX(obj);
    params->inv_stRightAxis.y = g_st_pfnRetMicroSound.DTY(obj);
    params->inv_stRightAxis.z = g_st_pfnRetMicroSound.DTZ(obj);
    if (params->inv_stFrontAxis.x == 0 && params->inv_stFrontAxis.y == 0 && params->inv_stFrontAxis.z == 0) {
        params->inv_stFrontAxis.x = 0x10000;
    }
    if (params->inv_stRightAxis.x == 0 && params->inv_stRightAxis.y == 0 && params->inv_stRightAxis.z == 0) {
        params->inv_stRightAxis.y = 0x10000;
    }
}

void* pre_associe_sound(void)
{
    Association* entry;

    if (dword_3D80E8[0] >= 0x64) {
        SND_fn_vDisplayErrorEx(0x12, unk_393858, 3);
        return 0;
    }
    entry = SND_fn_pvMallocSndEx(0, 0x40);
    entry->next = 0;
    entry->prev = 0;
    entry->list = 0;
    return entry;
}

void erase_association(s32* node)
{
    Association* entry = (Association*) node;
    Association* link;
    AssociationList_inv* owner;

    link = entry->next;
    if (link != 0) {
        link->prev = entry->prev;
    }
    link = entry->prev;
    if (link != 0) {
        link->next = entry->next;
    }
    owner = entry->list;
    if (owner != 0) {
        owner->count = owner->count - 1;
        owner = entry->list;
        if (owner->first == entry) {
            owner->first = entry->next;
            owner = entry->list;
        }
        if (owner->last == entry) {
            owner->last = entry->prev;
        }
    }
    entry->next = 0;
    entry->prev = 0;
    entry->list = 0;
    SND_fn_vFreeSndEx(0, node);
}

void associe_sound(Association* node, s32 a1, s32 object, s32 type, s32 t0, s32 snd,
                   s32 t2, SndBinResource* t3, s32 sp0, s32 sp1)
{
    Association* cur;
    Association* next;
    AssociationList_inv* list;
    Association* last;

    cur = association.first;
    if (cur != 0) {
        next = cur->next;
        if (cur->active != 0 && cur->snd != snd)
            goto L_advance;
        do {
            erase_association((s32*) cur);
        L_advance:
            cur = next;
            if (cur == 0)
                break;
            next = cur->next;
            if (cur->active != 0 && cur->snd != snd)
                goto L_advance;
        } while (1);
    }

    cur = association.first;
    while (cur != 0) {
        if (cur->object == object && cur->type == type) {
            break;
        }
        cur = cur->next;
    }

    if (cur != 0) {
        next = cur->next;

        if (node->next != 0) {
            node->next->prev = node->prev;
        }
        if (node->prev != 0) {
            node->prev->next = node->next;
        }
        list = node->list;
        if (list != 0) {
            list->count = list->count - 1;
            list = node->list;
            if (list->first == node) {
                list->first = node->next;
                list = node->list;
            }
            if (list->last == node) {
                list->last = node->prev;
            }
        }
        node->list = 0;

        node->next = next;
        node->prev = cur;
        if (cur != 0) {
            cur->next = node;
            node->list = cur->list;
        } else {
            next->list->first = node;
        }
        if (next != 0) {
            next->prev = node;
            node->list = next->list;
        } else {
            cur->list->last = node;
        }
        node->list->count = node->list->count + 1;
    } else {
        if (node->next != 0) {
            node->next->prev = node->prev;
        }
        if (node->prev != 0) {
            node->prev->next = node->next;
        }
        list = node->list;
        if (list != 0) {
            list->count = list->count - 1;
            list = node->list;
            if (list->first == node) {
                list->first = node->next;
                list = node->list;
            }
            if (list->last == node) {
                list->last = node->prev;
            }
        }
        node->prev = 0;
        node->list = 0;
        node->next = 0;

        last = association.last;
        node->list = &association;
        node->prev = last;
        if (association.first == 0) {
            association.first = node;
        } else {
            association.last->next = node;
        }
        association.last = node;
        association.count = association.count + 1;
    }

    node->channelActive = 1;
    node->channel = a1;
    node->object = object;
    node->type = type;
    node->unk14 = t0;
    node->snd = snd;
    node->event = (_SND_tdstBlockEvent*) t2;
    node->resource = t3;
    node->active = 1;
    node->detune = sp0;
    node->handle = sp1;
}

Association* get_association_object(void* object, s32 typeIndex, void* channel)
{
    Association* node;

    if (channel == (void*) 0) {
        node = association.first;
        while (node != 0) {
            if (node->active != 0 && node->object == (s32) object && node->type == typeIndex) {
                break;
            }
            node = node->next;
        }
    } else {
        node = association.first;
        while (node != 0) {
            if (node->active != 0 && node->object == (s32) object && node->event == channel &&
                node->type == typeIndex) {
                break;
            }
            node = node->next;
        }
    }
    return node;
}

s32* get_association_channel(s32 channel, s32 object, s32 type, s32 event)
{
    Association* node;

    if (event == 0) {
        node = association.first;
        while (node != 0) {
            if (node->active != 0 && node->channel == channel && node->object == object &&
                node->type == type) {
                break;
            }
            node = node->next;
        }
    } else {
        node = association.first;
        while (node != 0) {
            if (node->active != 0 && node->channel == channel && node->object == object &&
                node->event == (_SND_tdstBlockEvent*) event && node->type == type) {
                break;
            }
            node = node->next;
        }
    }
    return node;
}

void stop_all_son_objet(s32 object, s32 typeIndex)
{
    Association* node;

    while ((node = get_association_object(
                (void*) object, typeIndex, 0)) != 0) {
        if (node->event->bStopable != 0) {
            SND_fn_vStopSnd((PlaySndSlot*) node->snd);
        } else {
            annule_pre_associe_snd((PlaySndSlot*) node->snd);
        }
        erase_association((s32*) node);
    }
}

void stop_un_son_objet(void* ownerObject, s32 typeIndex, void* channel)
{
    Association* object;

    while ((object = get_association_object(ownerObject, typeIndex, channel)) != 0) {
        if (object->event->bStopable) {
            SND_fn_vStopSnd((PlaySndSlot*) object->snd);
        } else {
            invalidate_ptrvalid_snd((PlaySndSlot*) object->snd);
        }
        erase_association((s32*) object);
    }
}

void SND_fn_vStopSoundFromHandle(u32 ownerValue)
{
    Association* node;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x2CE);
    node = association.first;
    while (node != 0) {
        if (node->handle == ownerValue) {
            SND_fn_vStopSnd((PlaySndSlot*) node->snd);
            break;
        }
        node = node->next;
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void stop_son_type_all(s32 type)
{
    Association* entry;
    Association* next;

    for (entry = association.first, next = entry ? entry->next : NULL;
         entry != NULL; entry = next, next = entry ? entry->next : NULL) {
        if (entry->type == type) {
            if (entry->active)
                SND_fn_vStopSnd((PlaySndSlot*) entry->snd);
            erase_association((s32*) entry);
        }
    }
}

void stop_son_all(void)
{
    Association* entry;
    Association* next;

    for (entry = association.first, next = entry ? entry->next : NULL;
         entry != NULL; entry = next, next = entry ? entry->next : NULL) {
        if (entry->active)
            SND_fn_vStopSnd((PlaySndSlot*) entry->snd);
        erase_association((s32*) entry);
    }
    inv_SND_fn_vStopAllOwnedSnd();
}

void stop_son_type_but_all(s32 type)
{
    Association* entry;
    Association* next;

    for (entry = association.first, next = entry ? entry->next : NULL;
         entry != NULL; entry = next, next = entry ? entry->next : NULL) {
        if (entry->active && entry->type != type) {
            SND_fn_vStopSnd((PlaySndSlot*) entry->snd);
            erase_association((s32*) entry);
        }
    }
}

void get_RollOff(void* object, s32 recordAddress, _SoundParam* params)
{
    s32 flags;
    s32 index;
    SND_tdstObjectSoundRecord* entry;
    void* target;

    flags = params->flags;
    index = params->microCount;
    params->flags = flags & ~0x10;
    recordAddress = recordAddress * 0x74 + inv_SND_objectSoundTypes;
    entry = (SND_tdstObjectSoundRecord*) recordAddress;
    target = (char*) params + (index * 8 + 0x28);
    if (entry->rollOffHandler != 0) {
        if (entry->rollOffHandler(object, target) != 0)
            params->flags = params->flags | 0x10;
    }
}

void set_SwitchParam(s32 object, s32 typeIndex, u8* params)
{
    s32 flags = *(s32*) (params + 0x20);
    s32 extensionOffset = *(s32*) (params + 0x24);

    flags |= 0x20;
    extensionOffset <<= 3;
    *(s32*) (params + 0x20) = flags;
    extensionOffset += 0x28;
    params = params + extensionOffset;
    *(s32*) (params + 8) = typeIndex;
    *(s32*) (params + 0xC) = object;
}

void inv_MOD_fn_vNoOp(void)
{
}

s32 fn_bIsChannelActive(s32 a0, s32 superObject, s32 recordAddress)
{
    s32 (*callback)(s32, s32);
    s32 stride;

    stride = 0x74;
    recordAddress = recordAddress * stride;
    recordAddress = recordAddress + inv_SND_objectSoundTypes;
    callback = *(s32(**)(s32, s32))(recordAddress + 0x4C);
    return callback(a0, superObject);
}

SndFixedVector3* fn_vComputeRelToAbsCoord(SndFixedVector3* out, SndFixedVector3 base,
                                          SndFixedVector3 relative, SndFixedVector3 axisX,
                                          SndFixedVector3 axisY, SndFixedVector3 axisZ)
{
    SndFixedVector3* pRelative;
    SndFixedVector3* pAxisX;
    SndFixedVector3* pAxisY;
    SndFixedVector3* pAxisZ;
    SndFixedVector3 result;

    pRelative = &relative;
    pAxisX = &axisX;
    pAxisY = &axisY;
    pAxisZ = &axisZ;

    result.x = SND_fn_rMulRealRealSnd(pRelative->x, pAxisX->x) +
               SND_fn_rMulRealRealSnd(pRelative->y, pAxisY->x) +
               SND_fn_rMulRealRealSnd(pRelative->z, pAxisZ->x);
    result.y = SND_fn_rMulRealRealSnd(pRelative->x, pAxisX->y) +
               SND_fn_rMulRealRealSnd(pRelative->y, pAxisY->y) +
               SND_fn_rMulRealRealSnd(pRelative->z, pAxisZ->y);
    result.z = SND_fn_rMulRealRealSnd(pRelative->x, pAxisX->z) +
               SND_fn_rMulRealRealSnd(pRelative->y, pAxisY->z) +
               SND_fn_rMulRealRealSnd(pRelative->z, pAxisZ->z);

    result.x += base.x;
    result.y += base.y;
    result.z += base.z;

    *out = result;
    return out;
}

typedef s32 (*FillFn1)(s32);
typedef void (*FillVecFn)(s32, SndFixedVector3*);
typedef s32 (*FillMicroFn)(s32, gu_tdstMicroSoundNode*);

void fill_pre_filled_param(s32 object, s32 obj, s32 type, _SoundParam* param)
{
    SndFixedVector3 axisX;
    SndFixedVector3 axisY;
    SndFixedVector3 axisZ;
    SndFixedVector3 relative0;
    SndFixedVector3 base0;
    SndFixedVector3 relative1;
    SndFixedVector3 base1;
    SndFixedVector3 stAbsolute0;
    SndFixedVector3 stAbsolute1;
    f32 delta[3];
    SxdMicroParams_inv microParam;
    gu_tdstMicroSoundNode* micro;
    s32 index;
    s32 selectedMicroIndex;
    f32 selectedDistance;
    f32 distanceValue;
    SND_tdstObjectSoundRecord* record;
    s32 recordOffset0;
    s32 recordOffset1;

    param->flags = 0;

    if (object == -1) {
        if (obj != object) {
            recordOffset0 = type * 0x74;
            param->position.x = ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                             ->callbacks.callback00(obj);
            param->position.y = ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                             ->callbacks.callback04(obj);
            param->position.z = ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                             ->callbacks.callback08(obj);
            param->velocity.x = ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                             ->callbacks.callback0C(obj);
            param->velocity.y = ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                             ->callbacks.callback10(obj);
            param->velocity.z = ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                             ->callbacks.callback14(obj);
            param->reverb =
                (u8) ((SND_tdstObjectSoundRecord*) (recordOffset0 + inv_SND_objectSoundTypes))
                    ->callbacks.retReverb(obj);
            param->pan = 0x40;
            param->dolby = 0;
            if (g_bDopplerActive == 0)
                param->flags |= 2;
        } else {
            param->pan = 0x40;
            param->flags = 3;
            param->reverb = 0;
            goto block_11;
        }
    } else {
        recordOffset1 = type * 0x74;
        record = (SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes);
        if (record->info.get.useDirectPosition != 0) {
            param->position.x = record->info.get.getRelP0X(object, obj);
            param->position.y = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                             ->info.get.getRelP0Y(object, obj);
            param->position.z = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                             ->info.get.getRelP0Z(object, obj);
            param->velocity.x = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                             ->info.get.getRelP1X(object, obj);
            param->velocity.y = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                             ->info.get.getRelP1Y(object, obj);
            param->velocity.z = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                             ->info.get.getRelP1Z(object, obj);
            param->pan =
                (param->reverb =
                     (u8) ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                         ->callbacks.retReverb(obj),
                 0x40);
        } else {
            record->callbacks.retVectorI(obj, &axisX);
            ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                ->callbacks.retVectorJ(obj, &axisY);
            ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                ->callbacks.retVectorK(obj, &axisZ);

            relative0.x = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                              ->info.get.getRelP0X(object, obj);
            relative0.y = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                              ->info.get.getRelP0Y(object, obj);
            relative0.z = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                              ->info.get.getRelP0Z(object, obj);
            base0.x = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                          ->callbacks.callback00(obj);
            base0.y = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                          ->callbacks.callback04(obj);
            base0.z = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                          ->callbacks.callback08(obj);
            fn_vComputeRelToAbsCoord(&stAbsolute0, base0, relative0, axisX, axisY, axisZ);

            relative1.x = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                              ->info.get.getRelP1X(object, obj);
            relative1.y = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                              ->info.get.getRelP1Y(object, obj);
            relative1.z = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                              ->info.get.getRelP1Z(object, obj);
            base1.x = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                          ->info.get.getRelP1X(object, obj);
            base1.y = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                          ->info.get.getRelP1Y(object, obj);
            base1.z = ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                          ->info.get.getRelP1Z(object, obj);
            fn_vComputeRelToAbsCoord(&stAbsolute1, base1, relative1, axisX, axisY, axisZ);

            param->position.x = stAbsolute0.x;
            param->position.y = stAbsolute0.y;
            param->position.z = stAbsolute0.z;
            param->velocity.x = stAbsolute1.x;
            param->velocity.y = stAbsolute1.y;
            param->velocity.z = stAbsolute1.z;
            param->pan =
                (param->reverb =
                     (u8) ((SND_tdstObjectSoundRecord*) (recordOffset1 + inv_SND_objectSoundTypes))
                         ->callbacks.retReverb(obj),
                 0x40);
        }
    block_11:
        param->dolby = 0;
    }

    micro = micros_sound.head;
    index = 0;
    if (micro != 0) {
        do {
            if (obj != -1)
                param->micros[index].fill.link =
                    ((SND_tdstObjectSoundRecord*) (inv_SND_objectSoundTypes + type * 0x74))
                        ->callbacks.retMicroLink(obj, micro);
            else
                param->micros[index].fill.link = 1;
            param->micros[index].fill.id = micro->id;
            micro = micro->next;
            index++;
        } while (micro != 0);
    }

    if (g_bUseNearestMicro != 0) {
        micro = micros_sound.head;
        if (object != -1 || obj != object) {
            selectedMicroIndex = -1;
            selectedDistance = 32000.0f;
            index = 0;
            if (micro != 0) {
                do {
                    if (param->micros[index].fill.link != 0) {
                        GetMicroParam((s32) micro, &microParam);
                        delta[0] = (f32) param->position.x * (1.0f / 65536.0f) -
                                   (f32) microParam.position.x * (1.0f / 65536.0f);
                        delta[1] = (f32) param->position.y * (1.0f / 65536.0f) -
                                   (f32) microParam.position.y * (1.0f / 65536.0f);
                        delta[2] = (f32) param->position.z * (1.0f / 65536.0f) -
                                   (f32) microParam.position.z * (1.0f / 65536.0f);
                        {
                            f32 squaredX = delta[0] * delta[0];
                            f32 squaredY = delta[1] * delta[1];
                            distanceValue = (squaredX * squaredY) * (delta[2] * delta[2]);
                        }
                        if (selectedMicroIndex == -1 || distanceValue < selectedDistance) {
                            selectedDistance = distanceValue;
                            selectedMicroIndex = index;
                        }
                    }
                    micro = micro->next;
                    index++;
                } while (micro != 0);
            }

            micro = micros_sound.head;
            if (selectedMicroIndex != -1) {
                index = 0;
                if (micro != 0) {
                    s32* link = &param->micros[0].fill.link;
                    do {
                        if (index != selectedMicroIndex)
                            *link = 0;
                        micro = micro->next;
                        link += 2;
                        index++;
                    } while (micro != 0);
                }
            }
        }
    }

    get_RollOff((void*) obj, type, param);
    set_SwitchParam(obj, type, (u8*) param);
}

void get_param_objet2(void* objectArg, s32 type, _SND_tduRefEvt uEvent, void* paramArg,
                      void* pitchArg, void* prefillArg)
{
    s32 object = (s32) objectArg;

    _SoundParam* param = paramArg;
    s32 pitch = (s32) pitchArg;
    _SoundParam* prefill = prefillArg;
    typedef void (*SND_tfnExtraCoef)(s32, _SND_tdstBlockEvent*, s32*, s32*, s32*);

    s32 twin0;
    s32 twin1;
    s32 extra;
    u8* pVol;
    s32 typeVol;
    s32 objVol;
    s32 vol;
    s32 flags;
    _SND_tdstBlockEvent* event = uEvent.pstEvent;

    SND_fn_vCopySoundParam(param, prefill);

    pVol = (u8*) ((SND_tdstObjectSoundRecord*) (*&inv_SND_objectSoundTypes))[type].volumeLine;
    if (pVol != 0) {
        param->vol = *pVol;
        param->flags |= 4;
    } else {
        param->vol = 0x7F;
    }

    typeVol = fn_lGetVolumeForType(type);
    objVol = fn_lGetVolumeForObject(object, type);
    if (typeVol != 0x7F || objVol != typeVol) {
        param->flags |= 4;
        param->vol = (param->vol * typeVol * objVol) >> 14;
    }
    param->pitch = pitch;

    if (event->eType != 6) {
        if (event->eType != 10 || event->uParams.stExtraAll.rtCoefPitch == 0)
            goto next_vol;
        SND_fn_vSndRealTwinToSndReal(event->uParams.stExtraAll.rtCoefPitch, &twin0, &twin1);
    } else {
        twin0 = event->uParams.stExtraParam.CoefA;
        twin1 = event->uParams.stExtraParam.CoefB;
    }
    {
        ((SND_tfnExtraCoef) ((SND_tdstObjectSoundRecord*) (*&inv_SND_objectSoundTypes))[type]
             .callbacks.retExtraCoef)(object, event, &extra, 0, 0);
        extra = SND_fn_rMulRealRealSnd(extra, twin0) + twin1;
        param->pitch = SND_fn_rMulRealRealSnd(extra, param->pitch);
    }

next_vol:
    if (event->eType != 7) {
        if (event->eType != 10 || event->uParams.stExtraAll.rtCoefVol == 0)
            goto next_pan;
        SND_fn_vSndRealTwinToSndReal(event->uParams.stExtraAll.rtCoefVol, &twin0, &twin1);
    } else {
        twin0 = event->uParams.stExtraParam.CoefA;
        twin1 = event->uParams.stExtraParam.CoefB;
    }
    {
        ((SND_tfnExtraCoef) ((SND_tdstObjectSoundRecord*) (*&inv_SND_objectSoundTypes))[type]
             .callbacks.retExtraCoef)(object, event, 0, 0, &extra);
        vol = (SND_fn_rMulRealRealSnd(extra, twin0) + twin1) >> 16;
        if (vol < 0) {
            param->vol = 0;
            flags = param->flags;
        } else {
            flags = param->flags;
            if (vol < 0x80) {
                if (flags & 4) {
                    param->vol = (vol * param->vol) >> 7;
                } else {
                    param->vol = vol;
                }
            } else if (!(flags & 4)) {
                param->vol = 0x7F;
            }
        }
        param->flags = flags | 4;
    }
next_pan:

    if (event->eType != 8) {
        if (event->eType != 10 || event->uParams.stExtraAll.rtCoefPan == 0)
            goto done;
        SND_fn_vSndRealTwinToSndReal(event->uParams.stExtraAll.rtCoefPan, &twin0, &twin1);
    } else {
        twin0 = event->uParams.stExtraParam.CoefA;
        twin1 = event->uParams.stExtraParam.CoefB;
    }
    {
        ((SND_tfnExtraCoef) ((SND_tdstObjectSoundRecord*) (*&inv_SND_objectSoundTypes))[type]
             .callbacks.retExtraCoef)(object, event, 0, &extra, 0);
        vol = (SND_fn_rMulRealRealSnd(extra, twin0) + twin1) >> 16;
        if (vol < 0) {
            param->pan = 0;
        } else if (vol >= 0x80) {
            param->pan = 0x7F;
        } else {
            param->pan = vol;
        }
        param->flags |= 1;
    }
done:;
}

void get_param_objet(s32 a0, void* superObject, s32 a2, void* eventRef, void* t0, void* t1)
{
    u8 buf[0x60];

    ((_SoundParam*) buf)->microCount = g_iNbActiveMicros;
    fill_pre_filled_param(a0, (s32) superObject, a2, (_SoundParam*) buf);
    ((void (*)(void*, s32, void*, void*, void*, void*)) get_param_objet2)(superObject, a2, eventRef,
                                                                          t0, t1, buf);
}

s32 fn_rGetDeTune(_SND_tduRefEvt uRef)
{
    _SND_tdstBlockEvent* pstEvent;
    u32 type;
    s32 detune;
    s32 result;
    s32 lOffset;

    pstEvent = uRef.pstEvent;
    type = pstEvent->eType;
    if (type == 1) {
        detune = pstEvent->uParams.stPlay.stDeTune.rDelta;
    } else {
        detune = 0;
        if (type != 0) {
            if (!((u32) type >= 9)) {
                if (!((u32) type < 6)) {
                    detune = pstEvent->uParams.stExtraParam.stDeTune.rDelta;
                }
            }
        }
    }
    if (detune != 0) {
        result = SND_fn_rGetRandomSnd(detune << 1);
        lOffset = detune + -0x10000;
        result = result - lOffset;
    } else {
        result = 0x10000;
    }
    return result;
}

int fn_rRetObjectSoundSndRealDefault(void)
{
    return 0;
}

int fn_ucRetObjectSoundREVERBDefault(void)
{
    return 0;
}

void fn_vRetObjectSoundExtraCoefDefault(s32 unused0, s32 unused1, s32* pitch, s32* pan, s32* volume)
{
    if (pitch != 0) {
        *pitch = 0x10000;
    }
    if (pan != 0) {
        *pan = 0x10000;
    }
    if (volume != 0) {
        *volume = 0x10000;
    }
}

int fn_lRetObjectSoundSwitchDefault(void)
{
    return 0;
}

int fn_bRetObjectSoundMicroLinkDefault(void)
{
    return 1;
}

void fn_vRetObjectSoundVectorIDefault(void* unused, SndFixedVector3* output)
{
    if (output != 0) {
        output->y = 0;
        output->x = 0x10000;
        output->z = 0;
    }
}

void fn_vRetObjectSoundVectorJDefault(void* unused, SndFixedVector3* output)
{
    if (output != 0) {
        output->x = 0;
        output->y = 0x10000;
        output->z = 0;
    }
}

void fn_vRetObjectSoundVectorKDefault(void* unused, SndFixedVector3* output)
{
    if (output != 0) {
        output->y = 0;
        output->z = 0x10000;
        output->x = 0;
    }
}

s32 fn_rRetMicroSoundSndReal0Default(s32 object)
{
    return 0;
}

s32 fn_rRetMicroSoundSndReal1Default(s32 object)
{
    return 0x10000u;
}

s32 fn_lRetMicroSoundFILTREDefault(s32 object)
{
    return 0;
}

int fn_rRetChannelSoundSndReal0Default(void)
{
    return 0;
}

u32 fn_rRetChannelSoundSndReal1Default(void)
{
    return 0x10000u;
}

#ifdef PC_PORT
int fn_bRetChannelSoundActiveDefault(void* unused0, void* unused1)
#else
int fn_bRetChannelSoundActiveDefault(void)
#endif
{
    return 1;
}

int fn_bRetChannelSoundAbsCoordDefault(void)
{
    return 1;
}

void check_micro_fct(void)
{
    micros_sound.head = NULL;
    micros_sound.tail = 0;
    micros_sound.count = 0;
    g_iNbActiveMicros = 0;
    g_st_pfnRetMicroSound.X = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.Y = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.Z = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.VX = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.VY = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.VZ = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.DTX = fn_rRetMicroSoundSndReal1Default;
    g_st_pfnRetMicroSound.DTY = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.DTZ = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.DNX = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.DNY = fn_rRetMicroSoundSndReal0Default;
    g_st_pfnRetMicroSound.DNZ = fn_rRetMicroSoundSndReal1Default;
    g_st_pfnRetMicroSound.Filtre = fn_lRetMicroSoundFILTREDefault;
}

void fn_vInitObjectType(void)
{
    gi_NbObjectType = 0;
    SND_fn_lAddObjectTypeSound2(&g_astTypeObjectSoundInfosDefault, 0);
}

void desinit_object_type(void)
{
    SND_fn_vFreeSndEx(0, (void*) inv_SND_objectSoundTypes);
}

static inline VLNode* inv_SND_fn_p_stFirstVolumeLine(void)
{
    return *(VLNode**) g_VolumeLines;
}

void inv_SND_fn_vFreeAllVolumeLines(void)
{
    VLNode* node;
    VLNode* next;

    for (node = inv_SND_fn_p_stFirstVolumeLine(), next = node ? node->next : NULL; node != NULL;
         node = next, next = node ? node->next : NULL)
        SND_fn_vFreeSndEx(0, node);
    nb_volume_lines = 0;
}

void init_association(void)
{
    s32* listWords = (s32*) &association;
    association.first = 0;
    listWords[1] = 0;
    listWords[2] = 0;
}

void inv_SND_fn_vNoOpSound3D(s32 lIndice)
{
    (void) lIndice;
}

s32 SND_fn_iInitSound(void* parameters)
{
    s32 opt;
    s32 snd;
    s32 result;

    SND_fn_vInitThreadSnd();
    SND_fn_vInitErrorSnd(parameters);
    opt = SND_fn_iInitOptions(parameters);
    SND_fn_vInitMallocSnd();
    fn_vInitObjectType();
    check_micro_fct();
    init_association();
    snd = SND_fn_iInitSnd(parameters);

    if (snd == -1 || opt == -1) {
        result = -1;
    } else if (snd == 1 || opt == 1) {
        result = 1;
    } else {
        result = 3;
    }
    SND_fn_vInitBankSnd(0);
    SND_fn_vQuitCriticalSectionThreadSnd();
    g_bInitDoneSound = 1;
    return result;
}

s32 SND_fn_iTestInitSound(void)
{
    return SND_fn_iTestInitOptions();
}

s32 SND_fn_iDesInitSound(void)
{
    s32 result;

    result = -2;
    if (g_bInitDoneSound != 0) {
        g_bInitDoneSound = 0;
        SND_fn_vDestroyAllMicroSound();
        SND_fn_vEnterCriticalSectionThreadSnd_();
        SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x6DD);
        stop_son_all();
        desinit_object_type();
        inv_SND_fn_vFreeAllVolumeLines();
        SND_fn_vDesInitBankSnd();
        SND_fn_vDesInitSnd();
        SND_fn_vDesInitThreadSnd();
        SND_fn_vDesInitMallocSnd();
        SND_fn_vDesInitErrorSnd();
        result = SND_fn_iDesInitOptions();
    }
    return result;
}

s32 SND_fn_iTestDesInitSound(void)
{
    return SND_fn_iTestDesInitOptions();
}

void SND_fn_vSetDopplerFactorSound(s32 factor)
{
    if (factor == 0) {
        g_bDopplerActive = 0;
    } else {
        g_bDopplerActive = 1;
    }
#ifdef PC_PORT
    SND_fn_vSetDopplerFactorSnd(factor);
#else
    SND_fn_vSetDopplerFactorSnd(factor);
#endif
}

#ifdef PC_PORT
void SND_fn_vSetDefaultRollOffSound(s32 a0)
{
    SND_fn_vSetDefaultRollOffSnd(a0);
}
#else
void SND_fn_vSetDefaultRollOffSound(SndRollOff* rollOff)
{
    SND_fn_vSetDefaultRollOffSnd(rollOff);
}
#endif

void fn_vRajAllVolumeForLine(s32* object)
{
        s32 ownerKey = (s32) object;

    Association* assoc;
    Association* next;
    union {
        _SoundParam param;
        u8 bytes[0x60];
    } blk;
    u8* bp;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x755);
    bp = (u8*) &blk;
    ((_SoundParam*) bp)->microCount = g_iNbActiveMicros;
    assoc = association.first;
    if (assoc != 0) {
        next = assoc->next;
        do {
            if (assoc->active != 0) {
                if ((s32) ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[assoc->type]
                        .volumeLine == ownerKey) {
                    get_param_objet(assoc->channel, (void*) assoc->object, assoc->type, assoc->event, bp,
                                    (void*) assoc->detune);
                    if (SND_fn_bSetParamSnd((PlaySndSlot*) assoc->snd, bp) == 0) {
                        erase_association((s32*) assoc);
                    }
                }
            } else {
                erase_association((s32*) assoc);
            }
            assoc = next;
            next = 0;
            if (assoc != 0) {
                next = assoc->next;
            }
        } while (assoc != 0);
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vSetFadeOutTicks(s32 mode)
{
    VLNode* node;
    VLNode* next;

    g_ulNbFadeOutTicks = mode;
    for (node = inv_SND_fn_p_stFirstVolumeLine(), next = node ? node->next : NULL; node != NULL;
         node = next, next = node ? node->next : NULL)
        node->inv_volBeforeFade = node->vol;
}

void SND_fn_vFadeOut(u32 frames)
{
    u32 amount;
    VLNode* node;
    VLNode* next;
    s32 volume;
    f32 scaledAmount;

    scaledAmount = (f32) frames * 127.0f;
    scaledAmount = scaledAmount / (f32) (u32) g_ulNbFadeOutTicks;
#ifdef PC_PORT
    amount = fptoui(scaledAmount);
#else
    amount = (u32) scaledAmount;
#endif
    node = *(VLNode**) g_VolumeLines;
L:
    if (node == 0) {
        return;
    }
    volume = node->inv_volBeforeFade - (s32) amount;
    next = node->next;
    if (volume >= 0) {
        SND_fn_vSetVolumeLineSound((s32*) node, volume);
    }
    node = next;
    goto L;
}

void SND_fn_vRestoreVolumeLines(void)
{
    VLNode* node;
    VLNode* next;

    for (node = inv_SND_fn_p_stFirstVolumeLine(), next = node ? node->next : NULL; node != NULL;
         node = next, next = node ? node->next : NULL)
        SND_fn_vSetVolumeLineSound((s32*) node, node->inv_volBeforeFade);
}

void SND_fn_vSynchroSound(void)
{
    SxdMicroParams_inv microParams;

    union {
        _SoundParam param;
        u8 bytes[0x60];
    } parameters;

    union {
        _SoundParam param;
        u8 bytes[0x60];
    } prefilledParameters;

    gu_tdstMicroSoundNode* micro;
    Association* node;
    Association* next;
    _SoundParam* prefilled;
    _SoundParam* param;
    _SND_tduRefEvt event;
    s32 lastChannel;
    s32 lastObject;
    s32 lastType;
    s32 valid;

    prefilled = NULL;
    lastChannel = -1;
    lastObject = -1;
    lastType = -1;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x7D0);

    for (micro = micros_sound.head; micro != NULL; micro = micro->next) {
        GetMicroParam((s32) micro, &microParams);
        SND_fn_vSetMicroParamSnd(micro->id, &microParams);
    }
    param = &parameters.param;
    parameters.param.microCount = g_iNbActiveMicros;

    node = association.first;
    for (;;) {
        if (node == NULL)
            goto done;
        next = node->next;
        if (node->active != 0) {
            if (node->event->bDynamic) {
                if (node->channelActive == 0) {
                    goto test_playing;
                }
                node->channelActive = fn_bIsChannelActive(node->channel, node->object, node->type);
            }
            if (node->channelActive != 0) {
                if (prefilled == NULL) {
                    prefilled = &prefilledParameters.param;
                    prefilledParameters.param.microCount = g_iNbActiveMicros;
                    lastChannel = node->channel;
                    lastObject = node->object;
                    lastType = node->type;
                    fill_pre_filled_param(lastChannel, lastObject, lastType, prefilled);
                } else if (node->channel != lastChannel || node->object != lastObject ||
                           node->type != lastType) {
                    lastChannel = node->channel;
                    lastObject = node->object;
                    lastType = node->type;
                    fill_pre_filled_param(lastChannel, lastObject, lastType, prefilled);
                }
                event.pstEvent = node->event;
                get_param_objet2((void*) node->object, node->type, event, param,
                                 (void*) node->detune, prefilled);
                valid = SND_fn_bSetParamSnd((PlaySndSlot*) node->snd, param);
            } else {
            test_playing:
                valid = SND_fn_bTestIsPlayingSnd((PlaySndSlot*) node->snd);
            }
            if (valid == 0) {
                erase_association((s32*) node);
            }
        } else {
            erase_association((s32*) node);
        }
        node = next;
    }
done:
    SND_fn_vSynchroSnd();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

s32 SND_fn_lSendRequestSound(s32 superObject, s32 a1, s32 eventRef, s32 a3, SndEndCallback_inv a4)
{
    return SND_fn_lSendRequestChannelSound(-1, superObject, a1, (_SND_tdstBlockEvent*) eventRef,
                                           a3, a4);
}

s32 inv_SND_fn_lSendStopRequestSound(void* unk0, void* a1, void* a2, s32* a3, void* unk4, s32 a5)
{
    s32 flag;
    s32 state;

    if (a3 == 0) {
        return -2;
    }
    SND_fn_vEnterCriticalSectionThreadSnd_();
    state = 1;
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x88E);
    if (a3[1] == state) {
        stop_un_son_objet(a1, (s32) a2, a3);
    } else {
        SND_fn_vDisplayError(0x13, (*(u8(*)[]) & unk_393858));
    }
    fnSetLastEventForSoundObjectType((s32) a2, (s32) a3);
    SND_fn_vQuitCriticalSectionThreadSnd();
    if (a5 == 0) {
        return 1;
    }
    if (flag == 0) {
        return -1;
    }
    return 1;
}

s32 SND_fn_lSendRequestChannelSound(s32 a0, s32 a1, s32 a2, _SND_tdstBlockEvent* req, s32 a4,
                                    SndEndCallback_inv a5)
{
    u8 buf[0x60];
    u8* pBuf;
    s32 counter;
    s32 flag;
    s32 ok;
    Association* slot;
    SndBinResource* node;
    s32 detune;
    s32 playResult;
    s32 result;

    if (req == 0) {
        return -2;
    }
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x8E7);
    counter = HandleCallback;
    pBuf = buf;
    ((_SoundParam*) buf)->microCount = g_iNbActiveMicros;
    HandleCallback = counter + 1;
    if ((u32) (counter + 3) < 2) {
        HandleCallback = 1;
    }

    ok = 1;
    flag = 0;
    switch (req->eType) {
    case 1:
    case 6:
    case 7:
    case 8:
    case 10:
        slot = pre_associe_sound();
        if (slot == 0) {
            ok = 0;
            goto tail;
        }
        if (req->eType == 1) {
            node = req->uParams.stPlay.uResource;
        } else if (req->eType == 10) {
            node = req->uParams.stExtraAll.uResource;
        } else {
            node = req->uParams.stExtraParam.uResource;
        }
        if (node != 0) {
            detune = ((s32(*)(s32)) fn_rGetDeTune)((s32) req);
            get_param_objet(a0, (void*) a1, a2, req, pBuf, (void*) detune);
            playResult = SND_fn_lPlaySnd(node, (_SoundParam*) pBuf, a4, a5, counter, &slot->active);
            if (playResult != -1) {
                associe_sound(slot, a0, a1, a2, a4, playResult,
                              (s32) req, node, detune, counter);
                flag = (a5 != 0);
            } else {
                erase_association((s32*) slot);
                ok = 0;
            }
        } else {
            erase_association((s32*) slot);
            ok = 0;
        }
        goto tail;
    case 2:
        stop_un_son_objet((void*) a1, a2, req->uParams.stStop.uEvt.pstEvent);
        goto tail;
    case 3:
        stop_all_son_objet(a1, a2);
        goto tail;
    case 4:
        stop_un_son_objet((void*) a1, a2, req->uParams.stStopNGo.uEvtStop.pstEvent);
        flag = 1;
        counter = SND_fn_lSendRequestChannelSound(a0, a1, a2, req->uParams.stStopNGo.uEvtGo.pstEvent, a4, a5);
        if (counter == -2) {
            ok = 0;
        }
        goto tail;
    case 9:
        j_j_j_nullsub_22(req->uParams.stEffect.iIdEffect);
        goto tail;
    case 11:
        if (req->uParams.stChangeVolume.bConcernWithObjectType != 0) {
            fn_vChangeVolumeForType(a2, req->uParams.stChangeVolume.lNewVolume);
            SND_fn_vSynchroSound();
        } else {
            fn_vChangeVolumeForObject(a1, a2, req->uParams.stChangeVolume.lNewVolume);
            SND_fn_vSynchroSound();
        }
        goto tail;
    default:
        SND_fn_vDisplayError(0x13, unk_393858);
        ok = 0;
        goto tail;
    }

tail:
    fnSetLastEventForSoundObjectType(a2, (s32) req);
    SND_fn_vQuitCriticalSectionThreadSnd();
    if (ok == 0) {
        result = -2;
    } else {
        if (a5 == 0) {
            result = counter;
        } else {
            result = -1;
            if (flag != 0) {
                result = counter;
            }
        }
    }
    return result;
}

void j_j_j_j_nullsub_22(s32 a0)
{
    j_j_j_nullsub_22(a0);
}

s32 SND_fn_rGetPosSound(s32 id)
{
    s32 found;
    Association* node;
    s32 position;

    found = 0;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x98D);
    node = association.first;
    while (node != 0) {
        if (node->handle == id) {
            found = 1;
            break;
        }
        node = node->next;
    }
    position = -2;
    if (found != 0) {
        position = SND_fn_rGetPosSnd((PlaySndSlot*) node->snd);
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
    return position;
}

s32 SND_fn_rGetLengthSound(s32 id)
{
    Association* node = association.first;
    s32 found = 0;
    s32 length;

    for (; node != 0; node = node->next) {
        if (node->handle == id) {
            found = 1;
            break;
        }
    }
    length = -1;
    if (found != 0) {
        return SND_fn_rGetLengthSnd((PlaySndSlot*) node->snd);
    }
    return length;
}

void SND_fn_vKillChannelSound(s32 channel, s32 object, s32 type)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0x9DF);
    if (channel != -1) {
        *get_association_channel(channel, object, type, 0) = 0;
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vSetRetChannelTypeSound(s32 lIndice, SND_tstObjectSoundInfoBlock* pstInfo)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA05);

    ((SND_tdstObjectSoundRecord*) (lIndice * 0x74 + inv_SND_objectSoundTypes))->info.block =
        *pstInfo;

    inv_SND_fn_vNoOpSound3D(lIndice);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vKillObjectSound2(s32 object, s32 typeIndex)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA1A);
    stop_all_son_objet(object, typeIndex);
    fn_vResetVolumeForObject(object, typeIndex);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vKillObjectSound(void)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA32);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vKillAllObjectTypeSound(s32 typeIndex)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA48);
    stop_son_type_all(typeIndex);
    fn_vChangeVolumeForType(typeIndex, 0x7F);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vKillAllObjectSound(void)
{
    s32 typeIndex;

    typeIndex = 0;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA5C);
    stop_son_all();
    if (gi_NbObjectType > 0) {
        do {
            fn_vChangeVolumeForType(typeIndex, 0x7F);
            typeIndex += 1;
        } while (typeIndex < gi_NbObjectType);
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vKillAllObjectButTypeSound(s32 typeIndex)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA6F);
    stop_son_type_but_all(typeIndex);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

long SND_fn_lCreateMicroSound(void)
{
    gu_tdstMicroSoundNode* sound;
    gu_tdstMicroSoundNode* link;
    gu_tdstMicroSoundHead* owner;
    s32 count;
    long result;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xA83);
    count = g_iNbActiveMicros;
    if (count == 5) {
        SND_fn_vDisplayError(0x1D, (*(char(*)[]) & unk_393858));
        SND_fn_vQuitCriticalSectionThreadSnd();
        return -1;
    }
    g_iNbActiveMicros = count + 1;
    sound = SND_fn_pvMallocSndEx(0, 0x10);
    sound->owner = 0;
    sound->next = 0;
    sound->prev = 0;
    owner = sound->owner;
    if (owner != 0) {
        gu_tdstMicroSoundNode* detach_link;
        owner->count--;
        owner = sound->owner;
        detach_link = owner->head;
        if (detach_link == sound) {
            detach_link = sound->next;
            owner->head = detach_link;
            owner = sound->owner;
        }
        detach_link = owner->tail;
        if (detach_link == sound) {
            detach_link = sound->prev;
            owner->tail = detach_link;
        }
    }
    sound->prev = 0;
    sound->owner = 0;
    sound->next = 0;
    count = (s32) micros_sound.tail;
    sound->owner = &micros_sound;
    sound->prev = (gu_tdstMicroSoundNode*) count;
    if (micros_sound.head != 0) {
        link = micros_sound.tail;
    } else {
        micros_sound.head = sound;
        goto L_tail;
    }
    link->next = sound;
L_tail:
    micros_sound.tail = sound;
    count = micros_sound.count + 1;
    micros_sound.count = count;
    result = (long) sound;
    sound->id = SND_fn_lCreateMicroSnd(0);
    SND_fn_vQuitCriticalSectionThreadSnd();
    return result;
}

void SND_fn_vDestroyMicroSound(s32* node)
{
    s32* linkOrOwner;
    s32 value;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xAA9);
    g_iNbActiveMicros = g_iNbActiveMicros - 1;
    linkOrOwner = (s32*) node[1];
    if (linkOrOwner != (s32*) 0) {
        linkOrOwner[2] = node[2];
    }
    linkOrOwner = (s32*) node[2];
    if (linkOrOwner == (s32*) 0) {
        linkOrOwner = (s32*) node[3];
    } else {
        linkOrOwner[1] = node[1];
        linkOrOwner = (s32*) node[3];
    }
    if (linkOrOwner == (s32*) 0) {
        node[1] = 0;
    } else {
        value = linkOrOwner[2];
        value = value - 1;
        linkOrOwner[2] = value;
        linkOrOwner = (s32*) node[3];
        value = linkOrOwner[0];
        if (value != (s32) node) {
            value = linkOrOwner[1];
        } else {
            value = node[1];
            linkOrOwner[0] = value;
            linkOrOwner = (s32*) node[3];
            value = linkOrOwner[1];
        }
        if (value != (s32) node) {
            node[1] = 0;
        } else {
            value = node[2];
            linkOrOwner[1] = value;
            node[1] = 0;
        }
    }
    node[2] = 0;
    node[3] = 0;
    dbgSND_fn_vKillMicro(node[0]);
    SND_fn_vFreeSndEx(0, node);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vDestroyAllMicroSound(void)
{
    s32* node;
    s32* next;
    s32* linkOrOwner;
    s32 value;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xAC4);
    g_iNbActiveMicros = 0;
    node = (s32*) micros_sound.head;
L_loop:
    if (node != (s32*) 0) {
        next = (s32*) node[1];
        do {
            dbgSND_fn_vKillMicro(node[0]);
        } while (0);
        linkOrOwner = (s32*) node[1];
        if (linkOrOwner == (s32*) 0) {
            linkOrOwner = (s32*) node[2];
        } else {
            linkOrOwner[2] = node[2];
            linkOrOwner = (s32*) node[2];
        }
        if (linkOrOwner == (s32*) 0) {
            linkOrOwner = (s32*) node[3];
        } else {
            linkOrOwner[1] = node[1];
            linkOrOwner = (s32*) node[3];
        }
        if (linkOrOwner != (s32*) 0) {
            value = linkOrOwner[2];
            value = value - 1;
            linkOrOwner[2] = value;
            linkOrOwner = (s32*) node[3];
            value = linkOrOwner[0];
            if (value != (s32) node) {
                value = linkOrOwner[1];
            } else {
                value = node[1];
                linkOrOwner[0] = value;
                linkOrOwner = (s32*) node[3];
                value = linkOrOwner[1];
            }
            if (value == (s32) node) {
                value = node[2];
                linkOrOwner[1] = value;
            }
        }
        node[1] = 0;
        node[2] = 0;
        node[3] = 0;
        SND_fn_vFreeSndEx(0, node);
        node = next;
        goto L_loop;
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vSetRetMicroSound(_tdstAllRetMicroSound* src)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xADD);
    inv_MOD_fn_vNoOp();
    g_st_pfnRetMicroSound = *src;
    SND_fn_vQuitCriticalSectionThreadSnd();
}

VLNode* SND_fn_lAddVolumeLineSound(void)
{
    VLNode* node;
    VLNode* tail;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xAEF);
    if (nb_volume_lines == 0) {
        *(s32*) g_VolumeLines = 0;
        ((VLList*) g_VolumeLines)->last = 0;
        ((VLList*) g_VolumeLines)->count = 0;
    }
    node = SND_fn_pvMallocSndEx(0, 0x14);
    memset(node, 0, 0x14);
    *(volatile s32*) &node->list = 0;
    node->next = 0;
    node->prev = 0;
    if (node->list != 0) {
        node->list->count = node->list->count - 1;
        if (node->list->first == node) {
            node->list->first = node->next;
        }
        if (node->list->last == node) {
            node->list->last = node->prev;
        }
    }
    node->prev = 0;
    *(volatile s32*) &node->list = 0;
    node->next = 0;
    tail = ((VLList*) g_VolumeLines)->last;
    node->list = (VLList*) g_VolumeLines;
    node->prev = tail;
    if (((VLList*) g_VolumeLines)->first == 0) {
        ((VLList*) g_VolumeLines)->first = node;
    } else {
        ((VLList*) g_VolumeLines)->last->next = node;
    }
    ((VLList*) g_VolumeLines)->last = node;
    ((VLList*) g_VolumeLines)->count = ((VLList*) g_VolumeLines)->count + 1;
    node->vol = 127;
    nb_volume_lines = nb_volume_lines + 1;
    SND_fn_vQuitCriticalSectionThreadSnd();
    return node;
}

void fn_vUpdateTypeObjectSoundInfos(SND_tdstTypeObjectSoundInfos* dst,
                                    SND_tdstTypeObjectSoundInfos* src)
{
    *dst = g_astTypeObjectSoundInfosDefault;

    if (src->callback00 != 0) {
        dst->callback00 = src->callback00;
    }
    if (src->callback04 != 0) {
        dst->callback04 = src->callback04;
    }
    if (src->callback08 != 0) {
        dst->callback08 = src->callback08;
    }
    if (src->callback0C != 0) {
        dst->callback0C = src->callback0C;
    }
    if (src->callback10 != 0) {
        dst->callback10 = src->callback10;
    }
    if (src->callback14 != 0) {
        dst->callback14 = src->callback14;
    }
    if (src->retReverb != 0) {
        dst->retReverb = src->retReverb;
    }
    if (src->retExtraCoef != 0) {
        dst->retExtraCoef = src->retExtraCoef;
    }
    if (src->retSwitch != 0) {
        dst->retSwitch = src->retSwitch;
    }
    if (src->retMicroLink != 0) {
        dst->retMicroLink = src->retMicroLink;
    }
}

#define RECORD_SIZE 0x74
#define BLOCK_OFF 0x34

typedef struct {
    u32 w[8];
} Blk32;

void fn_vSetDefaultRetChannelSound(s32 typeIndex)
{
    u32 recordAddress = RECORD_SIZE * (u32) typeIndex + inv_SND_objectSoundTypes;

    *(Blk32*) (recordAddress + BLOCK_OFF) = *(Blk32*) &qword_3704F0[0];
}

s32 SND_fn_lAddObjectTypeSound2(void* callbacks, VLNode* volumeLine)
{
    s32 count;
    s32 recordOffset;
    SND_tdstObjectSoundRecord* record;
    SndObjectVolumeListView* volumes;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xB7A);
    if (gi_NbObjectType == 0) {
        count = 0;
        inv_SND_objectSoundTypes = (s32) SND_fn_pvMallocSndEx(0, 0x74);
        gi_NbObjectType = 0;
    } else {
        count = gi_NbObjectType;
        inv_SND_objectSoundTypes = (s32) SND_fn_pvReAllocSndEx(
            0, (void*) inv_SND_objectSoundTypes, (count + 1) * 0x74);
    }
    recordOffset = 0x74 * count;
    memset((void*) (recordOffset + inv_SND_objectSoundTypes), 0, 0x74);
    record = (SND_tdstObjectSoundRecord*) (recordOffset + inv_SND_objectSoundTypes);
    record->lastEvent = 0;
    volumes = &record->volumes;
    record->volumes.first = 0;
    volumes->last = 0;
    volumes->count = 0;
    record->volume = 0x7F;
    if (volumeLine != 0) {
        record->volumeLine = volumeLine;
    } else {
        record->volumeLine = 0;
    }
    gi_NbObjectType = gi_NbObjectType + 1;
    fn_vUpdateTypeObjectSoundInfos((void*) (inv_SND_objectSoundTypes + count * 0x74), callbacks);
    fn_vSetDefaultRetChannelSound(count);
    SND_fn_vQuitCriticalSectionThreadSnd();
    return count;
}

void SND_fn_vSetRetObjectTypeSound(s32 lIndice, s32 lSource)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xBB8);
    if (lIndice < 0 || lIndice >= gi_NbObjectType) {
        SND_fn_vDisplayError(0x1D, unk_393858);
    } else {
        fn_vUpdateTypeObjectSoundInfos(
            (SND_tdstTypeObjectSoundInfos*) (inv_SND_objectSoundTypes + lIndice * 0x74),
            (SND_tdstTypeObjectSoundInfos*) lSource);
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vSetInfoForTypeSound(s32 typeIndex, SndObjectInfoCallback_inv callback)
{
    if (typeIndex < 0) {
        return;
    }
    if (typeIndex >= gi_NbObjectType) {
        return;
    }
    ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].infoCallback = callback;
}

void SND_fn_vGetInfoForObjectSound(s32 object, s32 recordAddress, char* output, s32 size)
{
    s32 stride;
    void (*callback)(s32, char*, s32);

    stride = 0x74;
    recordAddress = recordAddress * stride;
    recordAddress = recordAddress + inv_SND_objectSoundTypes;
    callback = *(void (**)(s32, char*, s32))(recordAddress + 0x58);
    if (callback == 0) {
        strncpy(output, "no info", size);
        return;
    }
    callback(object, output, size);
}

void SND_fn_vSetRollOffFunctionForTypeSound(s32 typeIndex, SndRollOffCallback_inv callback)
{
    if (typeIndex >= 0 && typeIndex < gi_NbObjectType) {
        ((SND_tdstObjectSoundRecord*) inv_SND_objectSoundTypes)[typeIndex].rollOffHandler = callback;
    }
}

s32 SND_fn_lGetVolumeLineSound(s32* line)
{
    s32 volume = 0x7F;
    if (line != 0) {
        volume = *line;
    }
    return volume;
}

void SND_fn_vSetVolumeLineSound(s32* line, s32 volume)
{
    if (line == 0) {
        return;
    }
    *line = volume;
    fn_vRajAllVolumeForLine(line);
}

void SND_fn_vPauseSound(void)
{
    printf("START SND_fn_vPauseSound\n");
    if (g_bInitDoneSound != 0) {
        SND_fn_vEnterCriticalSectionThreadSnd_();
        SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xC1E);
        SND_fn_vPauseAllSnd();
        SND_fn_vQuitCriticalSectionThreadSnd();
    }
    printf("END SND_fn_vPauseSound\n");
}

void SND_fn_vResumeSound(void)
{
    printf("START SND_fn_vResumeSound\n");
    if (g_bInitDoneSound != 0) {
        SND_fn_vEnterCriticalSectionThreadSnd_();
        SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xC2E);
        SND_fn_vResumeAllSnd();
        SND_fn_vQuitCriticalSectionThreadSnd();
    }
    printf("END SND_fn_vResumeSound\n");
}

#define SOUND_CONTROL_OBJECT(object, typeIndex, traceLine, operation)                              \
    Association* node;                                                                                     \
                                                                                                   \
    if (g_bInitDoneSound == 0) {                                                                         \
        return;                                                                                    \
    }                                                                                              \
    SND_fn_vEnterCriticalSectionThreadSnd_();                                                      \
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", traceLine);                                         \
    node = association.first;                                                                  \
    if (node != 0) {                                                                               \
        do {                                                                                       \
            if (node->active != 0) {                                                             \
                if (node->object == object) {                                                     \
                    if (node->type == typeIndex) {                                              \
                        operation((PlaySndSlot*) node->snd);                                         \
                    }                                                                              \
                }                                                                                  \
            }                                                                                      \
            node = node->next;                                                          \
        } while (node != 0);                                                                       \
    }                                                                                              \
    SND_fn_vQuitCriticalSectionThreadSnd();

void SND_fn_vPauseObjectSound(s32 object, s32 typeIndex)
{
    SOUND_CONTROL_OBJECT(object, typeIndex, 0xC43, SND_fn_vPauseSnd);
}

void SND_fn_vResumeObjectSound(s32 object, s32 typeIndex)
{
    SOUND_CONTROL_OBJECT(object, typeIndex, 0xC5D, SND_fn_vResumeSnd);
}

#define SOUND_CONTROL_TYPES(typeIndex, comparison, traceLine, operation)                           \
    Association* node;                                                                                     \
                                                                                                   \
    if (g_bInitDoneSound == 0) {                                                                         \
        return;                                                                                    \
    }                                                                                              \
    SND_fn_vEnterCriticalSectionThreadSnd_();                                                      \
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", traceLine);                                         \
    node = association.first;                                                                  \
    if (node != 0) {                                                                               \
        do {                                                                                       \
            if (node->active != 0) {                                                             \
                if (node->type comparison typeIndex) {                                          \
                    operation((PlaySndSlot*) node->snd);                                             \
                }                                                                                  \
            }                                                                                      \
            node = node->next;                                                          \
        } while (node != 0);                                                                       \
    }                                                                                              \
    SND_fn_vQuitCriticalSectionThreadSnd();

void SND_fn_vPauseTypeSound(s32 typeIndex)
{
    SOUND_CONTROL_TYPES(typeIndex, ==, 0xC7B, SND_fn_vPauseSnd);
}

void SND_fn_vResumeTypeSound(s32 typeIndex)
{
    SOUND_CONTROL_TYPES(typeIndex, ==, 0xC96, SND_fn_vResumeSnd);
}

void SND_fn_vPauseButOneTypeSound(s32 typeIndex)
{
    SOUND_CONTROL_TYPES(typeIndex, !=, 0xCB3, SND_fn_vPauseSnd);
}

void SND_fn_vResumeButOneTypeSound(s32 typeIndex)
{
    SOUND_CONTROL_TYPES(typeIndex, !=, 0xCCD, SND_fn_vResumeSnd);
}

void SND_fn_vForceReleaseDriverSound(void)
{
    SND_fn_vForceReleaseDriverSnd();
}

void SND_fn_vForceRestoreDriverSound(void)
{
    SND_fn_vForceRestoreDriverSnd();
}

s32 gu_SND_fn_bReleaseDriverSound(void)
{
    s32 result;

    if (g_bInitDoneSound != 0) {
        if (func_0015F7F8() == 0) {
            result = 0;
            goto done;
        }
        SND_fn_vReleaseDriverSnd();
        SND_fn_vQuitCriticalSectionThreadSnd();
        result = 1;
        goto done;
    }
    result = 0;
done:
    return result;
}

s32 gu_SND_fn_bRestoreDriverSound(void)
{
    s32 result;

    if (g_bInitDoneSound != 0) {
        if (func_0015F7F8() == 0) {
            result = 0;
            goto done;
        }
        SND_fn_vRestoreDriverSnd();
        SND_fn_vQuitCriticalSectionThreadSnd();
        result = 1;
        goto done;
    }
    result = 0;
done:
    return result;
}

void func_0015D4E8(void)
{
    gu_SND_fn_bReleaseDriverSound();
}

void func_0015D4F0(void)
{
    gu_SND_fn_bRestoreDriverSound();
}

s32 SND_fn_lGetVolumeSound(void)
{
    return SND_fn_ucGetSoundVolumeSnd();
}

void func_0015D518(s32 a0)
{
    SND_fn_vSetSoundVolumeSnd((s32) (s8) a0);
}

void j_j_nullsub_103(u8 a0)
{
    j_nullsub_103();
}

s32 SND_fn_bIsMusicPlayingSound(void *a0)
{
    return SND_fn_bIsMusicPlayingSnd();
}

s32 SND_fn_bGetStereoSound(void *a0)
{
    return SND_fn_bGetStereoSnd();
}

void SND_fn_vSetReverseStereoSound(void)
{
    SND_fn_vSetReverseStereoSnd();
}

s32 SND_fn_bGetReverseStereoSound(void)
{
    return SND_fn_bGetReverseStereoSnd();
}

#ifdef PC_PORT
void SND_fn_vGetTargetLabelSound(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSnd(name, size);
}
#else
void SND_fn_vGetTargetLabelSound(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSnd(name, size);
}
#endif

void SND_fn_vSetupTargetSound(void)
{
    SND_fn_vSetupTargetSnd();
}

s32 SND_fn_bCanSetupTargetSound(void)
{
    return SND_fn_bCanSetupTargetSnd();
}

#ifdef PC_PORT
s32 SND_fn_lCreateNewBufferSnd(s32 a0, s32 a1, s32 a2, s32 a3, s32 t0, s32 t1, s32 t2);

s32 SND_fn_lCreateNewBufferSound(s32 a0, s32 a1, s32 a2, s32 a3, s32 t0, s32 t1, s32 t2)
{
    return SND_fn_lCreateNewBufferSnd(a0, a1 & 65535, a2 & 65535, a3, t0, t1, t2);
}
#else
s32 SND_fn_lCreateNewBufferSound(s32 voice, s32 extra0, s32 extra1, s32 extra2, _SoundParam* param3D,
                                  s32 extra3, s32 user)
{
    return SND_fn_lCreateNewBufferSnd(voice, extra0 & 65535, extra1 & 65535, extra2, param3D, extra3, user);
}
#endif

#ifdef PC_PORT
void SND_fn_vDeleteBufferSound(void* buffer)
{
    SND_fn_vDeleteBufferSnd(buffer);
}

void SND_fn_vPauseBufferSound(void* buffer)
{
    SND_fn_vPauseBufferSnd(buffer);
}

void SND_fn_vResumeBufferSound(void* buffer)
{
    SND_fn_vResumeBufferSnd(buffer);
}

s32 SND_fn_rGetPosBufferSnd(void* buffer);

s32 SND_fn_rGetPosBufferSound(void* buffer)
{
    return SND_fn_rGetPosBufferSnd(buffer);
}

s32 SND_fn_lCreateNewBufferExSnd(s32* desc, s32* params, s32 a2, s32 a3);

s32 SND_fn_lCreateNewBufferExSound(s32* desc, s32* params, s32 a2, s32 a3)
{
    return SND_fn_lCreateNewBufferExSnd(desc, params, a2, a3);
}

s32 SND_fn_lGetPosBufferExSnd(void* buffer);

s32 SND_fn_lGetPosBufferExSound(void* buffer)
{
    return SND_fn_lGetPosBufferExSnd(buffer);
}

s32 SND_fn_lPushBufferExSnd(void* buffer, s32* data);

s32 SND_fn_lPushBufferExSound(void* buffer, s32* data)
{
    return SND_fn_lPushBufferExSnd(buffer, data);
}

void SND_fn_vSetParamBufferSound(void* buffer, const void* param3D)
{
    SND_fn_vSetParamBufferSnd(buffer, param3D);
}
#else
void SND_fn_vDeleteBufferSound(s32 lBuffer)
{
    SND_fn_vDeleteBufferSnd(lBuffer);
}

void SND_fn_vPauseBufferSound(s32 lBuffer)
{
    SND_fn_vPauseBufferSnd(lBuffer);
}

void SND_fn_vResumeBufferSound(s32 lBuffer)
{
    SND_fn_vResumeBufferSnd(lBuffer);
}

s32 SND_fn_rGetPosBufferSound(s32 lBuffer)
{
    return SND_fn_rGetPosBufferSnd(lBuffer);
}

s32 SND_fn_lCreateNewBufferExSound(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                   _SoundParam* param3D, void* user)
{
    return SND_fn_lCreateNewBufferExSnd(description, format, param3D, user);
}

s32 SND_fn_lGetPosBufferExSound(s32 lBuffer)
{
    return SND_fn_lGetPosBufferExSnd(lBuffer);
}

s32 SND_fn_lPushBufferExSound(s32 lBuffer, TransitionPushArg* data)
{
    return SND_fn_lPushBufferExSnd(lBuffer, data);
}

void SND_fn_vSetParamBufferSound(s32 lBuffer, _SoundParam* param3D)
{
    SND_fn_vSetParamBufferSnd(lBuffer, param3D);
}
#endif

s32 SND_fn_lGetSwitchIndice(s32 a0, s32 superObject, SndBinResource* resource)
{
    s32 stride;
    s32* table;
    u8* recordAddress;
    s32 (*getIndex)(s32, s32);

    stride = 0x74;
    recordAddress = (u8*) (a0 * stride);
    table = (s32*) inv_SND_objectSoundTypes;
    a0 = superObject;
    recordAddress = recordAddress + (s32) table;
    getIndex = *(s32(**)(s32, s32))(recordAddress + 0x20);
    return getIndex(a0, resource->swtch.inv_ulInfoType);
}

s32 SND_fn_lGetNbVoiceOptimumSound(void)
{
    return SND_fn_lGetNbVoiceOptimumSnd();
}

s32 SND_fn_lGetNbVoiceWishedSound(void)
{
    return SND_fn_lGetNbVoiceWishedSnd();
}

#ifdef PC_PORT
void SND_fn_vSetNbVoiceWishedSound(s32 a0)
{
    SND_fn_vSetNbVoiceWishedSnd(a0);
}
#else
void SND_fn_vSetNbVoiceWishedSound(s32 count)
{
    SND_fn_vSetNbVoiceWishedSnd(count);
}
#endif

void SND_fn_vChangeVolumeSound(s32 object, s32 typeIndex, s32 volume)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE0C);
    fn_vChangeVolumeForObject(object, typeIndex, volume);
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResetVolumeSound(s32 object, s32 typeIndex)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE19);
    fn_vResetVolumeForObject(object, typeIndex);
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

s32 SND_fn_lGetVolumeObjectSound(s32 object, s32 typeIndex)
{
    s32 volume;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE28);
    volume = fn_lGetVolumeForObject(object, typeIndex);
    SND_fn_vQuitCriticalSectionThreadSnd();
    return volume;
}

void SND_fn_vChangeVolumeAllObjectButOneSound(s32 skip8, s32 skipC, s32 vol)
{
    Association* node;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE3F);
    node = association.first;
    if (node != 0) {
        do {
            if (node->active != 0) {
                if (node->object != skip8) {
                    if (node->type != skipC) {
                        fn_vChangeVolumeForObject((s32) node->object, node->type, vol);
                    }
                }
            }
            node = node->next;
        } while (node != 0);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResetVolumeAllObjectButOneSound(s32 excludeObj, s32 excludeSnd)
{
    Association* node;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE54);
    node = association.first;
    if (node != 0) {
        do {
            if (node->active != 0) {
                if ((s32) node->object != excludeObj) {
                    if (node->type != excludeSnd) {
                        fn_vResetVolumeForObject((s32) node->object, node->type);
                    }
                }
            }
            node = node->next;
        } while (node != 0);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vChangeVolumeTypeSound(s32 typeIndex, s32 volume)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE6C);
    fn_vChangeVolumeForType(typeIndex, volume);
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResetVolumeTypeSound(s32 typeIndex)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE79);
    fn_vChangeVolumeForType(typeIndex, 0x7F);
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

s32 SND_fn_lGetVolumeTypeSound(s32 typeIndex)
{
    s32 volume;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE88);
    volume = fn_lGetVolumeForType(typeIndex);
    SND_fn_vQuitCriticalSectionThreadSnd();
    return volume;
}

void SND_fn_vChangeVolumeButOneTypeSound(s32 skip, s32 vol)
{
    s32 typeIndex;

    typeIndex = 0;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xE9D);
    if (gi_NbObjectType > 0) {
        do {
            if (typeIndex != skip) {
                fn_vChangeVolumeForType(typeIndex, vol);
            }
            typeIndex += 1;
        } while (typeIndex < gi_NbObjectType);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResetVolumeButOneTypeSound(s32 skip)
{
    s32 typeIndex;

    typeIndex = 0;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xEAE);
    if (gi_NbObjectType > 0) {
        do {
            if (typeIndex != skip) {
                fn_vResetVolumeForType(typeIndex);
            }
            typeIndex += 1;
        } while (typeIndex < gi_NbObjectType);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vChangeVolumeAllObjectSound(s32 obj, s32 vol)
{
    Association* node;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xEC6);
    node = association.first;
    if (node != 0) {
        do {
            if (node->active != 0) {
                if (node->type == obj) {
                    fn_vChangeVolumeForObject((s32) node->object, node->type, vol);
                }
            }
            node = node->next;
        } while (node != 0);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResetVolumeAllObjectSound(s32 obj)
{
    Association* node;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xEDB);
    node = association.first;
    if (node != 0) {
        do {
            if (node->active != 0) {
                if (node->type == obj) {
                    fn_vResetVolumeForObject((s32) node->object, node->type);
                }
            }
            node = node->next;
        } while (node != 0);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vChangeVolumeAllTypeSound(s32 vol)
{
    s32 typeIndex;

    typeIndex = 0;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xEF5);
    if (gi_NbObjectType > 0) {
        do {
            fn_vChangeVolumeForType(typeIndex, vol);
            typeIndex += 1;
        } while (typeIndex < gi_NbObjectType);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResetVolumeAllTypeSound(void)
{
    s32 typeIndex;

    typeIndex = 0;
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sound3d.c", 0xF05);
    if (gi_NbObjectType > 0) {
        do {
            fn_vResetVolumeForType(typeIndex);
            typeIndex += 1;
        } while (typeIndex < gi_NbObjectType);
    }
    SND_fn_vSynchroSound();
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void* func_0015DD48(s32 a0, s32 a1, s32 a2)
{
    s32* node;

    if (g_bInitDoneSound != 0) {
        node = SND_fn_pvMallocSndEx(0, 0x1C);
        memset(node, 0, 0x1C);
        node[1] = 1;
        node[0x18 / 4] |= 3;
        node[2] = SND_fn_uGenerateResourceSnd(a0, a1, a2);
        return node;
    }
    return (void*) D_00393830;
}

void* func_0015DDE8(s32 a0)
{
    s32* node;

    if (g_bInitDoneSound != 0) {
        node = SND_fn_pvMallocSndEx(0, 0x1C);
        memset(node, 0, 0x1C);
        node[1] = 2;
        node[0x18 / 4] |= 3;
        node[2] = a0;
        return node;
    }
    return (void*) D_00393830;
}

void SND_fn_vDestroyEventSound(_SND_tdstBlockEvent* instance)
{
    if (instance != 0) {
        switch (instance->eType) {
        case 2:
            SND_fn_vFreeSndEx(0, instance);
            break;
        case 1:
            SND_fn_vDestroyResourceSnd((void*) instance->uParams.stPlay.uResource);
            SND_fn_vFreeSndEx(0, instance);
            break;
        default:
            SND_fn_vDisplayError(0x13, unk_393858);
            break;
        }
    }
}

int func_0015DEF0(void)
{
    return 0;
}

int func_0015DEF8(void)
{
    return -1;
}

int func_0015DF00(void)
{
    return 0;
}

int func_0015DF08(void)
{
    return 0;
}

void func_0015DF10(void)
{
}

int func_0015DF18(void)
{
    return 0;
}

int func_0015DF20(void)
{
    return 0;
}

int func_0015DF28(void)
{
    return 0;
}

void SND_fn_vSetMusicFadeOut(float value)
{
    inv_SND_bThemeFadeStarted = 0;
    inv_SND_lThemeFadeDuration = (s32) (value * 65536.0f);
}
