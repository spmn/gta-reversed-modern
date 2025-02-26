#pragma once

#include "Event.h"
#include "Ped.h"

class CEventPedToFlee : public CEvent {
public:
    CPed* m_ped;

public:
    explicit CEventPedToFlee(CPed* ped);
    ~CEventPedToFlee() override;

    eEventType GetEventType() const override { return EVENT_PED_TO_FLEE; }
    int32 GetEventPriority() const override { return 2; }
    int32 GetLifeTime() override { return 0; }
    CEvent* Clone() override;
    bool AffectsPed(CPed* ped) override { return ped->IsAlive(); }

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CEventPedToFlee* Constructor(CPed* ped);

    CEvent* Clone_Reversed();
};

VALIDATE_SIZE(CEventPedToFlee, 0x10);
