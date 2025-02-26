#pragma once

#include <minwinbase.h> // SYSTEMTIME

#include "eWeatherType.h"

class CVector;

class CSimpleVariablesSaveStructure {
public:
    uint32       m_nVersionId;
    char         m_szSaveName[100];
    bool         m_bMissionPackGame;
    char         _pad69[3];
    int32        m_nCurrLevel;
    CVector      m_vecCamPosn;
    uint32       m_nMsPerMinute;
    uint32       m_nLastClockTick;
    uint8        m_nGameMonth;
    uint8        m_nGameDays;
    uint8        m_nGameHours;
    uint8        m_nGameMinutes;
    uint8        m_nGameDayOfWeek;
    uint8        m_nStoredGameMonth;
    uint8        m_nStoredGameMonthDay;
    uint8        m_nStoredGameHours;
    uint8        m_nStoredGameMinutes;
    bool         m_bClockHasBeenStored;
    int16        m_nPadMode;
    bool         m_bHasPlayerCheated;
    char         _pad91[3];
    uint32       m_nTimeInMilliseconds;
    float        m_fTimeScale;
    float        m_fTimeStep;
    float        m_fTimeStepNonClipped;
    uint32       m_nFrameCounter;
    eWeatherType m_nOldWeatherType;
    eWeatherType m_nNewWeatherType;
    eWeatherType m_nForcedWeatherType;
    char         _padAE[2];
    float        m_fWeatherInterpolationValue;
    uint32       m_nWeatherTypeInList;
    float        m_fWeatherRain;
    uint32       m_nCameraCarZoom;
    uint32       m_nCameraPedZoom;
    int32        m_nCurrArea;
    bool         m_bInvertLook4Pad;
    char         _padC9[3];
    int32        m_nExtraColour;
    bool         m_bExtraColourOn;
    char         _padD1[3];
    int32        m_fExtraColourInter;
    int32        m_nExtraColourWeatherType;
    uint32       m_nWaterConfiguration;
    bool         m_bLARiots;
    bool         m_bLARiots_NoPoliceCar;
    char         _padE2[2];
    uint32       m_nMaximumWantedLevel;
    uint32       m_nMaxChaos;
    bool         m_bFrenchGame;
    bool         m_bGermanGame;
    bool         m_bNastyGame;
    char         _padEF[45];
    char         m_bCineyCamMessageDisplayed;
    char         _pad11D[1];
    SYSTEMTIME   m_systemTime;
    char         _pad12E[2];
    int32        m_nTargetBlipIndex;
    bool         m_bHasDisplayedPlayerQuitEnterCarHelpText;
    bool         m_bAllTaxisNitro;
    bool         m_bProstitutesPayYou;
    char         _pad137;

public:
    static void InjectHooks();

    void Construct();
    void Extract(uint32& versionId) const;
};

VALIDATE_SIZE(CSimpleVariablesSaveStructure, 0x138);
