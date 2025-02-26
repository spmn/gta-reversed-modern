/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Automobile.h"

enum ePlaneNodes {
    PLANE_NODE_NONE = 0,
    PLANE_CHASSIS = 1,
    PLANE_WHEEL_RF = 2,
    PLANE_WHEEL_RM = 3,
    PLANE_WHEEL_RB = 4,
    PLANE_WHEEL_LF = 5,
    PLANE_WHEEL_LM = 6,
    PLANE_WHEEL_LB = 7,
    PLANE_DOOR_RF = 8,
    PLANE_DOOR_RR = 9,
    PLANE_DOOR_LF = 10,
    PLANE_DOOR_LR = 11,
    PLANE_STATIC_PROP = 12,
    PLANE_MOVING_PROP = 13,
    PLANE_STATIC_PROP2 = 14,
    PLANE_MOVING_PROP2 = 15,
    PLANE_RUDDER = 16,
    PLANE_ELEVATOR_L = 17,
    PLANE_ELEVATOR_R = 18,
    PLANE_AILERON_L = 19,
    PLANE_AILERON_R = 20,
    PLANE_GEAR_L = 21,
    PLANE_GEAR_R = 22,
    PLANE_MISC_A = 23,
    PLANE_MISC_B = 24,
    PLANE_NUM_NODES
};

class CPlane : public CAutomobile {
public:
    float        m_fLeftRightSkid;
    float        m_fSteeringUpDown;
    float        m_fSteeringLeftRight;
    float        m_fAccelerationBreakStatus;
    float        m_fAccelerationBreakStatusPrev;
    float        m_fSteeringFactor;
    float        field_9A0;
    float        m_planeCreationHeading; // The heading when plane is created or placed on road properly
    float        m_maxAltitude;
    float        m_altitude;
    float        m_minAltitude;
    float        m_planeHeading;
    float        m_planeHeadingPrev;
    float        m_forwardZ;
    uint32       m_nStartedFlyingTime;
    float        m_fPropSpeed;
    float        field_9C8;
    float        m_fLandingGearStatus;
    int32        m_planeDamageWave;
    FxSystem_c** m_pGunParticles;
    uint8        m_nFiringMultiplier;
    int32        field_9DC;
    int32        field_9E0;
    int32        field_9E4;
    FxSystem_c*  m_apJettrusParticles[4];
    FxSystem_c*  m_pSmokeParticle;
    uint32       m_nSmokeTimer;
    bool         m_bSmokeEjectorEnabled;

public:
    static int32& GenPlane_ModelIndex;
    static uint32& GenPlane_Status;
    static uint32& GenPlane_LastTimeGenerated;

    static bool& GenPlane_Active;               // true
    static float& ANDROM_COL_ANGLE_MULT;        // 0.00015
    static float& HARRIER_NOZZLE_ROTATE_LIMIT;  // 5000.0
    static float& HARRIER_NOZZLE_SWITCH_LIMIT;  // 3000.0
    static float& PLANE_MIN_PROP_SPEED;         // 0.05
    static float& PLANE_STD_PROP_SPEED;         // 0.18
    static float& PLANE_MAX_PROP_SPEED;         // 0.34
    static float& PLANE_ROC_PROP_SPEED;         // 0.01

public:
    CPlane(plugin::dummy_func_t) : CAutomobile(plugin::dummy) { /* todo: remove NOTSA */ }
    CPlane(int32 modelIndex, eVehicleCreatedBy createdBy);
    ~CPlane() override;

    bool SetUpWheelColModel(CColModel* wheelCol) override;
    bool BurstTyre(uint8 tyreComponentId, bool bPhysicalEffect) override;
    void ProcessControl() override;
    void ProcessControlInputs(uint8 playerNum) override;
    void ProcessFlyingCarStuff() override;
    void PreRender() override;
    void Render() override;
    void BlowUpCar(CEntity* damager, uint8 bHideExplosion) override;
    void Fix() override;
    void OpenDoor(CPed* ped, int32 componentId, eDoors door, float doorOpenRatio, bool playSound) override;
    void SetupDamageAfterLoad() override;
    void VehicleDamage(float damageIntensity, eVehicleCollisionComponent component, CEntity* damager, CVector* vecCollisionCoors, CVector* vecCollisionDirection, eWeaponType weapon) override;

    static void InitPlaneGenerationAndRemoval();

    void IsAlreadyFlying();
    void SetGearUp();
    void SetGearDown();

    static uint32 CountPlanesAndHelis();
    static bool AreWeInNoPlaneZone();
    static bool AreWeInNoBigPlaneZone();
    static void SwitchAmbientPlanes(bool enable);
    static void FindPlaneCreationCoors(CVector* center, CVector* playerCoords, float* outHeading, float* outHeight, bool arg4);
    static void DoPlaneGenerationAndRemoval();

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CPlane* Constructor(int32 modelIndex, eVehicleCreatedBy createdBy) { this->CPlane::CPlane(modelIndex, createdBy); return this; }
    CPlane* Destroy() { this->CPlane::~CPlane(); return this; }

    bool SetUpWheelColModel_Reversed(CColModel* wheelCol) { return CPlane::SetUpWheelColModel(wheelCol); };
    bool BurstTyre_Reversed(uint8 tyreComponentId, bool bPhysicalEffect) { return CPlane::BurstTyre(tyreComponentId, bPhysicalEffect); };
    void ProcessControl_Reversed() { CPlane::ProcessControl(); };
    void ProcessControlInputs_Reversed(uint8 playerNum) { CPlane::ProcessControlInputs(playerNum); };
    void ProcessFlyingCarStuff_Reversed() { CPlane::ProcessFlyingCarStuff(); };
    void PreRender_Reversed() { CPlane::PreRender(); };
    void Render_Reversed() { CPlane::Render(); };
    void BlowUpCar_Reversed(CEntity* damager, uint8 bHideExplosion) { CPlane::BlowUpCar(damager, bHideExplosion); };
    void Fix_Reversed() { CPlane::Fix(); };
    void OpenDoor_Reversed(CPed* ped, int32 componentId, eDoors door, float doorOpenRatio, bool playSound) { CPlane::OpenDoor(ped, componentId, door, doorOpenRatio, playSound); };
    void SetupDamageAfterLoad_Reversed() { CPlane::SetupDamageAfterLoad(); };
    void VehicleDamage_Reversed(float damageIntensity, eVehicleCollisionComponent component, CEntity* damager, CVector* vecCollisionCoors, CVector* vecCollisionDirection, eWeaponType weapon) { CPlane::VehicleDamage(damageIntensity, component, damager, vecCollisionCoors, vecCollisionDirection, weapon); };
};

VALIDATE_SIZE(CPlane, 0xA04);
