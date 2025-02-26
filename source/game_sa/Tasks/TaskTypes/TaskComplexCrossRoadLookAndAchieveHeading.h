#pragma once

#include "TaskComplex.h"

class CTaskComplexCrossRoadLookAndAchieveHeading : public CTaskComplex {
public:
    int32 m_nDuration;
    float m_fTargetHeading;

public:
    CTaskComplexCrossRoadLookAndAchieveHeading(int32 durationInMs, float fTargetHeading);
    ~CTaskComplexCrossRoadLookAndAchieveHeading() override;

    CTask* Clone() override { return new CTaskComplexCrossRoadLookAndAchieveHeading(m_nDuration, m_fTargetHeading); } // 0x636500
    CTask* ControlSubTask(CPed* ped) override { return m_pSubTask; } // 0x631CB0
    CTask* CreateFirstSubTask(CPed* ped) override { return CreateSubTask(TASK_SIMPLE_ACHIEVE_HEADING, ped); } // 0x631C90
    CTask* CreateNextSubTask(CPed* ped) override;
    CTask* CreateSubTask(eTaskType taskType, CPed* ped);
    eTaskType GetTaskType() override { return TASK_COMPLEX_CROSS_ROAD_LOOK_AND_ACHIEVE_HEADING; } // 0x631B10
    bool MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) override;

#if ANDROID
    void Serialize();
#endif

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CTaskComplexCrossRoadLookAndAchieveHeading* Constructor(int32 durationInMs, float fTargetHeading);
};
