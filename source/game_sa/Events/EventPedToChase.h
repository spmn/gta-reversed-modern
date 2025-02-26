#pragma once

#include "Event.h"
#include "Ped.h"

class CEventPedToChase : public CEvent {
public:
    CPed* m_ped;

public:
    CEventPedToChase(CPed* ped);
    ~CEventPedToChase() override;

    eEventType GetEventType() const override { return EVENT_PED_TO_CHASE; }
    int32 GetEventPriority() const override { return 1; }
    int32 GetLifeTime() override { return 0; }
    CEvent* Clone() override;
    bool AffectsPed(CPed* ped) override { return ped->IsAlive(); }

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventPedToChase* Constructor(CPed* ped);

    CEvent* Clone_Reversed();
};

VALIDATE_SIZE(CEventPedToChase, 0x10);
