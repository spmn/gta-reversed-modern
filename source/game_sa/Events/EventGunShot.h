#pragma once

#include "EventEditableResponse.h"
#include "Ped.h"

class CEventGunShot : public CEventEditableResponse {
public:
    CEntity* m_entity;
    CVector  m_startPoint;
    CVector  m_endPoint;
    bool     m_bHasNoSound;

    static float& ms_fGunShotSenseRangeForRiot2;
    static void InjectHooks();

    CEventGunShot(CEntity* entity, CVector startPoint, CVector endPoint, bool bHasNoSound);
    ~CEventGunShot();
private:
    CEventGunShot* Constructor(CEntity* entity, CVector startPoint, CVector endPoint, bool bHasNoSound);
public:
    eEventType GetEventType() const override { return EVENT_SHOT_FIRED; }
    int32 GetEventPriority() const override { return 35; }
    int32 GetLifeTime() override { return 0; }
    bool AffectsPed(CPed* ped) override;
    bool IsCriminalEvent() override;
    void ReportCriminalEvent(CPed* ped) override { } // empty
    CEntity* GetSourceEntity() const override { return m_entity; }
    bool TakesPriorityOver(const CEvent& refEvent) override;
    float GetLocalSoundLevel() override { return m_bHasNoSound ? 0.0f : 160.0f; }
    bool CanBeInterruptedBySameEvent() override { return true; }
    CEventEditableResponse* CloneEditable() override;

    bool AffectsPed_Reversed(CPed* ped);
    bool IsCriminalEvent_Reversed();
    bool TakesPriorityOver_Reversed(const CEvent& refEvent);
    CEventEditableResponse* CloneEditable_Reversed();
};

VALIDATE_SIZE(CEventGunShot, 0x34);
