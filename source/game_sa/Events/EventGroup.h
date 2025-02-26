/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Event.h"

class CPed;

const int32 TOTAL_EVENTS_PER_EVENTGROUP = 16;

class CEventGroup {
public:
    CPed*   m_pPed;
    int32   m_count;
    CEvent* m_events[TOTAL_EVENTS_PER_EVENTGROUP];

public:
    static void InjectHooks();

    CEventGroup(CPed* ped);
    virtual ~CEventGroup();
    CEventGroup* Constructor(CPed* ped);

    CEvent* Add(CEvent* event, bool bValid = false);
    bool HasScriptCommandOfTaskType(eTaskType taskId);
    bool HasEventOfType(CEvent* event);
    bool HasEvent(CEvent* event);
    CEvent* GetHighestPriorityEvent();
    void TickEvents();
    void Remove(CEvent* event);
    void RemoveInvalidEvents(bool bRemoveNonScriptCommandEvents);
    void Reorganise();
    void Flush(bool bAvoidFlushingTaskComplexBeInGroup);
    CEvent* GetEventOfType(eEventType type) const noexcept;
};

VALIDATE_SIZE(CEventGroup, 0x4C);
