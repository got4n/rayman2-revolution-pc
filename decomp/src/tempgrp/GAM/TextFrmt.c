#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "AGO/AGO.h"

#define TEXT_LETTER_COUNT 256
#define TEXT_PER_MILLE 1000.0f
#define TEXT_JUSTIFY_LEFT 'l'
#define TEXT_JUSTIFY_CENTER 'c'

#define SUPER_OBJECT_OPAQUE 255.0f

typedef struct tdstDefaultValuesForCharacter_ {
    f32 m_xCharWidth;
    f32 m_xLineHeight;
    f32 m_xCharZoom;
} tdstDefaultValuesForCharacter_;

extern tdstDefaultValuesForCharacter_ g_stDefaultValuesForCharacter;
extern f32 TFMT_ga_fLetterSpacing[TEXT_LETTER_COUNT];
extern f32 TFMT_ga_fLetterScale[TEXT_LETTER_COUNT];
extern u32 OffsetObjectTable;
extern s32 TFMT_ga_ulNbScale;
extern s32 g_bFreeTextAfficheBlocs[TEXT_AFFICHE_COUNT];

char* strtok(char* string, const char* delimiters);
f32 __floatdisf(s32 value);
s32 atol(const char* string);
void TFMT_vInitScapingTable(f32 value);
u8* TFMT_pszReadNextChar2(stTfmtStatus_* status, HIE_tdstSuperObject_* text, u8* string,
                          u8* character, u8* first, s32 initialMode, s16* index, s32* result,
                          u32** state, s16* count, u16* advance);
u8* func_00105540(u8* string, u8* character, s32 initialMode, s16* index, s32* result, u32** state,
                  u16* count, u16* advance);

static inline gu_tdst3dData_* inv_GAM_fn_p_stText3dData(HIE_tdstSuperObject_* text)
{
    return ((tdstEngineObject_*) text->hLinkedObject)->h_3dData;
}

static inline s32 inv_GAM_fn_lLetterIndex(s32 character)
{
    if (character < 0x20) {
        return 0;
    }
    if (character >= 'a' && character <= 'z') {
        return character - 0x40;
    }
    if (character >= 'A' && character <= 'Z') {
        return character - 0x20;
    }
    if (character >= 0xA0) {
        return character - 0x40;
    }
    return character - 0x20;
}

void TFMT_vSetDefaultValuesCharacter(f32 _xCharWidth, f32 _xLineHeight, f32 _xCharZoom)
{
    g_stDefaultValuesForCharacter.m_xCharWidth = 0.0f;
    if (g_stDefaultValuesForCharacter.m_xCharWidth <= _xLineHeight) {
        g_stDefaultValuesForCharacter.m_xLineHeight = _xLineHeight;
    }
    if (g_stDefaultValuesForCharacter.m_xCharWidth <= _xCharZoom) {
        g_stDefaultValuesForCharacter.m_xCharZoom = _xCharZoom;
    }
    TFMT_vInitScapingTable(_xCharWidth);
}

void TFMT_vInitFormatterStatus(stTfmtStatus_* status, HIE_tdstSuperObject_* text, f32 lineStart)
{
    f32 zero = 0.0f;
    void* font;

    OffsetObjectTable = 0;
    status->xBaseCharZ = zero;
    status->xBaseCharX = zero;
    status->xBaseToAddX = zero;
    status->xScreenWidth = lineStart;
    status->xCharWidth = g_stDefaultValuesForCharacter.m_xCharWidth;
    status->xLineHeight = g_stDefaultValuesForCharacter.m_xLineHeight;
    status->xCharZoom = g_stDefaultValuesForCharacter.m_xCharZoom;
    status->xCurCharZoom = zero;
    status->cJustificationMode = TEXT_JUSTIFY_LEFT;
    font = fn_hGetObjectsTableOfFamilyByIndex(inv_GAM_fn_p_stText3dData(text)->h_Family, 0);
    status->xRandomPosRadius = zero;
    status->hSourceObjectTable = font;
}

u8* TFMT_pszReadNextChar(stTfmtStatus_* _p_stStatus, HIE_tdstSuperObject_* _hSuperObjPerso, u8* _pszCurrentChar,
                         u8* _p_ucReturnedChar, u8* _p_bLineBreak)
{
    s32 c;
    s32 value;
    f32 x;
    f32 zoom;
    tdstFamilyListView* fonts;

    *_p_bLineBreak = 0;
    if (_pszCurrentChar != NULL && (c = *_pszCurrentChar++) != 0) {
        if (c == '/') {
            do {
                switch (*_pszCurrentChar) {
                case 'L':
                case 'l':
                    *_p_bLineBreak = 1;
                    _p_stStatus->xBaseCharX = _p_stStatus->xBaseToAddX = 0.0f;
                    _p_stStatus->xBaseCharZ = _p_stStatus->xBaseCharZ - _p_stStatus->xLineHeight;
                    break;
                case 'Z':
                case 'z':
                    value = inv_atoi(++_pszCurrentChar);
                    _p_stStatus->xCharZoom = (f32) value / TEXT_PER_MILLE;
                    break;
                case 'W':
                case 'w':
                    value = inv_atoi(++_pszCurrentChar);
                    _p_stStatus->xCharWidth = _p_stStatus->xCharZoom * ((f32) value / TEXT_PER_MILLE);
                    break;
                case 'H':
                case 'h':
                    value = inv_atoi(++_pszCurrentChar);
                    _p_stStatus->xLineHeight = _p_stStatus->xCharZoom * ((f32) value / TEXT_PER_MILLE);
                    break;
                case 'C':
                case 'c':
                    _pszCurrentChar++;
                    fonts = inv_GAM_fn_p_stText3dData(_hSuperObjPerso)->h_Family;
                    _p_stStatus->hSourceObjectTable =
                        fn_hGetObjectsTableOfFamilyByIndex(fonts, inv_atoi(_pszCurrentChar));
                    break;
                case 'O':
                case 'o':
                    _pszCurrentChar++;
                    OffsetObjectTable = inv_atoi(_pszCurrentChar);
                    break;
                case 'J':
                case 'j':
                    _pszCurrentChar++;
                    _p_stStatus->cJustificationMode = *_pszCurrentChar;
                    break;
                case 'R':
                case 'r':
                    value = inv_atoi(++_pszCurrentChar);
                    _p_stStatus->xRandomPosRadius = (f32) value / TEXT_PER_MILLE;
                    break;
                case '/':
                case ':':
                    _pszCurrentChar++;
                    break;
                }
                do {
                    _pszCurrentChar++;
                } while (*_pszCurrentChar != ':');
                _pszCurrentChar++;
                c = *_pszCurrentChar;
                if (*_p_bLineBreak == 0) {
                    _pszCurrentChar++;
                }
            } while (c == '/');
        }
        x = _p_stStatus->xBaseCharX;
        if (x == 0.0f && c == ' ') {
            do {
                c = *_pszCurrentChar++;
            } while (c == ' ');
        }
        _p_stStatus->xBaseCharX = x + _p_stStatus->xBaseToAddX;
        if (*_p_bLineBreak == 0) {
            if (_p_stStatus->xCharWidth != 0.0f) {
                _p_stStatus->xBaseToAddX = _p_stStatus->xCharWidth;
            } else {
                zoom = _p_stStatus->xCharZoom;
                if (zoom == 0.0f) {
                    zoom = TFMT_ga_ulNbScale != 0 ? TFMT_ga_fLetterScale[0] : 1.0f;
                }
                _p_stStatus->xBaseToAddX = zoom * TFMT_ga_fLetterSpacing[(u8) inv_GAM_fn_lLetterIndex(c)];
            }
        }
    } else {
        *_p_bLineBreak = 1;
        *_p_ucReturnedChar = 0;
        return NULL;
    }
    *_p_ucReturnedChar = c;
    return _pszCurrentChar;
}

s32 TFMT_lProcessFormater(stTfmtStatus_* p_stStatus, u8* szString, u8* p_bLineBreak, u8* p_bStartLine)
{
    s32 length = 1;
    u8 code = *szString++;

    switch (code) {
    case 'C':
    case 'c':
        p_stStatus->cJustificationMode = TEXT_JUSTIFY_CENTER;
        break;
    case 'H':
    case 'h':
        p_stStatus->xLineHeight = (f32) inv_atoi(szString) / TEXT_PER_MILLE;
        break;
    case 'L':
    case 'l':
        *p_bLineBreak = 1;
        *p_bStartLine = 1;
        p_stStatus->xBaseCharX = p_stStatus->xBaseToAddX = 0.0f;
        p_stStatus->xBaseCharZ = p_stStatus->xBaseCharZ - p_stStatus->xLineHeight;
        break;
    case 'W':
    case 'w':
        p_stStatus->xCharWidth = (f32) inv_atoi(szString) / TEXT_PER_MILLE;
        break;
    case 'Z':
    case 'z':
        p_stStatus->xCharZoom = (f32) inv_atoi(szString) / TEXT_PER_MILLE;
        break;
    case 'O':
    case 'o':
        OffsetObjectTable = inv_atoi(szString);
        break;
    }
    while (*szString != ':') {
        szString++;
        length++;
    }
    return length + 1;
}

u8* TFMT_pszReadNextChar2(stTfmtStatus_* _p_stStatus, HIE_tdstSuperObject_* text, u8* _pszCurrentChar,
                          u8* _p_ucReturnedChar, u8* _p_bLineBreak, s32 bDisplayFullString, s16* wCurrentChar, s32* ulEvent,
                          u32** ulDelay, s16* wCurrentChar4Channels, u16* p_uwIndexOfChar)
{
    u8 ucTemp;
    s32 i;
    u8 bStartLine;
    f32 xZoom;
    f32 xWidth;
    f32 xPrevWidth;
    f32 xPrevZoom;
    s32 lNbChar;
    u16 uwLenString;
    u8 ucChar;

    xWidth = 0.0f;
    xPrevWidth = 0.0f;
    xZoom = 0.0f;
    xPrevZoom = 0.0f;
    *_p_bLineBreak = 0;
    lNbChar = 0;

    if (!bDisplayFullString) {
        bStartLine = 1;
        i = 0;
        uwLenString = inv_strlen(_pszCurrentChar);
        ucChar = _pszCurrentChar[*wCurrentChar];
        if (*ulEvent) {
            if (ucChar == '/') {
                if ((u8) (_pszCurrentChar[*wCurrentChar] + 1) == 'E' || (u8) (_pszCurrentChar[*wCurrentChar] + 1) == 'e') {
                    (*wCurrentChar)++;
                    (*wCurrentChar)++;
                    *ulEvent = inv_atoi(_pszCurrentChar + *wCurrentChar);
                    while (_pszCurrentChar[(*wCurrentChar)++] != ':')
                        ;
                } else {
                    *ulEvent = 0;
                }
            } else {
                *ulEvent = 0;
            }
            *_p_ucReturnedChar = 0;
            return _pszCurrentChar + uwLenString;
        }
        {
            while (ucChar == '/') {
                (*wCurrentChar)++;
                ucChar = _pszCurrentChar[*wCurrentChar];
                if (ucChar == 'E' || ucChar == 'e') {
                    (*wCurrentChar)++;
                    *ulEvent = inv_atoi(_pszCurrentChar + *wCurrentChar);
                    while (_pszCurrentChar[(*wCurrentChar)++] != ':')
                        ;
                    *_p_ucReturnedChar = 0;
                    return _pszCurrentChar + uwLenString;
                }
                if (ucChar == 'D' || ucChar == 'd' || ucChar == 'P' || ucChar == 'p') {
                    (*wCurrentChar)++;
                    **ulDelay = inv_atoi(_pszCurrentChar + *wCurrentChar);
                    if (ucChar == 'P' || ucChar == 'p')
                        **ulDelay = -**ulDelay;
                    while (_pszCurrentChar[(*wCurrentChar)++] != ':')
                        ;
                    *_p_ucReturnedChar = 0;
                    return _pszCurrentChar + uwLenString;
                }
                while (_pszCurrentChar[(*wCurrentChar)++] != ':')
                    ;
                ucChar = _pszCurrentChar[*wCurrentChar];
            }
        }
        *wCurrentChar4Channels = -1;
        if (*wCurrentChar >= 0) {
            while (*wCurrentChar >= i) {
                ucChar = _pszCurrentChar[i];
                if (ucChar == '/') {
                    i += TFMT_lProcessFormater(_p_stStatus, _pszCurrentChar + i + 1, _p_bLineBreak, &bStartLine);
                } else {
                    xZoom = _p_stStatus->xCharZoom;
                    if (xZoom == 0.0f) {
                        if (TFMT_ga_ulNbScale != 0)
                            xZoom = TFMT_ga_fLetterScale[lNbChar % (u32) TFMT_ga_ulNbScale];
                        else
                            xZoom = 1.0f;
                    }
                    _p_stStatus->xCurCharZoom = xZoom;
                    if (_p_stStatus->xCharWidth != 0.0f) {
                        xWidth = _p_stStatus->xCharWidth;
                    } else {
                        ucTemp = (u8) inv_GAM_fn_lLetterIndex(ucChar);
                        xWidth = TFMT_ga_fLetterSpacing[ucTemp];
                    }
                    (*wCurrentChar4Channels)++;
                    if (!bStartLine)
                        _p_stStatus->xBaseToAddX += xPrevZoom * xPrevWidth;
                    else
                        bStartLine = 0;
                    lNbChar++;
                    xPrevZoom = xZoom;
                    xPrevWidth = xWidth;
                }
                i++;
            }
        }
        _p_stStatus->xCurCharWidth = xZoom * xWidth;
        _p_stStatus->xBaseCharX += _p_stStatus->xBaseToAddX;
    } else {
        bStartLine = 0;
        uwLenString = inv_strlen(_pszCurrentChar);
        lNbChar = *p_uwIndexOfChar;
        while (*_pszCurrentChar++ == '/')
            _pszCurrentChar += TFMT_lProcessFormater(_p_stStatus, _pszCurrentChar, _p_bLineBreak, &bStartLine);
        ucChar = _pszCurrentChar[-1];
        xZoom = _p_stStatus->xCharZoom;
        if (xZoom == 0.0f) {
            if (TFMT_ga_ulNbScale != 0)
                xZoom = TFMT_ga_fLetterScale[lNbChar % (u32) TFMT_ga_ulNbScale];
            else
                xZoom = 1.0f;
        }
        _p_stStatus->xCurCharZoom = xZoom;
        if (!bStartLine)
            _p_stStatus->xBaseCharX += _p_stStatus->xBaseToAddX;
        else
            bStartLine = 0;
        if (_p_stStatus->xCharWidth != 0.0f) {
            _p_stStatus->xBaseToAddX = xZoom * _p_stStatus->xCharWidth;
        } else {
            _p_stStatus->xBaseToAddX = xZoom * TFMT_ga_fLetterSpacing[(u8) inv_GAM_fn_lLetterIndex(ucChar)];
        }
        _p_stStatus->xCurCharWidth = _p_stStatus->xBaseToAddX;
        lNbChar++;
    }
    *_p_ucReturnedChar = ucChar;
    *p_uwIndexOfChar = lNbChar;
    return bDisplayFullString ? _pszCurrentChar : _pszCurrentChar + uwLenString;
}

u8* func_00105540(u8* string, u8* character, s32 initialMode, s16* index, s32* result, u32** state,
                  u16* count, u16* advance)
{
    u16 length;
    s32 advanced;
    u8 current;

    advanced = 0;
    if (initialMode == 0) {
        do {
        } while (0);
        length = inv_strlen(string);
        current = string[*index];

        if (*result != 0) {
            if (current == '/') {
                *result = 0;
            } else {
                *result = 0;
            }
            *character = 0;
            return string + length;
        }

        while (current == '/') {
            (*index)++;
            current = string[*index];
            if (current == 'E' || current == 'e') {
                (*index)++;
                *result = inv_atoi(string + *index);
                while (string[(*index)++] != ':') {
                }
                *character = 0;
                return string + length;
            }

            if (current == 'D' || current == 'd' || current == 'P' || current == 'p') {
                (*index)++;
                **state = inv_atoi(string + *index);
                if (current == 'P' || current == 'p') {
                    **state = -**state;
                }
                while (string[(*index)++] != ':') {
                }
                *character = 0;
                return string + length;
            }

            while (string[(*index)++] != ':') {
            }
            current = string[*index];
        }
    } else {
        length = inv_strlen(string);
        advanced = *advance;
        current = string[-1];
        advanced++;
    }
    *character = current;
    *advance = advanced;
    return string + length;
}

void fn_vImplementFirstTimeChannelCreation(gu_tdst3dData_* data, s32 count)
{
    tdstAChannel_* list[4];
    u16 recordCount = count;

    data->ulNumberOfChannels = recordCount;
    fn_vInitCSOList(list, recordCount);
    data->hArrayOfChannels = list[0];
    data->hFirstActiveChannel = list[0];
}

void fn_vImplementChannelUpdate(HIE_tdstSuperObject_* _hSuperObjPerso, s16 _wCurrentChar4Channels, s16 _wSourceObject, f32 _xBaseCharX, f32 _xBaseCharZ, f32 _xCurCharWidth,
                        f32 _xRandomPosRadius, f32 _xCurCharZoom)
{
    gu_tdst3dData_* data = inv_GAM_fn_p_stText3dData(_hSuperObjPerso);
    tdstAChannel_* record = data->hArrayOfChannels;
    ObjectTableEntry* entries = data->h_CurrentObjectsTable->d_stObjectsTable;
    ObjectTableEntry* entry;
    HIE_tdstSuperObject_* letter = PLA_fn_hFindNextFreeSupObj();
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_ vector;
    f32 low;

    HIE_fn_vSuperObjectAddTail(_hSuperObjPerso, letter);

    record += _wCurrentChar4Channels;
    record->bControlledChannel = 1;
    gu_MTH4D_M_vSetIdentityMatrix(letter->gu_hLocalMatrix);
    record->hSupObject = letter;
    entry = &entries[_wSourceObject];
    letter->lTypeOfLinkedObject =
        entry->stElement.h_Target == 0 ? HIE_TYPE_EMPTY_inv : HIE_TYPE_PHYSICAL_OBJECT_inv;
    letter->hLinkedObject = (void*) entry->stElement.h_Target;
    letter->gu_fTransparenceLevel = SUPER_OBJECT_OPAQUE;
    if (_wCurrentChar4Channels != 0) {
        record[-1].p_stNextActiveChannel = record;
    }

    matrix = letter->gu_hLocalMatrix;
    low = inv_MTH_fn_xNegate(_xRandomPosRadius);
    _xRandomPosRadius = _xRandomPosRadius - low;
    MTH3D_M_vSetXYZ_inv(&vector,
                        _xBaseCharX + ((f32) rand() * _xRandomPosRadius / RAND_RANGE_inv + low) +
                            _xCurCharWidth * 0.5f,
                        0.0f,
                        _xBaseCharZ + ((f32) rand() * _xRandomPosRadius / RAND_RANGE_inv + low));
    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    *inv_POS_fn_p_stPosMatrixTranslation(matrix) = vector;
    matrix->stMatrix.stCol_3.xW = 1.0f;
    if (__builtin_fabsf(_xCurCharZoom - 1.0f) > 0.0001f) {
        MTH3D_M_vSetXYZ_inv(&vector, _xCurCharZoom, _xCurCharZoom, _xCurCharZoom);
        POS_fn_vSetScaleMatrix(matrix, &vector);
    }
}

s32 TFMT_wPositionModulesAccordingToText(stTfmtStatus_* status, HIE_tdstSuperObject_* text,
                                         u8* string, s32 start, s32 length, u8* mode, s16* index,
                                         s32* state)
{
    u8 character;
    u8 first;
    s16 currentIndex = 0;
    s16 count = 0;
    u16 advance;
    s32 result = 0;
    u32* currentState = state;
    gu_tdst3dData_* data;
    s32 textLength = 0;
    s32 initialMode = 0;
    u8* end;
    s16 elementCount;
    s32 element;
    s32 letter;
    s32 original;
    s32 inRange;
    s32 running;

    if (length < 0) {
        end = string + inv_strlen(string);
    } else {
        end = string + length;
    }
    if (*mode == 1) {
        textLength = (u16) inv_strlen(string);
    }
    currentIndex = *index;
    if (currentIndex == -1) {
        currentIndex = 0;
        initialMode = 1;
    }
    advance = start;
    TFMT_vInitFormatterStatus(status, text, 0.0f);
    string += start;
    data = inv_GAM_fn_p_stText3dData(text);
    inRange = string < end;
    running = inRange == 1;
    while (running && *string != 0) {
        if (running) {
            do {
                string =
                    TFMT_pszReadNextChar2(status, text, string, &character, &first, initialMode,
                                          &currentIndex, &result, &currentState, &count, &advance);
                elementCount = fn_uwGetObjectTableNumberOfelements(status->hSourceObjectTable);
                if (character < 0x20) {
                    element = -1;
                } else {
                    letter = character;
                    original = letter;
                    if (letter < 0x20) {
                        letter = 0;
                    } else if (letter >= 'a' && letter <= 'z') {
                        letter -= 0x40;
                    } else if (letter >= 'A' && letter <= 'Z') {
                        letter -= 0x20;
                    } else {
                        letter = original < 0xA0 ? letter - 0x20 : original - 0x40;
                    }
                    element = (s16) (OffsetObjectTable + (u8) letter);
                }
                if (element < 0 || element >= elementCount) {
                    *index = currentIndex;
                    return result;
                }
                if (*mode == 1) {
                    fn_vImplementFirstTimeChannelCreation(data, textLength);
                    *mode = 0;
                }
                fn_vImplementChannelUpdate(text, count, element, status->xBaseCharX, status->xBaseCharZ,
                                   status->xCurCharWidth, status->xRandomPosRadius,
                                   status->xCurCharZoom);
                inRange = string < end;
                count++;
                currentIndex++;
                *index = currentIndex;
            } while (inRange);
        }
        running = inRange;
    }
    return result;
}

s32 TFMT_wPositionModulesAccordingToText2(u8* string, s32 _lStart, s32 _lEnd, u8* bFirstTimeZ, s16* pwCurrentChar,
                                          u32* ulDelay)
{
    u8 character;
    s16 currentIndex = 0;
    u16 count = 0;
    u16 advance;
    s32 result = 0;
    u32* currentState = ulDelay;
    s32 initialMode = 0;
    u8* end;
    s32 element;
    s32 letter;
    s32 original;

    if (_lEnd < 0) {
        end = string + inv_strlen(string);
    } else {
        end = string + _lEnd;
    }
    currentIndex = *pwCurrentChar;
    if (currentIndex == -1) {
        currentIndex = 0;
        initialMode = 1;
    }
    string += _lStart;
    advance = _lStart;
    while (string < end && *string != 0) {
        while (string < end) {
            string = func_00105540(string, &character, initialMode, &currentIndex, &result,
                                   &currentState, &count, &advance);
            if (character < 0x20) {
                element = -1;
            } else {
                letter = character;
                original = letter;
                if (letter < 0x20) {
                    letter = 0;
                } else if (letter >= 'a' && letter <= 'z') {
                    letter -= 0x40;
                } else if (letter >= 'A' && letter <= 'Z') {
                    letter -= 0x20;
                } else {
                    letter = original < 0xA0 ? letter - 0x20 : original - 0x40;
                }
                element = (s16) (OffsetObjectTable + (u8) letter);
            }
            if (element < 0) {
                *pwCurrentChar = currentIndex;
                return result;
            }
            count++;
            currentIndex++;
            *pwCurrentChar = currentIndex;
        }
    }
    return result;
}

void TFMT_vInitTextAfficheStructure(void)
{
    s32 iCpt;

    for (iCpt = 0; iCpt < TEXT_AFFICHE_COUNT; iCpt++) {
        memset(&g_stInfoTextAffiche[iCpt], 0, sizeof(stInfoTextAffiche_));
        g_stInfoTextAffiche[iCpt].bFirstInit = 1;
        g_bFreeTextAfficheBlocs[iCpt] = 1;
        gp_SuperObjPersoGenerated[iCpt] = 0;
    }
}

u8 TFMT_ucNumberOfCurrentTextAffiche(HIE_tdstSuperObject_* key)
{
    u8 i = 0;

    do {
        if (key == gp_SuperObjPersoGenerated[i]) {
            return i;
        }
        i++;
    } while (i < TEXT_AFFICHE_COUNT);
    return i;
}

u8 TFMT_ucSetNumber4CurrentTextAffiche(void)
{
    u8 i = 0;

    do {
        if (g_bFreeTextAfficheBlocs[i] != 0) {
            g_bFreeTextAfficheBlocs[i] = 0;
            return i;
        }
        i++;
    } while (i < TEXT_AFFICHE_COUNT);
    return i;
}

void TFMT_ucClearNumberOfCurrentTextAffiche(HIE_tdstSuperObject_* key)
{
    u8 i = 0;

    do {
        if (key == gp_SuperObjPersoGenerated[i]) {
            g_bFreeTextAfficheBlocs[i] = 1;
            gp_SuperObjPersoGenerated[i] = 0;
            memset(&g_stInfoTextAffiche[i], 0, sizeof(stInfoTextAffiche_));
            g_stInfoTextAffiche[i].bFirstInit = 1;
            return;
        }
        i++;
    } while (i < TEXT_AFFICHE_COUNT);
}

void TFMT_vInitScapingTable(f32 value)
{
    s32 i;

    for (i = TEXT_LETTER_COUNT - 1; i >= 0; i--) {
        TFMT_ga_fLetterSpacing[i] = value;
    }
}

void TFMT_vUpdateSpacingTable(char* szText)
{
    f32* const table = TFMT_ga_fLetterSpacing;
    char* szCur = strtok(szText, ":");
    s32 i = 1;

    table[0] = __floatdisf(atol(szCur)) / TEXT_PER_MILLE;
    if (szCur != NULL) {
        do {
            szCur = strtok(NULL, ":");
            if (szCur == NULL) {
                break;
            }
            table[i] = __floatdisf(atol(szCur)) / TEXT_PER_MILLE;
            i++;
            szCur[-1] = ':';
        } while (i < TEXT_LETTER_COUNT);
    }
}

void TFMT_vUpdateScaleTable(char* szText)
{
    f32* const table = TFMT_ga_fLetterScale;
    char* szCur = strtok(szText, ":");
    s32 i = 1;

    table[0] = __floatdisf(atol(szCur)) / TEXT_PER_MILLE;
    if (szCur != NULL) {
        do {
            szCur = strtok(NULL, ":");
            if (szCur == NULL) {
                break;
            }
            table[i] = __floatdisf(atol(szCur)) / TEXT_PER_MILLE;
            i++;
            szCur[-1] = ':';
        } while (i < TEXT_LETTER_COUNT);
    }
    TFMT_ga_ulNbScale = i;
}
