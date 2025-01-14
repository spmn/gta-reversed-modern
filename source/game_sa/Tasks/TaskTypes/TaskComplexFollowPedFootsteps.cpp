#include "StdInc.h"

#include "TaskComplexFollowPedFootsteps.h"
// #include "PointRoute.h"

void CTaskComplexFollowPedFootsteps::InjectHooks() {
    RH_ScopedClass(CTaskComplexFollowPedFootsteps);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x694E20);
}

// 0x694E20
CTaskComplexFollowPedFootsteps::CTaskComplexFollowPedFootsteps(CPed* ped) : CTaskComplex() {
    m_targetPed               = ped;
    m_updateGoToPoint         = false;
    m_subTaskCreateCheckTimer = CTimer::GetTimeInMS();
    m_lineOfSightCheckTimer   = 0;
    m_pointRoute              = nullptr;
    m_moveState               = PEDMOVE_WALK;

    if (ped)
        ped->RegisterReference(reinterpret_cast<CEntity**>(&m_targetPed));

    m_pointRoute = new CPointRoute();
    if (m_pointRoute)
        m_pointRoute->field_0 = 0;
}

CTaskComplexFollowPedFootsteps* CTaskComplexFollowPedFootsteps::Constructor(CPed* ped) {
    this->CTaskComplexFollowPedFootsteps::CTaskComplexFollowPedFootsteps(ped);
    return this;
}

CTaskComplexFollowPedFootsteps::~CTaskComplexFollowPedFootsteps() {
    if (m_targetPed)
        m_targetPed->CleanUpOldReference(reinterpret_cast<CEntity**>(&m_targetPed));

    delete m_pointRoute;
    m_pointRoute = nullptr;
}

bool CTaskComplexFollowPedFootsteps::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return plugin::CallMethodAndReturn<bool, 0x694ED0, CTaskComplexFollowPedFootsteps*, CPed*, eAbortPriority, const CEvent*>(this, ped, priority, event);
}

CTask* CTaskComplexFollowPedFootsteps::CreateNextSubTask(CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x694EE0, CTaskComplexFollowPedFootsteps*, CPed*>(this, ped);
}

CTask* CTaskComplexFollowPedFootsteps::CreateFirstSubTask(CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x695000, CTaskComplexFollowPedFootsteps*, CPed*>(this, ped);
}

CTask* CTaskComplexFollowPedFootsteps::ControlSubTask(CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x695090, CTaskComplexFollowPedFootsteps*, CPed*>(this, ped);
}

CTask* CTaskComplexFollowPedFootsteps::CreateSubTask(eTaskType taskType, CPed* ped) {
    return plugin::CallMethodAndReturn<CTask*, 0x695E40, CTaskComplexFollowPedFootsteps*, eTaskType, CPed*>(this, taskType, ped);
}
