#include "StdInc.h"

#include "StuntJumpManager.h"

static constexpr uint16 STUNT_JUMP_COUNT = 256;

void CStuntJumpManager::InjectHooks() {
    RH_ScopedClass(CStuntJumpManager);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Init, 0x49CA50);
    RH_ScopedInstall(Shutdown, 0x49CBC0);
    RH_ScopedInstall(ShutdownForRestart, 0x49CB10);
    RH_ScopedInstall(Save, 0x5D5570);
    RH_ScopedInstall(Load, 0x5D5920);
    RH_ScopedInstall(AddOne, 0x49CB40);
    RH_ScopedInstall(Update, 0x49C490);
}

// 0x49CA50
void CStuntJumpManager::Init() {
    mp_poolStuntJumps = new CStuntJumpsPool(STUNT_JUMP_COUNT, "Stunt Jumps");
    m_bActive = true;
}

// 0x49CBC0
void CStuntJumpManager::Shutdown() {
    if (mp_poolStuntJumps)
        mp_poolStuntJumps->Flush();

    mp_poolStuntJumps = nullptr;
}

// 0x49CB10
void CStuntJumpManager::ShutdownForRestart() {
    mp_poolStuntJumps->Clear(); // 0x49C9A0
    mp_Active = nullptr;
    m_bHitReward = false;
    m_iTimer = 0;
    m_jumpState = eJumpState::START_POINT_INTERSECTED;
    m_iNumJumps = 0;
    m_iNumCompleted = 0;
}

// 0x5D5570
bool CStuntJumpManager::Save() {
    CGenericGameStorage::SaveDataToWorkBuffer(&m_iNumJumps, sizeof(m_iNumJumps));
    for (auto i = 0; i < STUNT_JUMP_COUNT; i++) {
        CStuntJump* jump = mp_poolStuntJumps->GetAt(i);
        if (jump) {
            CGenericGameStorage::SaveDataToWorkBuffer(jump, sizeof(CStuntJump));
        }
    }
    return true;
}

// 0x5D5920
bool CStuntJumpManager::Load() {
    uint32 jumpCount;
    CGenericGameStorage::LoadDataFromWorkBuffer(&jumpCount, sizeof(jumpCount));
    for (uint32 i = 0; i < jumpCount; i++) {
        CStuntJump* jump = mp_poolStuntJumps->New();
        CGenericGameStorage::LoadDataFromWorkBuffer(jump, sizeof(CStuntJump));
        m_iNumJumps++;
    }
    return true;
}

// 0x49CB40
void CStuntJumpManager::AddOne(const CBoundingBox& start, const CBoundingBox& end, const CVector& cameraPosn, int32 reward) {
    if (mp_poolStuntJumps) {
        CStuntJump* jump = mp_poolStuntJumps->New();
        if (jump) {
            jump->start = start;
            jump->end = end;
            jump->camera = cameraPosn;
            jump->reward = reward;
            jump->done = false;
            jump->found = false;
            ++m_iNumJumps;
        }
    }
}

// 0x49C490
void CStuntJumpManager::Update() {
    if (!mp_poolStuntJumps || CReplay::Mode == REPLAY_MODE_1)
        return;

    CPlayerPed* playerPed = FindPlayerPed();
    CVehicle* playerVehicle = playerPed->m_pVehicle;
    CPlayerInfo* playerInfo = playerPed->GetPlayerInfoForThisPlayerPed();

    switch (m_jumpState) {
    case eJumpState::START_POINT_INTERSECTED: {
        if (
            m_bActive &&
            playerInfo->m_nPlayerState == PLAYERSTATE_PLAYING &&
            playerPed->bInVehicle &&
            playerVehicle &&
            playerVehicle->m_pDriver == playerPed &&
            playerVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_BOAT &&
            playerVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_PLANE &&
            playerVehicle->GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI &&
            playerVehicle->m_nNumEntitiesCollided != 0 &&
            playerVehicle->m_vecMoveSpeed.Magnitude() * 50.0f >= 20.0f
        ) {
            for (auto jumpIndex = 0; jumpIndex < STUNT_JUMP_COUNT; jumpIndex++) {
                CStuntJump* jump = mp_poolStuntJumps->GetAt(jumpIndex);
                if (!jump)
                    continue;

                CVector& point = playerVehicle->GetPosition();
                if (!jump->start.IsPointWithin(point))
                    continue;

                m_jumpState = eJumpState::IN_FLIGHT;
                mp_Active = jump;
                m_iTimer = 0;
                m_bHitReward = false;

                if (!mp_Active->found) {
                    mp_Active->found = true;
                    CStats::IncrementStat(STAT_UNIQUE_JUMPS_FOUND, 1.0f);
                }

                CTimer::SetTimeScale(0.3f);
                CVector rotation{0.0f, 0.0f, 0.0f};
                TheCamera.SetCamPositionForFixedMode(&mp_Active->camera, &rotation);
                TheCamera.TakeControl(playerVehicle, MODE_FIXED, SWITCHTYPE_JUMPCUT, 1);
            }
        }
        break;
    }
    case eJumpState::IN_FLIGHT: {
        if (!mp_Active) {
            m_jumpState = eJumpState::START_POINT_INTERSECTED;
            break;
        }

        bool bFailed = false;
        if (playerVehicle->m_nNumEntitiesCollided != 0 && m_iTimer >= 100) {
            bFailed = true;
        }

        if (playerInfo->m_nPlayerState == PLAYERSTATE_HAS_DIED) {
            bFailed = true;
        }

        if (!playerPed->bInVehicle) {
            bFailed = true;
        }

        if (playerVehicle->m_nStatus == STATUS_WRECKED || playerVehicle->vehicleFlags.bIsDrowning || playerVehicle->physicalFlags.bSubmergedInWater) {
            bFailed = true;
        }

        CVector& point = playerVehicle->GetPosition();
        if (mp_Active->end.IsPointWithin(point))
            m_bHitReward = true;

        uint32 time;
        if (bFailed) {
            m_jumpState = eJumpState::END_POINT_INTERSECTED;
            time = 0;
        } else {
            time = m_iTimer;
        }

        m_iTimer = CTimer::GetTimeStepInMS() + time;
        if (m_iTimer > 1000 && time <= 1000) {
            auto vehicle = FindPlayerVehicle();
            if (vehicle) {
                CPed* randomPassenger = vehicle->PickRandomPassenger();
                if (randomPassenger)
                    randomPassenger->Say(37, 0, 1.0f, 0, 0, 0);
            }
        }

        break;
    }
    case eJumpState::END_POINT_INTERSECTED: {
        m_iTimer += CTimer::GetTimeStepInMS();
        if (m_iTimer < 300)
            return;

        CTimer::ResetTimeScale();
        TheCamera.RestoreWithJumpCut();

        if (!m_bHitReward || mp_Active->done) {
            m_jumpState = eJumpState::START_POINT_INTERSECTED;
            mp_Active = nullptr;
            break;
        }

        mp_Active->done = true;
        ++m_iNumCompleted;

        CStats::IncrementStat(STAT_UNIQUE_JUMPS_DONE, 1.0f);

        int32 reward = m_iNumCompleted == m_iNumJumps ? 10000 : mp_Active->reward;
        playerInfo->m_nMoney += reward;

        AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_PART_MISSION_COMPLETE, 0.0f, 1.0f);

        char* bonusMessage = TheText.Get("USJ"); // UNIQUE STUNT BONUS!
        if (bonusMessage)
            CMessages::AddBigMessageQ(bonusMessage, 5000, STYLE_MIDDLE_SMALLER_HIGHER);

        if (m_iNumCompleted == m_iNumJumps) {
            char* stuntsCompleteMessage = TheText.Get("USJ_ALL"); // ALL UNIQUE STUNTS COMPLETED!
            if (stuntsCompleteMessage)
                CHud::SetHelpMessage(stuntsCompleteMessage, false, false, false);
        }

        char* rewardMessage = TheText.Get("REWARD");
        if (rewardMessage)
            CMessages::AddBigMessageWithNumber(rewardMessage, 6000, STYLE_WHITE_MIDDLE_SMALLER, reward, -1, -1, -1, -1, -1);

        break;
    }
    }
}

// unused
// 0x49C370
void CStuntJumpManager::SetActive(bool active) {
    m_bActive = active;
}

// 0x0
void CStuntJumpManager::Render() {
    // NOP
}

// NOTSA
void ResetAllJumps() {
    for (auto jumpIndex = 0; jumpIndex < STUNT_JUMP_COUNT; jumpIndex++) {
        CStuntJump* jump = CStuntJumpManager::mp_poolStuntJumps->GetAt(jumpIndex);
        if (!jump)
            continue;

        jump->done = false;
        jump->found = false;
    }
}

// NOTSA
void StuntJumpTestCode() {
    CPad* pad = CPad::GetPad(0);
    if (pad->IsStandardKeyJustDown('1')) {
        printf("ResetAllJumps");
        ResetAllJumps();
    }
    if (pad->IsStandardKeyJustDown('2')) {
        printf("");
        auto player = FindPlayerPed();
        if (player) {
            CVector posn{-2053.93848f, 236.598221f, 35.5952835f};
            player->SetPosn(posn);
            CCheat::VehicleCheat(MODEL_NRG500);
        }
    }
}
