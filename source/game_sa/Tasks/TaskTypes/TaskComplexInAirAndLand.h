#pragma once

#include "TaskComplex.h"

class CTaskComplexInAirAndLand : public CTaskComplex {
public:
    bool m_bUsingJumpGlide;
    bool m_bUsingFallGlide;
    bool m_bInvalidClimb;

public:
    CTaskComplexInAirAndLand(bool bUsingJumpGlide, bool bUsingFallGlide);
    ~CTaskComplexInAirAndLand() override {}

    CTask* Clone() override { return new CTaskComplexInAirAndLand(m_bUsingJumpGlide, m_bUsingFallGlide); }
    eTaskType GetTaskType() override { return TASK_COMPLEX_IN_AIR_AND_LAND; }
    CTask* CreateFirstSubTask(CPed* ped) override;
    CTask* CreateNextSubTask(CPed* ped) override;
    CTask* ControlSubTask(CPed* ped) override;

    CTask* CreateFirstSubTask_Reversed(CPed* ped);
    CTask* CreateNextSubTask_Reversed(CPed* ped);
    CTask* ControlSubTask_Reversed(CPed* ped);

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CTaskComplexInAirAndLand* Constructor(bool bUsingJumpGlide, bool bUsingFallGlide);
};

VALIDATE_SIZE(CTaskComplexInAirAndLand, 0x10);
