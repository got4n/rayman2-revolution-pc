#ifndef REVO_GAM_GAM_H
#define REVO_GAM_GAM_H

#include "ps2/types.h"
#include "MTH/MTH.h"
#include "ps2/copy_types.h"
#include "CAM/CAM.h"
#include "SCT/SCT.h"
#include "snd/snd.h"
#include "HIE/HIE.h"
#include "GLI/GLD.h"
#include "MEC/MEC.h"
#include "WP/WP.h"

typedef struct tdstState_ {
    struct tdstAnim3d_* p_stAnim;
    struct {
        struct tdstTransition_* d_stElements;
        s32 lNumberOfElements;
    } hForTransitionArray;
    struct {
        struct tdstState_** d_hElements;
        s32 lNumberOfElements;
    } hForProhibitArray;
    struct tdstState_* h_NextState;
    void* h_LinkedMechanicsIdCard;
    u8 ucRepeatAnimation;
    s8 scSpeedAnim;
    u8 ucTransitionStatusFlag;
    u8 ucCustomBits;
} tdstState_;

typedef struct tdstTransition_ {
    tdstState_* h_TargetState;
    tdstState_* h_StateToGo;
    u8 ucLinkingType;
} tdstTransition_;

typedef union Data3dShadowControl_inv {
    s32 flags;
    u8 mode;
    u8 bytes[4];
} Data3dShadowControl_inv;

typedef union Data3dShadowSource_inv {
    void* texture;
    u8 listKind;
} Data3dShadowSource_inv;

typedef struct Data3dShadow_inv {
    Data3dShadowControl_inv control;
    f32 scaleX;
    f32 scaleY;
    Data3dShadowSource_inv source;
} Data3dShadow_inv;

#define SHADOW_QUEUE_SIZE 20

typedef struct ShadowEntry_inv {
    HIE_tdstSuperObject_* superObject;
    HIE_tdstSuperObject_* sectorObject;
    struct GliMaterialData* material;
    f32 scale;
    Data3dShadow_inv shadow;
    POS_tdstMatrix_ projection;
    POS_tdstMatrix_ matrix;
    f32 viewMinScale;
    f32 viewRadius;
    struct tdstFogParams_* fogParameters;
} ShadowEntry_inv;

typedef struct ShadowQueue_inv {
    u32 count;
    ShadowEntry_inv entries[SHADOW_QUEUE_SIZE];
    u32 firstCursor;
    u32 firstCapacity;
    struct GliMaterialData* firstMaterials[SHADOW_QUEUE_SIZE];
    u32 secondCursor;
    u32 secondCapacity;
    struct GliMaterialData* secondMaterials[SHADOW_QUEUE_SIZE];
} ShadowQueue_inv;

extern ShadowQueue_inv inv_SHW_stShadowQueue;

typedef struct tdstElement3d_ {
    u16 wElement;
    u8 ucChannelNumber;
    u8 eTypeOfElement;
    u8 ucTransparency;
    u8 ucAnimLevel;
} tdstElement3d_;

typedef struct tdstCouple_ {
    u16 wChild;
    u16 wFather;
} tdstCouple_;

typedef struct tdstPLA_Hierarchy_ {
    u32 ulNbOfCouples;
    tdstCouple_* d_stCouples;
} tdstPLA_Hierarchy_;

typedef struct tdstFrame3d_ {
    tdstElement3d_* p_stArrayOfElts3d;
    MTH3D_tdstMatrix_* p_stAngularSpeedMatrix;
    MTH3D_tdstVector_* p_stLinearSpeedVector;
    tdstPLA_Hierarchy_ stHierarchy;
    struct tdstAnim3d_* p_stAnim;
    u16 uwFrameNumber;
    u8 ucMatrixUpdated;
    u8 pad1B;
} tdstFrame3d_;

typedef struct tdstMorphChannelList_ tdstMorphChannelList_;

typedef struct LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn {
    tdstMorphChannelList_* hFirstElementDyn;
    tdstMorphChannelList_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn;

struct tdstMorphChannelList_ {
    tdstMorphChannelList_* hNextBrotherDyn;
    tdstMorphChannelList_* hPrevBrotherDyn;
    LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hFatherDyn;
    struct PO_tdstPhysicalObject_* from;
    struct PO_tdstPhysicalObject_* to;
    u16 uwChannelNumber;
    f32 blend;
};

typedef struct gu_tdst3dData_ {
    void* h_InitialState;
    tdstState_* h_CurrentState;
    tdstState_* h_FirstStateOfAction;
    struct tdstObjectsTablesList_* h_InitialObjectsTable;
    struct tdstObjectsTablesList_* h_CurrentObjectsTable;
    struct tdstFamilyListView* h_Family;
    POS_tdstMatrix_ stGLIObjectMatrix;
    u16 uwCurrentFrame;
    u8 ucRepeatAnimation;
    u8 ucNextEvent;
    u8* d_ucEventActivation;
    tdstCouple_* p_stCurrentHieCouples;
    s16 wCurrentHieNbCouples;
    s16 wSizeOfArrayOfElts3d;
    tdstState_* h_StateInLastFrame;
    tdstState_* h_WantedState;
    s16 sForcedFrame;
    u8 ucFlagEndState;
    u8 ucFlagEndOfAnim;
    struct tdstAChannel_* hArrayOfChannels;
    s32 ulNumberOfChannels;
    struct tdstAChannel_* hFirstActiveChannel;
    tdstFrame3d_ stFrame3d;
    LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hMorphChannelList;
    s32 ulStartTime;
    s32 ulTimeDelay;
    s32 ulTimePreviousFrame;
    s16 sLastFrame;
    u8 bStateJustModified;
    u8 bSkipCurrentFrame;
    Data3dShadow_inv shadow;
    u16 uwNbEngineFrameSinceLastMechEvent;
    u8 ucFrameRate;
    u8 ucFlagModifState;
    s32 lDrawMaskInit;
    s32 lDrawMask;
    s32 lLastComputeFrame;
    MTH3D_tdstVector_ stLastEventGlobalPosition;
    u8 bAnimMatrixChanged;
    u8 ucUserEventFlags;
    u8 ucBrainComputationFrequency;
    s8 cBrainCounter;
    s16 uwBrainMainCounter;
    u8 ucTransparency;
} gu_tdst3dData_;

typedef struct StandardGameList {
    struct gu_tdstStandardGame* first;
    struct gu_tdstStandardGame* last;
    s32 count;
} StandardGameList;

extern StandardGameList* inv_GAM_currentCharacterList;
extern StandardGameList* inv_GAM_previousCharacterList;

extern s32 g_lSoundObjectTypeAnim;
extern s32 g_lSoundObjectTypeVoice;
extern s32 g_lSoundObjectTypeMenu;
extern s32 inv_SND_moduleObjectSoundType;
extern s32 inv_SND_moduleVoiceObjectSoundType;

typedef struct gu_tdstStandardGame {
    struct gu_tdstStandardGame* listNext;
    struct gu_tdstStandardGame* listPrev;
    StandardGameList* listAnchor;
    u8 queueKey0C;
    u8 ucPlatFormType;
    u16 otObjectFamilyType;
    u16 otObjectModelType;
    u16 otObjectPersonalType;
    u16 inv_uwSaveId;
    void* h_SuperObject;
    u8 eInitFlagWhenOutOfZone;
    u8 eInitFlagWhenDeadOrTaken;
    s32 ulLastTrame;
    u32 ubf32Capabilities;
    u8 ucTractionFactor;
    u8 ucHitPoints;
    u8 ucHitPointsMax;
    u8 ucHitPointsMaxMax;
    u32 ulCustomBits;
    u32 ulAICustomBits;
    u8 ucMiscFlags;
    u8 ucTransparencyZoneMin;
    u8 ucTransparencyZoneMax;
    u8 pad37;
    u32 ulSaveCustomBits;
    u32 ulSaveAICustomBits;
    u8 ucSaveHitPoints;
    u8 ucSaveHitPointsMax;
    u8 ucSaveMiscFlags;
    u8 ucTooFarLimit;
} gu_tdstStandardGame;

static inline void inv_fn_vIsolateStandardGame(gu_tdstStandardGame* node)
{
    if (node->listNext != 0) {
        node->listNext->listPrev = node->listPrev;
    }
    if (node->listPrev != 0) {
        node->listPrev->listNext = node->listNext;
    }
    if (node->listAnchor != 0) {
        node->listAnchor->count--;
        if (node->listAnchor->first == node) {
            node->listAnchor->first = node->listNext;
        }
        if (node->listAnchor->last == node) {
            node->listAnchor->last = node->listPrev;
        }
    }
    node->listNext = 0;
    node->listPrev = 0;
    node->listAnchor = 0;
}

typedef enum tdeObjectTreeInit_ {
    OTI_WhenGameStart = 0,
    OTI_MapLoaded = 1,
    OTI_ReinitTheMap = 2,
    OTI_LoadSavedGame = 3,
    OTI_PlayerDead = 4,
    OTI_WhenGoOutOfZone = 4,
    OTI_Always = 4,
    OTI_AlwaysCreated = 5
} tdeObjectTreeInit_;

typedef struct tdstMicro_ {
    POS_tdstMatrix_* hMicroMatrix;
    u8 bIsActive;
} tdstMicro_;

typedef struct tdstEngineObject_ {
    gu_tdst3dData_* h_3dData;
    gu_tdstStandardGame* h_StandardGame;
    tdstDynam_* h_Dynam;
    struct tdstBrain_* h_Brain;
    tdstCineinfo_* h_Cineinfo;
    struct CollisionSet* h_CollSet;
    struct tdstMSWay_* h_MSWay;
    void* h_MSLight;
    tdstSectInfo_* h_SectInfo;
    struct tdstMicro_* h_Micro;
    RevoMSSoundAttenuation* h_MSSound;
} tdstEngineObject_;

typedef void (*ChannelCopyFn)(tdstEngineObject_* object, tdstEngineObject_* model);
typedef void (*ChannelCreateFn)(void** slots);
typedef void (*ChannelInitFn)(tdstEngineObject_* object, s32 mode);

typedef struct ChannelOps {
    ChannelCopyFn copy;
    ChannelCreateFn create;
    ChannelInitFn init;
    ChannelInitFn desinit;
} ChannelOps;

static inline tdstEngineObject_* inv_HIE_fn_p_stGetEngineObject(HIE_tdstSuperObject_* hSupObj)
{
    return HIE_fn_hGetSuperObjectObject(hSupObj);
}

static inline HIE_tdstSuperObject_* inv_fn_hGetOwnerSuperObject(tdstEngineObject_* object)
{
    return object->h_StandardGame->h_SuperObject;
}

static inline MTH3D_tdstVector_* inv_HIE_fn_p_stGetGlobalTranslation(HIE_tdstSuperObject_* hSupObj)
{
    return inv_POS_fn_p_stPosMatrixTranslation(hSupObj->gu_hGlobalMatrix);
}

static inline struct CollisionSet* inv_GAM_fn_p_stCollisionSetOf(tdstEngineObject_* object)
{
    return object->h_CollSet;
}

static inline void** inv_GAM_fn_pp_vChannelsOf(tdstEngineObject_* object)
{
    return (void**) object;
}

#define inv_LST2_M_vIsolateElement(_hElement)                                                      \
    {                                                                                              \
        if ((_hElement)->hNextBrotherDyn != NULL)                                                  \
            (_hElement)->hNextBrotherDyn->hPrevBrotherDyn = (_hElement)->hPrevBrotherDyn;          \
        if ((_hElement)->hPrevBrotherDyn != NULL)                                                  \
            (_hElement)->hPrevBrotherDyn->hNextBrotherDyn = (_hElement)->hNextBrotherDyn;          \
        if ((_hElement)->hFatherDyn != NULL) {                                                     \
            (_hElement)->hFatherDyn->lNumberOfElementsDyn--;                                       \
            if ((_hElement)->hFatherDyn->hFirstElementDyn == (_hElement))                          \
                (_hElement)->hFatherDyn->hFirstElementDyn = (_hElement)->hNextBrotherDyn;          \
            if ((_hElement)->hFatherDyn->hLastElementDyn == (_hElement))                           \
                (_hElement)->hFatherDyn->hLastElementDyn = (_hElement)->hPrevBrotherDyn;           \
        }                                                                                          \
        (_hElement)->hNextBrotherDyn = NULL;                                                       \
        (_hElement)->hPrevBrotherDyn = NULL;                                                       \
        (_hElement)->hFatherDyn = NULL;                                                            \
    }

#define inv_LST2_M_vAddTail(_p_stList, _hElement)                                                  \
    {                                                                                              \
        (_hElement)->hNextBrotherDyn = NULL;                                                       \
        (_hElement)->hPrevBrotherDyn = (_p_stList)->hLastElementDyn;                               \
        (_hElement)->hFatherDyn = (_p_stList);                                                     \
        if ((_p_stList)->hFirstElementDyn == NULL)                                                 \
            (_p_stList)->hFirstElementDyn = (_hElement);                                           \
        else                                                                                       \
            (_p_stList)->hLastElementDyn->hNextBrotherDyn = (_hElement);                           \
        (_p_stList)->hLastElementDyn = (_hElement);                                                \
        (_p_stList)->lNumberOfElementsDyn++;                                                       \
    }

#define LST2_M_vAddElementToList(_p_stList, _hElement)                                             \
    {                                                                                              \
        inv_LST2_M_vIsolateElement(_hElement)                                                      \
        inv_LST2_M_vAddTail(_p_stList, _hElement)                                                  \
    }

typedef struct EngineList {
    void* first;
    void* last;
    s32 count;
} EngineList;

typedef struct tdstAlwaysActiveCharacter_ tdstAlwaysActiveCharacter_;
typedef struct tdstFamilyListView tdstFamilyListView;

typedef struct LST2_tdstAnchorTotdxHandleToFamilyListDyn {
    tdstFamilyListView* hFirstElementDyn;
    tdstFamilyListView* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorTotdxHandleToFamilyListDyn;

typedef struct LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn {
    tdstAlwaysActiveCharacter_* hFirstElementDyn;
    tdstAlwaysActiveCharacter_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn;

struct tdstAlwaysActiveCharacter_ {
    HIE_tdstSuperObject_* hAlwActSuperObject;
    tdstAlwaysActiveCharacter_* hNextBrotherDyn;
    tdstAlwaysActiveCharacter_* hPrevBrotherDyn;
    LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn* hFatherDyn;
};

typedef struct tdstViewportManagement_ {
    HIE_tdstSuperObject_* hCamera;
    HIE_tdstSuperObject_* hTempCamera;
    struct GLI_tdstCamera_* p_stCamera;
    u8 bValid;
} tdstViewportManagement_;

typedef struct tdstEngineStructure_ {
    u8 eEngineMode;
    char szLevelName[0x1E];
    char szNextLevelName[0x1E];
    u32 ulDisplayMode;
    s16 hGLDDevice;
    s16 hGLDViewport;
    GLD_tdstViewportAttributes_ stViewportAttr;
    struct GLI_tdstCamera_* p_stGameViewportCamera;
    s16 hGLDFixViewport;
    GLD_tdstViewportAttributes_ stFixViewportAttr;
    GLI_tdstSpecificAttributesFor3D_ stFixAttributes3D;
    struct GLI_tdstCamera_* p_stFixCamera;
    tdstViewportManagement_* a_hViewportArray;
    LST2_tdstAnchorTotdxHandleToNodeCameraDyn hCameraList;
    LST2_tdstAnchorTotdxHandleToFamilyListDyn hFamilyList;
    LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn hAlwaysActiveCharactersList;
    HIE_tdstSuperObject_* initialSuperObject;
    HIE_tdstSuperObject_* activeSuperObject;
    HIE_tdstSuperObject_* pendingSuperObject;
    HIE_tdstSuperObject_* h_StdCamCaracter;
    u8 unk174[4];
    POS_tdstMatrix_ stMainCharacterPosition;
    POS_tdstMatrix_ stMainCameraPosition;
    u8 bEngineIsInPaused;
    u8 bEngineHasInPaused;
    char a_szLevelName[150][0x1E];
    char levelNumber[0x1E];
    char szFirstLevelName[0x1E];
    u8 ucNumberOfLevels;
    u8 ucCurrentLevel;
    u8 ucPreviousLevel;
    u8 ucExitIdToQuitPrevLevel;
    u8 ucLevelGlobalCounter;
    u8 xDemoMode;
    u8 ucCurrentLanguage;
    u8 ucNbLanguages;
    u8 bEngineFrozen;
    u8 bResurection;
    char cCameraMode;
} tdstEngineStructure_;

extern tdstEngineStructure_ g_stEngineStructure;

struct tdstAlwaysModelList_;

typedef struct LST2_tdstAnchorToALW_tdxHandleToModelListDyn {
    struct tdstAlwaysModelList_* hFirstElementDyn;
    struct tdstAlwaysModelList_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorToALW_tdxHandleToModelListDyn;

typedef struct tdstAlwaysModelList_ {
    struct tdstAlwaysModelList_* hNextBrotherDyn;
    struct tdstAlwaysModelList_* hPrevBrotherDyn;
    LST2_tdstAnchorToALW_tdxHandleToModelListDyn* hFatherDyn;
    tdstEngineObject_* p_stAlwaysObject;
    u16 otObjectModelType;
} tdstAlwaysModelList_;

typedef struct tdstAlways_ {
    u32 ulMaxNumberOfAlways;
    LST2_tdstAnchorToALW_tdxHandleToModelListDyn ALW_h_LstAlwaysModel;
    u8* used;
    HIE_tdstSuperObject_* d_stAlwaysSuperObject;
    tdstEngineObject_* d_stAlwaysEngineObjectInit;
    HIE_tdstSuperObject_** d_hAlwaysGenerator;
} tdstAlways_;

typedef struct tdstAChannel_ {
    HIE_tdstSuperObject_* hSupObject;
    HIE_tdstSuperObject_* hZoomSupObject;
    struct tdstAChannel_* p_stNextActiveChannel;
    struct tdstAChannel_* p_stFatherChannel;
    u8 bControlledChannel;
    u8 bActiveChannel;
    u8 unk12[2];
    MTH3D_tdstVector_ inv_stLastPosition;
} tdstAChannel_;

typedef void* (*MechanicsCallback)(DNM_stDynamics* dynamics, HIE_tdstSuperObject_* superObject,
                                   tdstDynam_* component, void* idCard, f32 delta);

typedef struct tdstMSWay_ {
    WP_tdstGraph_* m_hPath;
    s32 m_lIndex;
    s8 m_bIsCircular;
} tdstMSWay_;

extern s32 fn_bGetIfCharacterIsInAlwaysActiveCharacterList(HIE_tdstSuperObject_* hCharacter);
extern void fn_vAddCharacterInAlwaysActiveCharacterList(HIE_tdstSuperObject_* character);
extern void inv_ALWAYS_fn_vRemoveCharacterFromAlwaysActiveCharacterList(
    HIE_tdstSuperObject_* character);
extern HIE_tdstSuperObject_* fn_hGetCharacterInAlwaysActiveCharacterNode(
    tdstAlwaysActiveCharacter_* hCharNode);
extern tdstAlwaysActiveCharacter_* fn_hGetFirstAlwaysActiveCharNode(void);
extern tdstAlwaysActiveCharacter_* fn_hGetNextAlwaysActiveCharNode(
    tdstAlwaysActiveCharacter_* hCharNode);

extern void fn_vFreeNextLevelName(void);
extern char* inv_fn_p_szGetFullLevelName(void);
extern s32 fn_bIsNextLevelNameValid(void);
extern void GAM_fn_vActualizeAllGameViewportSize(void);
extern void GAM_fn_vSaveBlockEventSlotMemory(s32 event, s32 slot);
extern void GAM_fn_vCreateGameDeviceAndViewPorts(void);
extern void GAM_fn_vSetEngineInFixMode(void);
extern void GAM_fn_vSetEngineInLevelMode(void);
extern void fn_vChangeViewPortPercent(s32 gameViewport, s32 attribute, s32 widthPercent,
                                      s32 heightPercent, s32 clipTop, s32 clipLeft, s32 clipBottom,
                                      s32 clipRight, s32 originX, s32 originY);
extern void fn_vChangeViewportRatio(s32 gameViewport, f32 ratio);
extern void fn_vDestroyFixViewPort(void);
extern void fn_vReputGameViewportCamera(void);
extern void fn_vResetEngineFromPaused(void);
extern void fn_vSetEngineInPaused(void);
extern void inv_fn_vGetRandomLevelName(char* name);
extern void func_00112E60(void);
extern void fn_vSetNextLevelName(const char* name);
extern void GAM_fn_bIsSlotValid(s32 type, u8 value);
extern void GAM_fn_vAskToChangeLevel(const char* szNewLevelName);
extern void* fn_lGetSubMapNumber(void);
extern u8 fn_bIsEngineInPause(void);
extern void fn_vChangeEngineMode(u8 eNewMode);
extern char* fn_p_szGetFirstLevelName(void);
extern char* fn_p_szGetLevelName(void);
extern char* fn_p_szGetNextLevelName(void);
extern char* inv_fn_p_szGetLevelNumber(void);
extern void fn_vSetLevelName(const char* _szName);

extern s32 fn_lGetTemporalCurveType(void);
#ifdef PC_PORT
extern f32 inv_GAM_fn_xGetEngineClockTicks(void);
#endif
extern f32 inv_fn_xGetEngineClockScale(void);
extern f32 inv_fn_xGetEngineClockTime(void);
extern s32 inv_fn_vInitEngineClockLibrary(void);
extern s32 fn_vSaveEngineClock(void);
extern s32 fn_vLoadEngineClock(void);
extern f32 inv_fn_xGetEngineClockDelta(void);
extern u32 inv_fn_ulGetEngineTrameNumber(void);
extern u32 inv_fn_ulGetEngineClock(void);

extern void func_001003B0(void);

extern void func_00101438(HIE_tdstSuperObject_* superObject);
extern void fn_vDestructAllFatherSonLinks(void);
extern void fn_vDisplayAll(void);
extern void fn_vReputCharacterSuperObjectAtTheWorld(HIE_tdstSuperObject_* h_SupObject);
extern void fn_vTreatDynamicHierarchy(HIE_tdstSuperObject_* superObject, s32 treatChildren);
extern s32 inv_fn_bIsSuperObjectInDynamicWorld(HIE_tdstSuperObject_* superObject);
extern void fn_vEngine(void);
extern s32 GAM_fn_lGetNbPersoActive(void);
extern void fn_vTreatCharactersInSector(HIE_tdstSuperObject_* hCharacter);

extern void fn_vDesInitDeadLoop(void);
extern void fn_vInitLevelLoop(void);
extern void fn_vDesInitLevelLoop(void);
extern void fn_vFirstInitEngine(void);
extern void fn_vInitDeadLoop(void);
extern void fn_vInitGameLoop(void);
extern void fn_vLastDesinitEngine(void);
extern void fn_vReinitTheMap(void);

typedef struct MemoryBlockView {
    struct MemoryBlockView* next;
    struct MemoryBlockView* prev;
    struct MemoryBlockView* end;
} MemoryBlockView;

typedef struct MemoryListView {
    MemoryBlockView* first;
    MemoryBlockView* last;
    s32 count;
} MemoryListView;

typedef struct MemoryHeapView {
    union {
        MemoryListView list;
        u8* top;
    } used;

    MemoryListView freeList;
    MemoryBlockView* end;
    u16 align;
    u16 flags;
    u32 usedSize;
    u32 peakSize;
} MemoryHeapView;

extern void inv_MEM_fn_vFree(MemoryHeapView** slot);
extern s32 inv_MEM_fn_bIsInHeap(void* heap, void* address);
extern void inv_MEM_fn_vInitDefaultHeap(u32 size);
extern u32 inv_fn_ulGetBiggestFreeBlockSize(void);
extern void* inv_MEM_fn_p_vAlloc(MemoryHeapView* heap, u32 size);
extern void* inv_MEM_fn_p_vRealloc(MemoryHeapView* heap, void* ptr, u32 size);
extern u32 inv_MEM_fn_ulGetBlockSize(MemoryHeapView* heap, void* ptr);
extern MemoryHeapView* inv_MEM_fn_p_vAllocAligned(void* memory, u32 size, u16 align, u16 flags);
extern void inv_MEM_fn_vFreeInHeap(MemoryHeapView* heap, void* ptr);

extern void PLA_fn_vUpdateTransparencyForModules(HIE_tdstSuperObject_* p_stSuperObject);
extern s32 PLA_fn_bSkipCurrentFrame(HIE_tdstSuperObject_* p_stSuperObject);
extern s32 PLA_fn_stGetAngularSpeedMatrix(HIE_tdstSuperObject_* obj, s16 frame,
                                          MTH3D_tdstMatrix_* out);
extern s32 PLA_fn_stGetLinearSpeedVector(HIE_tdstSuperObject_* self, s16 start_idx,
                                         MTH3D_tdstVector_* out);
extern void PLA_fn_vCreateAndInitHeaps(void);
extern void PLA_fn_vInitHeaps(void);
extern void PLA_fn_vRefreshGlobalMatrixUnderCharacter(HIE_tdstSuperObject_* superObject);
extern int PLA_fn_vInitNewState(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_State);
extern s32 PLA_fn_bTestIfEndOfAction(HIE_tdstSuperObject_* p_stSuperObject);
extern s32 PLA_fn_bTestIfEndOfAnim(HIE_tdstSuperObject_* p_stSuperObject);
extern void PLA_fn_vDesInitAllChildOfCharacter(HIE_tdstSuperObject_* p_stSuperObject,
                                               tdstEngineObject_* p_stObject);
extern s32 PLA_fn_bDoAnimPlayerForCharacter(HIE_tdstSuperObject_* p_stSuperObject, u8 bNoDisplay,
                                            u8 _bHandleSkippedEventsIfRelevant);
extern s32 PLA_fn_bSetNewStateWithRandom(HIE_tdstSuperObject_* p_stSuperObject,
                                         tdstState_* h_WantedState);
extern u16 PLA_fn_uwGetNbFrameInCurrAnim(HIE_tdstSuperObject_* p_stSuperObject);
extern void PLA_fn_vSetCurrFrame(HIE_tdstSuperObject_* p_stSuperObject, s16 _wFrame);
extern s32 PLA_fn_bSetNewState(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_WantedState,
                               u8 _bForce, u8 _bHandleSkippedEventsIfRelevant,
                               u8 _bIgnoreTransitions);
extern s32 PLA_fn_bTestCurrentAction(HIE_tdstSuperObject_* p_stSuperObject,
                                     tdstState_* h_FirstStateOfAction);
extern s32 PLA_fn_bTestNewState(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* h_WantedState);
extern void* PLA_fn_hFindNextFreeSupObj(void);

struct tdstEvent_;
extern void fn_vPlayEventsSinceLastOneNoLoop(HIE_tdstSuperObject_* pOwner);
extern void fn_vPlayLastEventsBeforeRestart(HIE_tdstSuperObject_* superObject);
extern void fn_vPlayEvent(struct tdstEvent_* event, s32 backwards,
                          HIE_tdstSuperObject_* superObject);

extern void fn_vValidateAllOption(void);
extern void inv_fn_vFreeLevelDynamicLists(void);
extern s32 inv_SAV_fn_bCreateNewGameSave(void);
extern s32 inv_SAV_fn_bProcessCurrentSaveRequest(void);
extern s32 inv_fn_lGetLevelDynamicListCount(void);
extern u32 inv_SAV_fn_ulGetCurrentGameDuration(void);
extern void inv_SAV_fn_vSetCurrentGameDuration(u32 duration);
extern void inv_SND_fn_vInitDataLoadSnd(void);
extern u8 inv_fn_ucFindLevelNameIndex(const char* name);
extern s32 inv_fn_vDisplayPauseOrOptionsOverlay(void);
extern s32 fn_bSaveCurrentGameOptions(void);
extern s32 inv_SAV_fn_bInitCurrentSaveForPlayer(char* name);
extern u16 inv_SAV_fn_wGetCurrentSlotIndex(void);
extern void fn_vChangeVoiceVolumeValue(u32 volume);
extern void fn_vChangeMusicVolumeValue(u32 volume);
extern void fn_vChangeFxVolumeValue(u32 volume);
extern void inv_SAV_fn_vEnsureLevelDynamicListsLoaded(void);
extern void inv_SAV_fn_vSetCurrentSaveComment(char* comment);
extern char* inv_SAV_fn_szGetCurrentPlayerName(void);
extern char* inv_SAV_fn_szGetCurrentSaveComment(void);

struct SaveSlotInfo;
struct SaveSlot;
extern s32 inv_SAV_fn_bIsSaveDeviceFormatted(void);
extern s32 inv_SAV_fn_bIsSaveDeviceReady(void);
extern void inv_SIF_fn_vInitSoundInfos(void);
extern s32 inv_SAV_fn_lIsUnformattedDeviceReady(void);
extern s32 inv_SAV_fn_bCanSaveCurrentGame(void);
extern s32 inv_fn_bCanSaveGame(s32 size);
extern u8* inv_SAV_fn_p_ucWriteAligned(u8* cursor, void* source, s32 size);
extern u8* inv_SAV_fn_p_ucReadAligned(u8* cursor, void* destination, u32 length);
extern void inv_SAV_fn_vWriteHeader(s32 offset, void* source, u32 size);
extern void inv_SAV_fn_vReadHeader(s32 offset, void* destination, s32 size);
extern u8* inv_SAV_fn_p_ucGetOrCreateSaveRecord(s32 id);
extern u8* inv_SAV_fn_p_ucFindSaveRecord(s32 id);
extern void j_sprintf_SaveDirectoryName(char* directory, const char* name);
extern u32 inv_SAV_fn_ulGetSlotPlayTime(struct SaveSlot* slot);
extern s32 inv_SAV_fn_bDeleteSlot(struct SaveSlot* slot);
extern s32 inv_SAV_fn_bLoadSlotData(struct SaveSlot* slot);
extern s32 inv_fn_bIsSaveSlotValid(struct SaveSlotInfo* info);
extern struct SaveSlot* inv_SAV_fn_p_stAllocateSaveSlot(s16 id, char* name);
extern s32 inv_SAV_fn_bSaveCurrentGame(struct SaveSlot* slot);
extern s32 inv_SAV_fn_bSaveSelectedSlot(struct SaveSlot* slot);
extern struct SaveSlot* inv_SAV_fn_p_stGetSlotByIndex(s16 id);
extern char* inv_SAV_fn_szGetSlotComment(struct SaveSlot* slot);
extern char* inv_SAV_fn_szGetSlotPlayerName(struct SaveSlot* slot);
extern s32 inv_SAV_fn_uwGetSlotIndex(struct SaveSlot* slot);

extern void* gp_6F74;

extern void fn_vFreeLevelAllowedMemory(void);
extern void inv_fn_vAllocateLevelMemory(s32 value);
extern void inv_MEM_fn_vFreeLevelSlotHeap(s32 index);
extern void inv_MEM_fn_vCreateLevelSlotHeap(u32 size, s32 index);
extern void* inv_MEM_fn_p_vAllocAlignedHeap(u32 size);
extern void inv_MEM_fn_vFreeHeap(MemoryHeapView** slot);
extern void* inv_MEM_fn_p_vAllocInHeap(MemoryHeapView* heap, u32 size);

extern void* SAI_fn_p_vGetTableValuePointerOf(void* key, u32 id);

#define SAVED_MATRIX_ID 0x32

static inline POS_tdstMatrix_* inv_GAM_fn_p_stSavedMatrixOf(HIE_tdstSuperObject_* superObject)
{
    return SAI_fn_p_vGetTableValuePointerOf(&inv_HIE_fn_p_stGetEngineObject(superObject)->h_3dData->stGLIObjectMatrix,
                                            SAVED_MATRIX_ID);
}

typedef struct tdstObjectsTableElement_ {
    MTH3D_tdstVector_* p_stCustomZoom;
    s32 h_Target;
    u32 uwChannelNumber;
    u16 wTypeOfTarget;
    u8 ucPhoneme;
    u8 ucIntensity;
    u8 ucExpression;
    u8 pad11[3];
} tdstObjectsTableElement_;

typedef union ObjectTableEntry {
    PackedU64U64U32Aligned4 words;
    tdstObjectsTableElement_ stElement;
} ObjectTableEntry;

typedef struct tdstObjectsTablesList_ {
    union ObjectTableEntry* d_stObjectsTable;
    union ObjectTableEntry* d_stInitObjectsTable;
    u16 wNumberOfElement;
    u16 wZDxUsed;
} tdstObjectsTablesList_;

typedef struct LST2_tdstAnchorTotdxHandleToStateOpt {
    tdstState_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorTotdxHandleToStateOpt;

struct tdstFamilyListView {
    tdstFamilyListView* hNextBrotherDyn;
    tdstFamilyListView* hPrevBrotherDyn;
    LST2_tdstAnchorTotdxHandleToFamilyListDyn* hFatherDyn;
    LST2_tdstAnchorTotdxHandleToStateOpt hForStateArray;
    tdstObjectsTablesList_* h_DefaultObjectsTable;
    tdstObjectsTablesList_* hForObjectsTablesList;
    u8 pad1C[0x04];
    struct COL_tdstBoundingSphere_* hBoundingSphere;
    u32 ulNumberOfChannels;
};

extern void* fn_p_stGetAnimInState(tdstFamilyListView* h_Family);

typedef struct tdstEvent_ {
    void* p_stfEventInTBL;
    u16 usEventNumberInTBL;
    u16 uwFrameNumber;
    u16 uwChannelNumber;
    u16 uwIsLocalized;
} tdstEvent_;

typedef struct tdstEventInTable_ {
    union {
        u32 sound;
        u8 userFlag;
    } p_xEvent;

    u8 ucTypeOfEvent;
    u8 ucPriority;
    u8 ucFirstCall;
    u8 ucPeriod;
} tdstEventInTable_;

typedef struct AnimationMorphEntry {
    u8 target;
    u8 morphingAmount;
    u16 channel;
    u16 frame;
    u16 unk06;
} AnimationMorphEntry;

typedef struct AnimationData {
    MTH3D_tdstVector_* vectors;
    struct PackedQuaternion* quaternions;
    tdstCouple_* links;
    struct AnimationRecord* records;
    struct AnimationFrame* frames;
    struct AnimationChannel* channels;
    u16* remap;
    u16* keyIndices;
    struct AnimationKey* keys;
    tdstEvent_* events;
    AnimationMorphEntry* morphData;
    u8 inv_ucFrameRate;
    u8 unk2D;
    u16 inv_uwVectorCount;
    u16 inv_uwQuaternionCount;
    u16 inv_uwLinkCount;
    u16 inv_uwRecordCount;
    u16 unk36;
    u16 channelCount;
    u16 frameCount;
    u16 inv_uwKeyCount;
    u8 eventCount;
    u8 unk3F;
    u16 unk40;
    u16 unk42;
    u16 offsetVectorIndex;
    u16 unk46;
    u16 morphCount;
} AnimationData;

typedef struct tdstAnim3d_ {
    AnimationData* p_stData;
    AnimationMorphEntry* d_stMorphDataArray;
    tdstEvent_* d_stAnimEvent;
    MTH3D_tdstVector_ stOffsetMatrix;
    u16 uwNumberOfFrames;
    u8 ucFrameRate;
    u8 ucMaxNumberOfElements;
    u8 unk1C[2];
    u8 ucNumberOfEvents;
} tdstAnim3d_;

extern void* g_hCollisionGeoObj;
extern void* g_hStaticCollisionGeoObj;
extern HIE_tdstSuperObject_* g_hCharacterLauchingSoundEvents;
extern HIE_tdstSuperObject_* dword_252618[];
extern u32* g_stBeginMapSoundEvent;

extern void CAM_fn_vAveragePosition(tdstCineinfo_* _hCineinfo, MTH3D_tdstVector_* p_result,
                                    f32 limit);
extern void CAM_fn_vAverageSpeed(tdstCineinfo_* _hCineinfo, MTH3D_tdstVector_* p_result, f32 limit);
extern void CAM_fn_vLoadCameraConstants(void);
extern void CAM_fn_vSetCineinfoCurrentFromVisibility(tdstCineinfo_* _hCineinfo);
extern void CAM_fn_vUpdateTargetPosition(tdstCineinfo_* _hCineinfo);

extern HIE_tdstSuperObject_* fn_p_stAllocateAlways(u16 otObjectModelType, HIE_tdstSuperObject_* p_stFatherSuperObject, HIE_tdstSuperObject_* _hGenerator, s32 uwAction, POS_tdstMatrix_* p_stMatrix);
extern HIE_tdstSuperObject_* inv_ALWAYS_fn_hGetGeneratorOfAlways(HIE_tdstSuperObject_* _hAlways);
extern void fn_vKillAllAlways(void);
extern void fn_vKillAllAlwaysInNonActiveSectors(void);
extern void fn_vKillAllAlwaysModel(void);
extern void fn_vKillAlwaysByPersonalType(u16 otPersonalType);
extern s32 fn_ucIsAlwaysGeneratedByMe(HIE_tdstSuperObject_* _hAlways,
                                      HIE_tdstSuperObject_* _hGenerator);
extern void fn_vInitAlwaysModel(u8 eObjIniType);
extern u32 fn_ulGetNumberOfAlwaysBelongingToMe(HIE_tdstSuperObject_* _hGenerator);
extern u8 fn_ucIsAnAlwaysObject(tdstEngineObject_* p_stEngineObject);

extern void fn_vInitCSOList(tdstAChannel_** p_hChannelArray, u32 ulNumberOfChannels);
extern void fn_vFreeMorphList(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn** p_hMorphList);
extern void fn_vInitMorphList(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn** p_hMorphList);
extern void fn_vRemoveChannelFromMorphList(
    LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hMorphList, tdstMorphChannelList_* element);
extern void fn_vUpdateSPOFlagsOfChannelForActivation(HIE_tdstSuperObject_* _hSOChannel,
                                                     u8 _bActivate);
extern void fn_vActivateChannel(HIE_tdstSuperObject_* _hPerso, u16 _uwChannel, u8 _bActivate);
extern void fn_vTakeControlOfChannel(tdstEngineObject_* _hActor, u16 _uwChannel, u8 _bControlled);
extern s32 fn_vFreeCSOList(tdstAChannel_** p_hChannelArray, u32 ulNumberOfChannels);

extern s32 inv_COL_fn_lTestCharacterAgainstZdxList(DNM_stMecObstacle* hits,
                                                   HIE_tdstSuperObject_* character, void* matrixA,
                                                   void* matrixB);
extern void fn_vChangeCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities,
                                   s32 lValue);

extern void func_0010DA50(HIE_tdstSuperObject_* arg);

extern s32 fn_b3dDataCanComputeBrain(HIE_tdstSuperObject_* _hCharacter);
extern void fn_v3dDataFactorFrameRate(gu_tdst3dData_* _h3dData, s32 _ulFactor100);
extern void fn_v3dDataUpdateBrainCounter(HIE_tdstSuperObject_* _hCharacter);
extern void fn_vAllocArrayOfElts3d(gu_tdst3dData_* h_3dData, u32 _wNumberOfElts3d);
extern void fn_v3dDataSetUserEventFlag(gu_tdst3dData_* h_3dData, u32 _ucFlagIndex, s32 _bSet,
                                       MTH3D_tdstVector_* _p_stPosition);
extern void fn_v3dDataSetBrainComputationFrequency(gu_tdst3dData_* h_3dData, s32 _ucFrequency);
extern void fn_v3dDataSetFrameRate(gu_tdst3dData_* h_3dData, s32 ucFrameRate);
extern void fn_vFreeArrayOfElts3d(gu_tdst3dData_* h_3dData);
extern s32 fn_b3dDataGetUserEventFlag(gu_tdst3dData_* h_3dData, s32 _ucFlagIndex,
                                      MTH3D_tdstVector_* _p_stPosition);

struct tdstZoneSetList_;
extern s32 fn_bGetCharacterForceCollisionWhenNotMoving(HIE_tdstSuperObject_* _hSuperObject);
extern void* CS_fn_hGetActivationList(u8 ucZoneType, HIE_tdstSuperObject_* hSuperObj);
extern void* CS_fn_hGetZdxList(u8 ucZoneType, HIE_tdstSuperObject_* hSuperObj);
extern s32 CS_fn_eGetPrivilegedActivationZone(u8 _ucZoneType, struct CollisionSet* _hCollSet,
                                              s16 _wIndex);
extern void CS_fn_vSetPrivilegedActivationZone(u8 _ucZoneType, struct CollisionSet* _hCollSet,
                                               s16 _wIndex, u8 _ePrivilege);
extern void fn_vSetCurrentZoneSetOfType(u8 _ucZoneType, struct CollisionSet* _hCollSet,
                                        struct tdstZoneSetList_* _hCurrentZoneSet);
extern void* fn_hGetCurrentZoneSetOfType(u8 _ucZoneType, struct CollisionSet* _hCollSet);
extern u32 CS_fn_ulGetPrivilegedActivationZoneInCollSet(u8 _ucZoneType,
                                                        struct CollisionSet* _hCollSet);

extern s32 fn_bRetRollOffFactor(HIE_tdstSuperObject_* superObject, u32* distances);
extern s32 fn_xMSSoundGetBackGroundDistance(RevoMSSoundAttenuation* h_MSSound);
extern s32 fn_xMSSoundGetSaturationDistance(RevoMSSoundAttenuation* h_MSSound);
extern void fn_vMSSoundSetBackGroundDistance(RevoMSSoundAttenuation* h_MSSound,
                                             s32 _xBackGroundDistance);
extern void fn_vMSSoundSetSaturationDistance(RevoMSSoundAttenuation* h_MSSound,
                                             s32 _xSaturationDistance);

extern tdstEngineObject_* fn_hReturnHandleMicroFromIndex(s32 lIndex);
extern tdstEngineObject_* fn_p_stGetFirstEngineObjectInMicroList(void);
extern void fn_vInitMicrosFunction(void);
extern void fn_vRemoveAllMicros(void);
extern void GAM_fn_vChangeCharacterForMicro(HIE_tdstSuperObject_* _hCharacter);

struct GLI_tdstLight_;
struct tdstMSLight_;
extern void fn_vUpdateLight(HIE_tdstSuperObject_* hSupObject);
extern void fn_vMSLightSetBigAlpha(struct tdstMSLight_* h_MSLight, f32 xBigAlpha);
extern void fn_vMSLightSetColorRGBA(struct tdstMSLight_* h_MSLight, f32 xR, f32 xG, f32 xB, f32 xA);
extern void fn_vMSLightSetLittleAlpha(struct tdstMSLight_* h_MSLight, f32 xLittleAlpha);
extern void fn_vMSLightSetNearFar(struct tdstMSLight_* h_MSLight, f32 xNear, f32 xFar);
extern void fn_vMSLightSetOnOff(struct tdstMSLight_* h_MSLight, u8 ucOnOff);
extern void fn_vMSLightSetTypeOfLight(struct tdstMSLight_* h_MSLight, u16 lTypeOfLight);
extern void* fn_hMSLightGetLightNodeInSector(struct tdstMSLight_* light);
extern void fn_vMSLightSetLocalLight(struct tdstMSLight_* h_MSLight, u8 bLocalLight);
extern void fn_vMSLightSetOnlyLocalLight(struct tdstMSLight_* h_MSLight, u8 bOnlyLocalLight);
extern struct GLI_tdstLight_* fn_p_stMSLightGetLight(struct tdstMSLight_* h_MSLight);
extern u8 fn_bMSLightGetLocalLight(struct tdstMSLight_* h_MSLight);
extern u8 fn_bMSLightGetOnlyLocalLight(struct tdstMSLight_* h_MSLight);
extern u8 fn_bMSLightGetGiroPhare(struct tdstMSLight_* h_MSLight);
extern u8 fn_bMSLightGetPulse(struct tdstMSLight_* h_MSLight);
extern u8 fn_ucMSLightGetOnOff(struct tdstMSLight_* h_MSLight);
extern void fn_vMSLightSetGiroPhare(struct tdstMSLight_* h_MSLight, u8 enabled, f32 xStep);
extern void fn_vMSLightSetPulse(struct tdstMSLight_* h_MSLight, u8 enabled, f32 xStep, f32 xMaxRange);

extern void GAM_fn_vSectInfoTestChangeSectorForCharacter(HIE_tdstSuperObject_* _hCharacter);
extern void fn_vSectInfoInitForSingleCharacter(HIE_tdstSuperObject_* hCharacter,
                                               MTH3D_tdstVector_* hPosition);
extern HIE_tdstSuperObject_* GAM_fn_hGetCurrentSector(HIE_tdstSuperObject_* _hCharacter);
extern void fn_vInsertObjectInSectorList(HIE_tdstSuperObject_* hSector,
                                         HIE_tdstSuperObject_* hCharacter);
extern void fn_vRemoveObjectInSectorList(HIE_tdstSuperObject_* hCharacter);

extern void inv_fn_vIsolateCurrentCharacterList(void);
extern void inv_fn_vIsolatePreviousCharacterList(void);
extern void inv_fn_vInitComportOfCurrentCharacters(void);
extern void inv_fn_vClearEngineObjectLists(void);
extern void inv_fn_vDeactivateAllEngineObjectNodes(void);
extern void inv_fn_vInitEngineObjectLists(void);
extern void inv_fn_vRecycleAllActiveEngineObjectNodes(void);
extern void inv_fn_vTreatQueuedCharactersBySector(HIE_tdstSuperObject_* excluded,
                                                  MTH3D_tdstVector_* reference);
extern void inv_fn_vQueueEngineObjectByPriority(gu_tdstStandardGame* node);
extern void inv_fn_vMoveToInactiveCharacterList(gu_tdstStandardGame* node);

extern s32 fn_ucStdGameCheckValidity(HIE_tdstSuperObject_* superObject);
extern void fn_vStandardGameSetCustomBitsSO(HIE_tdstSuperObject_* superObject, u32 customBits);
extern void inv_TXT_fn_vInitTextTables(void);
extern u8 inv_fn_ucSuperObjectGetPlatFormType(HIE_tdstSuperObject_* superObject);
extern AnimationMorphEntry* inv_PLA_fn_p_stFindMorphData(tdstAnim3d_* p_stAnim, s32 channel, u16 frame);
extern void PLA_fn_vStopCurrentMorphAnim(HIE_tdstSuperObject_* superObject);

extern void CAM_fn_vApplyTransparencyOnSuperObject(tdstCineinfo_* _hCineinfo);
extern void CAM_fn_vRestoreTransparencyOnSuperObject(tdstCineinfo_* _hCineinfo);
extern void inv_CAM_fn_vInitTransparentSuperObjectArray(tdstCineinfo_* cineinfo);
extern void CAM_fn_vResetArrayOfTransparentSuperObject(tdstCineinfo_* _hCineinfo);
extern void inv_CAM_fn_vMarkSuperObjectForTransparency(tdstCineinfo_* cineinfo,
                                                       HIE_tdstSuperObject_* superObject);

extern s32 IPT_fn_bIsKeyboardControlAvailable(void);
extern s32 inv_fn_bIsEngineModeChangeRequested(void);
extern void fn_vChooseTheGoodDesInit(void);
extern void fn_vOneTrame(void);

extern void fn_vLastDesinitFamilyStructure(void);

extern void fn_vSetHitPoints(tdstEngineObject_* _p_stObject, u8 _ucValue);
extern void fn_vSetHitPointsMax(tdstEngineObject_* _p_stObject, u8 _ucValue);
extern u8 fn_ucGetHitPoints(tdstEngineObject_* _p_stObject);
extern u8 fn_ucGetHitPointsMax(tdstEngineObject_* _p_stObject);
extern u8 fn_ucSubHitPoints(tdstEngineObject_* _p_stObject, u8 _ucValue);
extern u8 fn_ucSubHitPointsMax(tdstEngineObject_* _p_stObject, u8 _ucValue);
extern void fn_vAddHitPoints(tdstEngineObject_* _p_stObject, s32 _ucValue);
extern void fn_vAddHitPointsMax(tdstEngineObject_* _p_stObject, s32 _ucValue);
extern void fn_vSetHitPointsInit(tdstEngineObject_* _p_stObject, u8 _ucValue);
extern void fn_vSetHitPointsMaxToMaxValue(tdstEngineObject_* _p_stObject);
extern void fn_vSetHitPointsToMaxValue(tdstEngineObject_* _p_stObject);

extern void fn_vEngineReadInput(void);

extern void fn_lInitSoundForAnim(void);
extern s32 GAM_p_stGetBlockEventSlotMemory(s32 s32_Index);

extern void fn_vInitAllObjectsWhenMapJustLoaded(void);
extern void fn_vInitAllObjectsWhenPlayerDead(void);
extern void fn_vKillEngineObjectOrAlwaysByPointer(tdstEngineObject_* p_stObject);
extern void fn_vSnifThePlayerIsDeadWithResurrectionAt(HIE_tdstSuperObject_* _hCharacterPositionOne,
                                                      HIE_tdstSuperObject_* _hCharacterPositionTwo);
extern void fn_vInitAllObjectsWhenNewGameStart(void);
extern void fn_vInitOneObject(tdstEngineObject_* p_stObject, u8 eObjIniType);
extern void fn_vForceReinitAllObjects(void);
extern void fn_vDesactiveAnObject(tdstEngineObject_* p_stObject);
extern void fn_vForceActiveAnObject(tdstEngineObject_* p_stObject);
extern void fn_vSnifThePlayerIsDead(void);
extern void fn_vDestroyAllHierarchy(HIE_tdstSuperObject_* p_stSuperObject);

extern s32 fn_hIfExistProhibitedState(tdstState_* h_State, tdstState_* h_TargetState);
extern tdstTransition_* fn_hIfExistTransitionalWithTargetState(tdstState_* h_State,
                                                               tdstState_* h_TargetState);
extern u32 fn_uwGetObjectTableNumberOfelements(tdstObjectsTablesList_* _hTargetTable);
extern void fn_vCopyObjectFromObjectTabletoAnother(tdstObjectsTablesList_* _hTargetTable,
                                                   tdstObjectsTablesList_* _hSourceTable,
                                                   u16 _uwTargetObject, u16 _uwSourceObject);
extern void fn_vExchangeTwoObjectsInCurrentObjectsTable(HIE_tdstSuperObject_* p_stSuperObject,
                                                        u16 w_ObjectA, u16 w_ObjectB);
extern ObjectTableEntry* fn_d_hGetInitObjectsTable(tdstObjectsTablesList_* h_ObjectsTablesListElement);
extern void fn_vReInitObjectTable(tdstObjectsTablesList_* param0);

extern void fn_v_InitSinTab(void);
extern void fn_v_ExpandQuat(f32* _a4_xQuatD, s16* _a4_xQuatS);
extern void fn_v_InterpolQuatWithOmega(f32* result, f32* from, f32* to, s16 angle, f32 t);

extern MTH3D_tdstMatrix_* fn_p_stGetAngularSpeedMatrix(tdstAnim3d_* _p_stAnim3d, u16 _uwFrame,
                                                       MTH3D_tdstMatrix_* p_stMatrix);
extern MTH3D_tdstVector_* fn_p_stGetLinearSpeedVector(tdstAnim3d_* _p_stAnim3d, u16 _uwFrame);
extern void fn_vUpdateFrame(void* _p_stSuperObject, tdstFrame3d_* _p_stFrame, tdstAnim3d_* _p_stAnim3d,
                            u16 _uwFrame);
extern void fn_vUpdateFrameMatrix(void* superObject, tdstFrame3d_* cache, tdstAnim3d_* animation, u16 frame, u8 force,
                                  struct tdstAChannel_* bones);
extern tdstEventInTable_* fn_p_GetEventInTable(gu_tdst3dData_* owner, tdstEvent_* key);

extern void inv_j_fn_v_InitSinTab(void);

extern f32 POS_fn_xGetXScale(POS_tdstMatrix_* matrix);
extern void inv_POS_fn_vZoom(POS_tdstMatrix_* matrix, f32 zoom);
extern void inv_POS_fn_vRemoveScaleFromMatrix(POS_tdstMatrix_* destination,
                                              POS_tdstMatrix_* source);
extern f32 POS_fn_xGetMaxScale(POS_tdstMatrix_* matrix);
extern void POS_fn_vGetScaleMatrix(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* scale);
extern void POS_fn_vQuatToMat(MTH4D_tdstMatrix_* matrix, f32 x, f32 y, f32 z, f32 w);
extern void POS_fn_vSetScaleMatrix(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* scale);

extern void GAM_fn_vReadGameDescriptionBinaryFile(void);
extern void inv_SNA_fn_vDelocateMemoryBlocks(void);
extern void inv_fn_vLoadLevelDataFiles(const char* levelName, const char* variant1,
                                       const char* variant2);

typedef struct stTfmtStatus_ {
    f32 xScreenWidth;
    f32 xBaseCharX;
    f32 xBaseCharZ;
    f32 xBaseToAddX;
    f32 xRandomPosRadius;
    f32 xCharWidth;
    f32 xLineHeight;
    f32 xCharZoom;
    f32 xCurCharZoom;
    f32 xCurCharWidth;
    tdstObjectsTablesList_* hSourceObjectTable;
    u8 cJustificationMode;
} stTfmtStatus_;

typedef struct TXT_tdstTextTables {
    u8 nbLanguages;
    u8 language;
    s16 count;
    char*** languageTexts;
    char** highIdTexts;
    char** texts;
} TXT_tdstTextTables;

extern TXT_tdstTextTables inv_TXT_textTableState;

#define TEXT_AFFICHE_COUNT 30

typedef struct stInfoTextAffiche_ {
    s32 bFirstInit;
    u32 ulCurrentDelay;
    u32 ulTimer;
    u32 ulEvent;
    s16 wCurrentLetterInString;
} stInfoTextAffiche_;

extern HIE_tdstSuperObject_* gp_SuperObjPersoGenerated[TEXT_AFFICHE_COUNT];
extern stInfoTextAffiche_ g_stInfoTextAffiche[TEXT_AFFICHE_COUNT];

extern s32 TFMT_wPositionModulesAccordingToText(stTfmtStatus_* status, HIE_tdstSuperObject_* text, u8* string, s32 start,
                                                s32 length, u8* mode, s16* index, s32* state);
extern void TFMT_vInitTextAfficheStructure(void);
extern void TFMT_vSetDefaultValuesCharacter(f32 _xCharWidth, f32 _xLineHeight, f32 _xCharZoom);
extern u8 TFMT_ucSetNumber4CurrentTextAffiche(void);
extern void TFMT_vUpdateScaleTable(char* szText);
extern void TFMT_vUpdateSpacingTable(char* szText);
extern u8 TFMT_ucNumberOfCurrentTextAffiche(HIE_tdstSuperObject_* key);
extern void TFMT_ucClearNumberOfCurrentTextAffiche(HIE_tdstSuperObject_* key);
extern s32 TFMT_wPositionModulesAccordingToText2(u8* string, s32 _lStart, s32 _lEnd, u8* bFirstTimeZ, s16* pwCurrentChar, u32* ulDelay);

extern char* fn_p_GetTextById(s32 id);
extern void inv_TXT_fn_vSetLanguage(u8 language);

extern s32 CAM_fn_bSuperObjectIsACamera(HIE_tdstSuperObject_* _h_SprObj);
extern void CAM_fn_vCameraManagement(void);
extern void CAM_fn_vInitCameras(void);
extern void CAM_fn_vReInitCameras(void);
extern void CAM_fn_vAssignCameraToAViewport(s32 _lViewportNumber, HIE_tdstSuperObject_* _hCamera);

extern void fn_vTurnMatrixX(POS_tdstMatrix_* matrix, f32 angle);
extern void fn_vTurnMatrixY(POS_tdstMatrix_* matrix, f32 angle);
extern void fn_vTurnMatrixZ(POS_tdstMatrix_* matrix, f32 angle);

struct tdstCsaList_;
struct COL_tdstCollideObject_;
extern void fn_vThisTypeVsThisType(HIE_tdstSuperObject_* _hCharacter, u16 _wCharacterMask,
                                   u16 _wOthersMask, HIE_tdstSuperObject_** _p_hObjectTable,
                                   u16* _p_uwNumber);
extern void fn_vGetAllCharactersInContactWithThisZde(HIE_tdstSuperObject_* _hSuperObj,
                                                     s32 ucChannelNumber,
                                                     HIE_tdstSuperObject_** _hSuperObjTable,
                                                     u16* _p_uwNumber, u16 _wMaterialMask);
extern void fn_vGetAllObjectsInZdd(HIE_tdstSuperObject_* _hSuperObj, s32 ucChannelNumber,
                                   HIE_tdstSuperObject_** _p_hSuperObjTable, u16* _p_uwNumber);
extern s32 fn_bIsCharacterTWODetectedByCharacterOne(HIE_tdstSuperObject_* _hCharacterONE,
                                                    HIE_tdstSuperObject_* _hCharacterTWO);
extern s32 fn_bIsCharacterTWODetectedByThisZddOfCharacterOne(HIE_tdstSuperObject_* _hCharacterONE,
                                                             s32 ucChannelNumber,
                                                             HIE_tdstSuperObject_* _hCharacterTWO);
extern s32 fn_bThisTypeFromOneVsThisTypeFromTwo(HIE_tdstSuperObject_* _hCharacterONE,
                                                u16 _wMaterialMaskOfONE,
                                                HIE_tdstSuperObject_* _hCharacterTWO,
                                                u16 _wMaterialMaskOfTWO);
extern s32 fn_bThisZdeFromOneVsAllZdeFromTwo(HIE_tdstSuperObject_* _hCharacterONE,
                                             u8 _ucModuleOrCharacter,
                                             HIE_tdstSuperObject_* _hCharacterTWO,
                                             u16 _wMaterialMask);
extern s32 fn_bThisZdeFromOneVsThisZdeFromTwo(HIE_tdstSuperObject_* _hCharacterONE,
                                              u8 _ucModuleOrCharacterForONE,
                                              HIE_tdstSuperObject_* _hCharacterTWO,
                                              u8 _ucModuleOrCharacterForTWO);
extern s32 fn_lGetZdxListNumberOfElements(u8 ucZoneType, HIE_tdstSuperObject_* hSuperObj);
extern void* fn_hIsThisZoneActive(HIE_tdstSuperObject_* _hCharacter, s32 _ucZoneType,
                                  s32 _ucZoneNumber);
extern void fn_vUpdateCurrentActivationForCharacter(HIE_tdstSuperObject_* _hCharacter);
extern struct GMT_tdstGameMaterial_* fn_hIsTypeExistsInGeoObj(struct COL_tdstCollideObject_* _hGeoObj, u16 _wMaterialMask);
extern struct COL_tdstCollideObject_* fn_hMakeOneObjectFromManyZdx(s32 _ucZoneType,
                                                                   HIE_tdstSuperObject_* _hCharacter);
extern struct COL_tdstCollideObject_* fn_hStaticMakeOneObjectFromManyZdx(s32 _ucZoneType,
                                                                         HIE_tdstSuperObject_* _hCharacter);
extern void fn_bHadToCorrectActivationList(struct tdstCsaList_* p_stCsaList, s32 context);

extern void InitArrayOfDynamicFunction(MechanicsCallback* p_fnDynamicFunction);
extern void fn_vMakeCharacterReact(HIE_tdstSuperObject_* p_stSuperObj);
extern void fn_vSuperObjectSetLastCollidedCharacter(HIE_tdstSuperObject_* superObject,
                                                    HIE_tdstSuperObject_* character);
extern void fn_vTransferSpeedInformationPLA_DNM(HIE_tdstSuperObject_* superObject, s16 frame);
extern HIE_tdstSuperObject_* fn_hSuperObjectGetLastCollidedCharacter(HIE_tdstSuperObject_* superObject);
extern void* fn_h_GetCurrentDNMIdCard(HIE_tdstSuperObject_* p_SuperObjPerso);
extern void func_001018B8(void* savedContext, void* vector, void* outParam);

struct PS2_tdstControllerData_;

extern void inv_DEMO_fn_vEndDemoPlayback(void);
extern void inv_DEMO_fn_vStartAutoPlay(void);
extern u32 inv_DEMO_fn_lGetPendingState(void);
extern s32 inv_DEMO_fn_vReadNextDemoRecord(struct PS2_tdstControllerData_* packet, s16* status);

extern void fn_v_DynamicFree(tdstDynam_* h_Dynam);
extern void fn_vDynamInit(tdstEngineObject_* p_stObject, u8 eObjectInit);
extern DNM_stDynamics* fn_p_stDynamGetDynamicsSO(HIE_tdstSuperObject_* hSupObj);

extern s32 MSWay_fn_lGetNumberWayPointInPath(tdstMSWay_* hWay);
extern s32 MSWay_fn_lGetCurrentIndexInPath(tdstMSWay_* hWay);
extern s32 MSWay_fn_lGetFirstIndexInPath(tdstMSWay_* hWay);
extern s32 MSWay_fn_lGetLastIndexInPath(tdstMSWay_* hWay);
extern s32 MSWay_fn_lGetWeightForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex);
extern s32 MSWay_fn_lIncrementCurrentIndexInPath(tdstMSWay_* hWay, s32 iStep);
extern s32 MSWay_fn_lSetCurrentIndexInPath(tdstMSWay_* hWay, s32 _lIndex);
extern s32 MSWay_fn_ulGetCapsForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex);
extern void MSWay_fn_vDesinitPath(tdstMSWay_* hWay);
extern WP_tdstWayPoint_* MSWay_fn_hGetWayPointOfPath(tdstMSWay_* hWay, s32 iIndex);
extern s32 MSWay_fn_lChangeCapsForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex, u32 ulNewCaps,
                                                     s32 lValue);
extern s32 MSWay_fn_lChangeWeightForNextWayPointOfPath(tdstMSWay_* hWay, s32 iIndex, s32 lWeight);
extern s32 MSWay_fn_lLookForWPInMSWay(tdstMSWay_* obj, WP_tdstWayPoint_* _hWayPoint);

extern LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn inv_GAM_hAlwaysActiveCharactersList[];
extern tdstAlways_ g_stAlways;
extern u32 inv_GAM_ulFramesInMode;
extern HIE_tdstSuperObject_* inv_GAM_currentSector;
extern HIE_tdstSuperObject_* inv_GAM_previousSector;
extern u8 inv_DEMO_bPlaying;
extern s32 g_lSoundObjectTypeMusic;
extern s32 g_lSoundVolumeLineMusic;
extern s32 g_lSoundVolumeLineSound;
extern s32 g_lSoundVolumeLineVoices;
extern HIE_tdstSuperObject_* SECT_hFatherSector;
extern HIE_tdstSuperObject_* gp_stDynamicWorld;
extern u32 inv_RW3_ulLevelTextureCount;
extern struct HIE_tdstSuperObject_* gp_stActualWorld;
extern u8 g_bBeginMapSoundEventFlag;
extern void* inv_SNA_p_hLevelGeometricObjects;
extern void* inv_RW3_p_stLevelTextures;
extern u8 gcGlobAleat;

extern tdstObjectsTablesList_* fn_hGetObjectsTableOfFamilyByIndex(tdstFamilyListView* _hFamily,
                                                                  s16 _xIndex);

extern void inv_MOR_fn_vStopAllMorphsInHierarchy(HIE_tdstSuperObject_* superObject);
extern tdstMorphChannelList_* inv_MOR_fn_p_stStartMorph(HIE_tdstSuperObject_* superObject, s16 fromIndex,
                                            s16 toIndex, s32 level, s16 id);
extern void inv_MOR_fn_vUpdateMorph(tdstMorphChannelList_* element);
extern s16 MOR_fn_xStopMorph(HIE_tdstSuperObject_* _hActorSuperObject, s16 _xChannelIndex);
extern void* inv_MOR_fn_p_vGetMorphTarget(tdstMorphChannelList_* owner);

extern void fn_vSetHitPointsMaxToInitValue(tdstEngineObject_* _p_stObject);
extern void fn_vSetHitPointsToInitValue(tdstEngineObject_* _p_stObject);
extern u32 fn_ulStandardGameGetCustomBitsSO(HIE_tdstSuperObject_* hSupObj);

extern void fn_vAddCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities);
extern void fn_vSetCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities);
extern void fn_vSubCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities);
extern u32 fn_ulGetCapabilities(tdstEngineObject_* _p_stObject);

struct GeometricObjectView;
extern void ANI_fn_vFreeze(HIE_tdstSuperObject_* hSuperObjPerso);
extern void ANI_fn_vUnFreeze(HIE_tdstSuperObject_* superObject);
extern void inv_GEO_fn_vGetElementVisualMaterial(struct GeometricObjectView* hObject, s16 hElement, struct GliMaterialData** p_hVisualMaterial);

extern void fn_vDesInitAllEvents(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* hNextState,
                                 s32 bStopEvents);
extern void fn_vInitAllEvents(HIE_tdstSuperObject_* p_stSuperObject, u32 uwFirstFrame);

extern s32 inv_VID_bMovieJustEnded;
extern u32 inv_GAM_textIdBase;

extern ChannelOps off_2523D0[];

extern struct SaveGameData inv_SAV_stGameData;
extern struct SaveSlot* inv_SAV_p_stDirectorySlots;

extern MechanicsCallback g_a_pfnDynamicFunction[2];
extern s16 ax_SinTab[];
extern u32 inv_SND_ulLevelBank;

#endif
