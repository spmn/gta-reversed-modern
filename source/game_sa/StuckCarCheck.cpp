#include "StdInc.h"

#include "StuckCarCheck.h"

void CStuckCarCheck::InjectHooks() {
    RH_ScopedClass(CStuckCarCheck);
    RH_ScopedCategoryGlobal();

    // RH_ScopedInstall(Init, 0x4639E0);
    // RH_ScopedInstall(AddCarToCheck, 0x465970);
    // RH_ScopedInstall(AttemptToWarpVehicle, 0x463A60);
    // RH_ScopedInstall(ClearStuckFlagForCar, 0x463C40);
    // RH_ScopedInstall(HasCarBeenStuckForAWhile, 0x463C00);
    // RH_ScopedInstall(IsCarInStuckCarArray, 0x463C70);
    // RH_ScopedInstall(Process, 0x465680);
    // RH_ScopedInstall(RemoveCarFromCheck, 0x463B80);
}

// 0x4639E0
void CStuckCarCheck::Init() {
    return plugin::Call<0x4639E0, CStuckCarCheck*>(this);

    for (auto& car : m_aStuckCars) {
        ResetArrayElement(car);
    }
}

// 0x465970
void CStuckCarCheck::AddCarToCheck(int32 carHandle, float distance, uint32 time, uint8 a5, bool bStuck, bool bFlipped, bool bWarp, int8 pathId) {
    plugin::CallMethod<0x465970, CStuckCarCheck*, int32, float, uint32, uint8, bool, bool, bool, int8>(this, carHandle, distance, time, a5, bStuck, bFlipped, bWarp, pathId);
}

// 0x463A60
bool CStuckCarCheck::AttemptToWarpVehicle(CVehicle* vehicle, CVector* origin, float orientation) {
    return plugin::CallMethodAndReturn<bool, 0x463A60, CStuckCarCheck*, CVehicle*, CVector*, float>(this, vehicle, origin, orientation);
}

// 0x463C40
void CStuckCarCheck::ClearStuckFlagForCar(int32 carHandle) {
    return plugin::Call<0x463C40, CStuckCarCheck*, int32>(this, carHandle);

    for (auto& car : m_aStuckCars) {
        if (car.m_nCarHandle == carHandle) {
            car.m_bCarStuck = false;
            return;
        }
    }
}

// 0x463C00
bool CStuckCarCheck::HasCarBeenStuckForAWhile(int32 carHandle) {
    return plugin::CallMethodAndReturn<bool, 0x463C00, CStuckCarCheck*, int32>(this, carHandle);

    for (auto& car : m_aStuckCars) {
        if (car.m_nCarHandle == carHandle && car.m_bCarStuck) {
            return true;
        }
    }
    return false;
}

// 0x463C70
bool CStuckCarCheck::IsCarInStuckCarArray(int32 carHandle) {
    return plugin::CallMethodAndReturn<bool, 0x463C70, CStuckCarCheck*, int32>(this, carHandle);

    for (auto& car : m_aStuckCars) {
        if (car.m_nCarHandle == carHandle) {
            return true;
        }
    }
    return false;
}

// 0x465680
void CStuckCarCheck::Process() {
    plugin::CallMethod<0x465680, CStuckCarCheck*>(this);
}

// See CStuckCarCheck::ResetArrayElement
// 0x463B80
void CStuckCarCheck::RemoveCarFromCheck(int32 carHandle) {
    return plugin::CallMethod<0x463B80, CStuckCarCheck*, int32>(this, carHandle);

    for (auto& car : m_aStuckCars) {
        if (car.m_nCarHandle == carHandle) {
            ResetArrayElement(car);
        }
    }
}

// Used in Init and RemoveCarFromCheck
// Not presented on PC because inlined. See Android PDB (v1.0 0x2B3534)
void CStuckCarCheck::ResetArrayElement(StuckCar& car) {
    car.m_nCarHandle = -1;
    car.m_vCarPos = CVector(-5000.0f, -5000.0f, -5000.0f);
    car.m_nStartTime = -1;
    car.m_fDistance = 0.0f;
    car.m_nStuckTime = 0;
    car.m_bCarStuck = false;
    car.field_1D = false;
    car.m_bStuck = false;
    car.m_bFlipped = false;
    car.m_bbWarp = false;
    car.m_pathID = false;
}
