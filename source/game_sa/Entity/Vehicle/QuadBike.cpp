#include "StdInc.h"

#include "QuadBike.h"

bool& bDoQuadDamping = *(bool*)0x8D3450; // true 0x8D3450
float& QUAD_HBSTEER_ANIM_MULT = *(float*)0x8D3454; // -0.4f 0x8D3454
CVector& vecQuadResistance = *(CVector*)0x8D3458; // { 0.995f, 0.995f, 1.0f } // 0x8D3458

void CQuadBike::InjectHooks() {
    RH_ScopedClass(CQuadBike);
    RH_ScopedCategory("Vehicle");

// todo: RH_ScopedInstall(Constructor, 0x6CE370);
    RH_ScopedInstall(Fix_Reversed, 0x6CE2B0);
    RH_ScopedInstall(GetRideAnimData_Reversed, 0x6CDC90);
    RH_ScopedInstall(PreRender_Reversed, 0x6CEAD0);
    RH_ScopedInstall(ProcessAI_Reversed, 0x6CE460);
    RH_ScopedInstall(ProcessControl_Reversed, 0x6CDCC0);
    RH_ScopedInstall(ProcessControlInputs_Reversed, 0x6CE020);
    RH_ScopedInstall(ProcessDrivingAnims_Reversed, 0x6CE280);
    RH_ScopedInstall(ProcessSuspension_Reversed, 0x6CE270);
    RH_ScopedInstall(ResetSuspension_Reversed, 0x6CDCB0);
    RH_ScopedInstall(SetupDamageAfterLoad_Reversed, 0x6CE340);
    RH_ScopedInstall(SetupSuspensionLines_Reversed, 0x6CDCA0);
}

// 0x6CE370
CQuadBike::CQuadBike(int32 modelIndex, eVehicleCreatedBy createdBy) : CAutomobile(modelIndex, createdBy, false)
{
    plugin::CallMethod<0x6CE370, CAutomobile*, int32, eVehicleCreatedBy>(this, modelIndex, createdBy);
    return;

    m_sRideAnimData.m_nAnimGroup = ANIM_GROUP_QUAD;
    m_pHandling = gHandlingDataMgr.GetBikeHandlingPointer(CModelInfo::GetModelInfo(modelIndex)->AsVehicleModelInfoPtr()->m_nHandlingId);
    m_nVehicleSubType = VEHICLE_TYPE_QUAD;

    { // unused
    field_9A8[0] = 0;
    field_9A8[1] = 0;
    field_9A8[2] = 0;
    field_9A8[3] = 1.0f; // see SetupSuspensionLines
    }

    SetupSuspensionLines();

    m_nQuadFlags &= ~1u; // useless
    m_fSteerAngle = 0.0f;
}

CQuadBike* CQuadBike::Constructor(int32 modelIndex, eVehicleCreatedBy createdBy) {
    this->CQuadBike::CQuadBike(modelIndex, createdBy);
    return this;
}

// 0x6CE2B0
void CQuadBike::Fix() {
    for (auto i = 2; i < 6; i++) {
        m_damageManager.SetDoorStatus(static_cast<eDoors>(i), eDoorStatus::DAMSTATE_NOTPRESENT);
    }
    vehicleFlags.bIsDamaged = false;
    RpClumpForAllAtomics(m_pRwClump, CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)2); // TODO Use RpAtomicVisibility::VISIBILITY_DAM instead of `2` here
    for (auto i = 0; i < 4; i++) {
        m_damageManager.SetWheelStatus((eCarWheel)i, eCarWheelStatus::WHEEL_STATUS_OK);
    }
}

// 0x6CDC90
CRideAnimData* CQuadBike::GetRideAnimData() {
    return &m_sRideAnimData;
}

// 0x6CEAD0
void CQuadBike::PreRender() {
    CAutomobile::PreRender();

    auto modelInfo = CModelInfo::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();
    {
        CVector wheelPos;
        modelInfo->GetWheelPosn(CARWHEEL_REAR_LEFT, wheelPos, false);
        SetTransmissionRotation(
            m_aCarNodes[QUAD_REAR_AXLE],
            m_wheelPosition[CARWHEEL_REAR_LEFT],
            m_wheelPosition[CARWHEEL_REAR_RIGHT],
            wheelPos,
            false
        );
    }
    
    CVector wheelFrontLeftPos;
    modelInfo->GetWheelPosn(CARWHEEL_FRONT_LEFT, wheelFrontLeftPos, false);
    
    // Original code saves position of each matrix (because calls to SetRotation set the pos. to 0), then restores it
    // We just use SetRotateYOnly which doesn't modify the position
    
    if (auto suspensionLF = m_aCarNodes[eQuadBikeNodes::QUAD_SUSPENSION_LF]) {
        CMatrix mat;
        mat.Attach(&suspensionLF->modelling, false);
        mat.SetRotateYOnly(atan2(m_wheelPosition[CARWHEEL_FRONT_LEFT] - wheelFrontLeftPos.z, fabs(wheelFrontLeftPos.x)));
        mat.UpdateRW();
    }
    
    if (auto suspensionRF = m_aCarNodes[eQuadBikeNodes::QUAD_SUSPENSION_RF]) {
        CMatrix mat;
        mat.Attach(&suspensionRF->modelling, false);
        mat.SetRotateYOnly(-atan2(m_wheelPosition[eCarWheel::CARWHEEL_FRONT_RIGHT] - wheelFrontLeftPos.z, fabs(wheelFrontLeftPos.x)));
        mat.UpdateRW();
    }
    
    if (auto handlebars = m_aCarNodes[eQuadBikeNodes::QUAD_HANDLEBARS]) {
        CMatrix mat;
        mat.Attach(&handlebars->modelling, false);
        mat.SetRotateZOnly(QUAD_HBSTEER_ANIM_MULT * m_sRideAnimData.m_fHandlebarsAngle);
        mat.UpdateRW();
    }
}

// 0x6CE460
bool CQuadBike::ProcessAI(uint32& extraHandlingFlags) {
    if (m_nStatus != STATUS_PLAYER) {
        return CAutomobile::ProcessAI(extraHandlingFlags);
    }

    GetColModel(); // Called, but not used?

    m_autoPilot.carCtrlFlags.bHonkAtCar = false;
    m_autoPilot.carCtrlFlags.bHonkAtPed = false;
    const auto recID = m_autoPilot.m_vehicleRecordingId;
    if (recID >= 0 && !CVehicleRecording::bUseCarAI[recID]) {
        return false;
    }
    m_vecCentreOfMass = m_pHandlingData->m_vecCentreOfMass;
    if (m_pDriver && m_pDriver->IsPlayer()) {    
        PruneReferences();
        if (m_pDriver->m_nPedState == PEDSTATE_ARRESTED ||
            m_pDriver->GetTaskManager().IsAnyTaskActiveByType({TASK_SIMPLE_CAR_WAIT_TO_SLOW_DOWN, TASK_COMPLEX_CAR_SLOW_BE_DRAGGED_OUT, TASK_COMPLEX_CAR_QUICK_BE_DRAGGED_OUT})
        ) {
            vehicleFlags.bIsHandbrakeOn = true;
            m_fBreakPedal = 1.0f;
            m_fGasPedal = 0.0f;
        } else {
            ProcessControlInputs((uint8)m_pDriver->m_nPedType);
            CPad* pad = m_pDriver->AsPlayer()->GetPadFromPlayer();
            
            float fTurnForcePerTimeStep = 0.0f;
            const float fLeanDirection = DotProduct(m_vecTurnSpeed, m_matrix->GetRight());
            if (m_sRideAnimData.dword10 >= 0.0f || fLeanDirection >= m_pHandling->m_fLeanBakCOM) {
                // Lean forward
                if (m_sRideAnimData.dword10 > 0.0f) {
                    if (fLeanDirection > -m_pHandling->m_fLeanFwdCOM) {
                        if (m_nNumContactWheels) {
                            if (m_fBreakPedal > 0.0f) {
                                fTurnForcePerTimeStep =
                                    m_pHandling->m_fLeanFwdForce * m_fTurnMass * m_sRideAnimData.dword10 * std::min(0.1f, m_vecMoveSpeed.Magnitude());
                            }
                        } else {
                            fTurnForcePerTimeStep = m_fTurnMass * m_sRideAnimData.dword10 * 0.0015f;
                        }
                        const float fTurnForce = -CTimer::GetTimeStep() * fTurnForcePerTimeStep;
                        ApplyTurnForce(m_matrix->GetUp() * fTurnForce, m_vecCentreOfMass + m_matrix->GetForward());
                    }
                }
            } else {
                // Lean back
                if (m_nNumContactWheels) {
                    if (m_fBreakPedal == 0.0f && !vehicleFlags.bIsHandbrakeOn) {
                        fTurnForcePerTimeStep =
                            m_pHandling->m_fLeanBakForce * m_fTurnMass * m_sRideAnimData.dword10 * std::min(0.1f, m_vecMoveSpeed.Magnitude());
                    }
                } else {
                    fTurnForcePerTimeStep = m_fTurnMass * m_sRideAnimData.dword10 * 0.0015f;
                }
                const float fTurnForce = -CTimer::GetTimeStep() * fTurnForcePerTimeStep;
                ApplyTurnForce(m_matrix->GetUp() * fTurnForce, m_vecCentreOfMass + m_matrix->GetForward());
            }

            if (IsAnyWheelNotMakingContactWithGround() && pad) {
                float steeringLeftRightProgress = (float)pad->GetSteeringLeftRight() / 128.0f;
                if (CCamera::m_bUseMouse3rdPerson && fabs(steeringLeftRightProgress) < 0.05f) {
                    steeringLeftRightProgress = clamp(CPad::NewMouseControllerState.X / 100.0f, -1.5f, 1.5f);
                }
                if (vehicleFlags.bIsHandbrakeOn) {
                    const float fTurnSpeed_Dot_MatUp = DotProduct(m_vecTurnSpeed, m_matrix->GetUp());
                    if (fTurnSpeed_Dot_MatUp < 0.029 && steeringLeftRightProgress < 0.0f ||
                        fTurnSpeed_Dot_MatUp > -0.029 && steeringLeftRightProgress > 0.0f)
                    {
                        const float fTurnForce = CTimer::GetTimeStep() * m_fTurnMass * 0.0015f * steeringLeftRightProgress;
                        ApplyTurnForce(m_matrix->GetRight() * fTurnForce, m_vecCentreOfMass + m_matrix->GetRight());
                    }
                } else if (pad->GetAccelerate()) {
                    const float fTurnSpeed_Dot_MatFwd = DotProduct(m_vecTurnSpeed, m_matrix->GetForward());
                    if (fTurnSpeed_Dot_MatFwd < 0.029 && steeringLeftRightProgress < 0.0f ||
                        fTurnSpeed_Dot_MatFwd > -0.029 && steeringLeftRightProgress > 0.0f)
                    {
                        const float fTurnForce = CTimer::GetTimeStep() * m_fTurnMass * 0.0015f * steeringLeftRightProgress;
                        ApplyTurnForce(m_matrix->GetRight() * fTurnForce, m_vecCentreOfMass + m_matrix->GetRight());
                    }
                }
            }
            const float fValue = std::pow(m_pHandling->m_fDesLean, CTimer::GetTimeStep()); // TODO: Name this variable properly
            m_sRideAnimData.m_fAnimLean = fValue * m_sRideAnimData.m_fAnimLean - m_pHandling->m_fFullAnimLean * m_fSteerAngle / DegreesToRadians(m_pHandlingData->m_fSteeringLock) * (1.0f - fValue);
                
            DoDriveByShootings();

            if (CCheat::IsActive(CHEAT_CARS_ON_WATER)) {
                Remove();
            }
        }
        
    }
    return false;
}

// 0x6CDCC0
void CQuadBike::ProcessControl() {
    if (m_nStatus != STATUS_PLAYER || !bDoQuadDamping) {
        CAutomobile::ProcessControl();
        return;
    }

    const auto turnSpeed_Mult_Matrix = Multiply3x3(m_vecTurnSpeed, *m_matrix);
    float v2 = vecQuadResistance.y, v5 = vecQuadResistance.x;
    if (AreFrontWheelsNotTouchingGround()) {
        if (!AreRearWheelsNotTouchingGround() && m_matrix->GetForward().z > 0.0f) {
            v5 = vecQuadResistance.x - std::min(0.07f, fabs(m_pHandling->m_fWheelieAng - m_matrix->GetForward().z) * 0.25f);
        }
    } else {
        if (m_aWheelTimer[CARWHEEL_REAR_LEFT] == 1.0f && m_aWheelTimer[CARWHEEL_REAR_RIGHT] == 1.0f) {
            if (m_matrix->GetForward().z < 0.0f) {
                v5 = vecQuadResistance.x * (0.9f + std::min(0.1f, fabs(m_pHandling->m_fStoppieAng - m_matrix->GetForward().z) * 0.3f));
            }
        } else {
            v2 = 0.5f;
        }
    }

    const CVector worldSpaceSpeed = Multiply3x3(m_vecTurnSpeed, *m_matrix);
    CVector unk{ // In the original code `x` is calculated once then immediately overwritten by the below line
        std::pow(vecQuadResistance.x, CTimer::GetTimeStep()),
        vecQuadResistance.y / (worldSpaceSpeed.y * worldSpaceSpeed.y + 1.0f),
        1.0f
    };
    const auto worldCentreOfMass = Multiply3x3(m_vecCentreOfMass, *m_matrix);

    const float v9 = std::pow(unk.y, CTimer::GetTimeStep()) * worldSpaceSpeed.y - worldSpaceSpeed.y;
    ApplyTurnForce(m_matrix->GetUp() * -1.0f * v9 * m_fTurnMass, m_matrix->GetRight() + worldCentreOfMass);

    const float v19 = worldSpaceSpeed.x * unk.x - worldSpaceSpeed.x;
    ApplyTurnForce(m_matrix->GetUp() * v19 * m_fTurnMass, m_matrix->GetForward() + worldCentreOfMass);

    CAutomobile::ProcessControl();
}

// 0x6CE020
void CQuadBike::ProcessControlInputs(uint8 playerNum) {
    CAutomobile::ProcessControlInputs(playerNum);

    CPad* pad = CPad::GetPad(playerNum);
    if (!CCamera::m_bUseMouse3rdPerson || !m_bEnableMouseSteering) {
        m_sRideAnimData.dword10 += (-pad->GetSteeringUpDown() / 128.0f - m_sRideAnimData.dword10) * CTimer::GetTimeStep() * 0.2f;
    } else {
        if (CPad::NewMouseControllerState.X == 0.0f && CPad::NewMouseControllerState.Y == 0.0f && // todo: Use CPad::? func
            (fabs(m_fRawSteerAngle) <= 0.0f || m_nLastControlInput != eControllerType::CONTROLLER_MOUSE || pad->IsSteeringInAnyDirection())
        ) {
            if (pad->GetSteeringUpDown() || m_nLastControlInput != eControllerType::CONTROLLER_MOUSE) {
                m_nLastControlInput = eControllerType::CONTROLLER_KEYBOARD1;
                m_sRideAnimData.dword10 += (-pad->GetSteeringUpDown() / 128.0f - m_sRideAnimData.dword10) * CTimer::GetTimeStep() * 0.2f;
            }
        } else {
            m_nLastControlInput = eControllerType::CONTROLLER_MOUSE;
            if (!pad->NewState.m_bVehicleMouseLook) {
                m_sRideAnimData.dword10 += CPad::NewMouseControllerState.Y * -0.035f;
            }
            if (pad->NewState.m_bVehicleMouseLook || fabs(m_sRideAnimData.dword10) < 0.35f) {
                m_sRideAnimData.dword10 *= pow(0.98f, CTimer::GetTimeStep());
            }
        }
    }
    m_sRideAnimData.dword10 = clamp(m_sRideAnimData.dword10, -1.0f, 1.0f);
    if (pad->DisablePlayerControls)
        m_sRideAnimData.dword10 = 0.0f;
}

// 0x6CE280
void CQuadBike::ProcessDrivingAnims(CPed* driver, uint8 bBlend) {
    if (!m_bOffscreen) { // see CBike::ProcessDrivingAnims
        CBike::ProcessRiderAnims(driver, this, &m_sRideAnimData, m_pHandling, 0);
    }
}

// 0x6CE270
void CQuadBike::ProcessSuspension() {
    CAutomobile::ProcessSuspension();
}

// 0x6CDCB0
void CQuadBike::ResetSuspension() {
    CAutomobile::ResetSuspension();
}

// 0x6CE340
void CQuadBike::SetupDamageAfterLoad() {
    if (m_aCarNodes[QUAD_BODY_FRONT])
        CAutomobile::SetBumperDamage(FRONT_BUMPER, false);

    if (m_aCarNodes[QUAD_BODY_REAR])
        CAutomobile::SetDoorDamage(DOOR_BONNET, false);
}

// 0x6CDCA0
void CQuadBike::SetupSuspensionLines() {
    field_9A8[0] = 1.0f;
    CAutomobile::SetupSuspensionLines();
}
