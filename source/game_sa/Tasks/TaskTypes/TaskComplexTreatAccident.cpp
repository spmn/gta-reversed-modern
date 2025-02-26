#include "StdInc.h"

#include "TaskComplexTreatAccident.h"

#include "TaskSimpleGiveCPR.h"
#include "TaskSimpleNone.h"
#include "TaskSimpleAchieveHeading.h"
#include "IKChainManager_c.h"

void CTaskComplexTreatAccident::InjectHooks()
{
    RH_ScopedClass(CTaskComplexTreatAccident);
    RH_ScopedCategory("Tasks/TaskTypes");
    RH_ScopedInstall(Constructor, 0x658AB0);
    RH_ScopedInstall(CreateSubTask, 0x659E90);
    RH_ScopedInstall(ComputeHeading, 0x658AF0);
    RH_ScopedInstall(Clone_Reversed, 0x659A90);
    RH_ScopedInstall(CreateFirstSubTask_Reversed, 0x65A8F0);
    RH_ScopedInstall(CreateNextSubTask_Reversed, 0x65A830);
    RH_ScopedInstall(ControlSubTask_Reversed, 0x658B90);
}

// 0x658AB0
CTaskComplexTreatAccident* CTaskComplexTreatAccident::Constructor(CAccident* pAcc)
{
    this->CTaskComplexTreatAccident::CTaskComplexTreatAccident(pAcc);
    return this;
}

// 0x658AB0
CTaskComplexTreatAccident::CTaskComplexTreatAccident(CAccident* accident)
{
    m_pAccident = accident;
}

// 0x65A830
CTask* CTaskComplexTreatAccident::CreateNextSubTask(CPed* ped)
{
    return CreateNextSubTask_Reversed(ped);
}

// 0x65A8F0
CTask* CTaskComplexTreatAccident::CreateFirstSubTask(CPed* ped)
{
    return CreateFirstSubTask_Reversed(ped);
}

// 0x658B90
CTask* CTaskComplexTreatAccident::ControlSubTask(CPed* ped)
{
    return ControlSubTask_Reversed(ped);
}

// 0x659A90
CTask* CTaskComplexTreatAccident::Clone()
{
    return Clone_Reversed();
}

CTask* CTaskComplexTreatAccident::CreateNextSubTask_Reversed(CPed* ped)
{
    eTaskType subTaskId = m_pSubTask->GetTaskType();

    if (subTaskId == TASK_SIMPLE_GIVE_CPR || subTaskId == TASK_NONE)
        return CreateSubTask(TASK_FINISHED, ped);

    if (subTaskId == TASK_SIMPLE_ACHIEVE_HEADING)
    {
        CPed* targetPed = m_pAccident->m_pPed;

        if (targetPed && !targetPed->bFadeOut)
        {
            targetPed->m_nDeathTime = CTimer::GetTimeInMS();
            auto newSubTask = CreateSubTask(TASK_SIMPLE_GIVE_CPR, ped);
            ped->m_fCurrentRotation = ComputeHeading(ped);
            ped->m_fAimingRotation = ped->m_fCurrentRotation;
            return newSubTask;
        }
        else
            return CreateSubTask(TASK_FINISHED, ped);
    }

    return nullptr;
}

CTask* CTaskComplexTreatAccident::CreateFirstSubTask_Reversed(CPed* ped)
{
    CPed* targetPed = m_pAccident->m_pPed;

    if (targetPed && !targetPed->bFadeOut)
    {
        targetPed->m_nDeathTime = CTimer::GetTimeInMS();
        ped->Say(232, 0, 1.0F, false, false, false);
        g_ikChainMan.LookAt("TaskTreatAccident", ped, targetPed, 5000, BONE_HEAD, nullptr, true, 0.25F, 500, 3, false);
        return CreateSubTask(TASK_SIMPLE_ACHIEVE_HEADING, ped);
    }
    else
        return CreateSubTask(TASK_NONE, ped);
}

CTask* CTaskComplexTreatAccident::ControlSubTask_Reversed(CPed* ped)
{
    return m_pSubTask;
}

CTask* CTaskComplexTreatAccident::Clone_Reversed()
{
    return new CTaskComplexTreatAccident(m_pAccident);
}

// 0x659E90
CTask* CTaskComplexTreatAccident::CreateSubTask(eTaskType taskType, CPed* ped)
{
    switch (taskType)
    {
    case TASK_SIMPLE_GIVE_CPR:
        return new CTaskSimpleGiveCPR(m_pAccident);
    case TASK_NONE:
        return new CTaskSimpleNone();
    case TASK_SIMPLE_ACHIEVE_HEADING:
        return new CTaskSimpleAchieveHeading(ComputeHeading(ped), 0.5F, 0.2F);
    default:
        return nullptr;
    }
}

// 0x658AF0
float CTaskComplexTreatAccident::ComputeHeading(CPed* ped)
{
    CVector bonePositions[2];

    m_pAccident->m_pPed->GetBonePosition(bonePositions[0], BONE_HEAD, false);
    m_pAccident->m_pPed->GetBonePosition(bonePositions[1], BONE_PELVIS, false);
    CVector targetPos = (bonePositions[0] + bonePositions[1]) * 0.5F;

    float fAngle = CGeneral::GetRadianAngleBetweenPoints(targetPos.x, targetPos.y, ped->GetPosition().x, ped->GetPosition().y);

    return CGeneral::LimitRadianAngle(fAngle);
}
