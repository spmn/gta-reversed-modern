#include "StdInc.h"

#include "Escalator.h"

void CEscalator::SwitchOff() {
    plugin::CallMethod<0x717860, CEscalator*>(this);
}

void CEscalator::AddThisOne(const CVector& vecStart, const CVector& vecBottom, const CVector& vecEnd, const CVector& vecTop, bool bMoveDown, CEntity* entity) {
    plugin::CallMethod<0x717970, CEscalator*, const CVector&, const CVector&, const CVector&, const CVector&, bool, CEntity*>(this, vecStart, vecBottom, vecEnd, vecTop, bMoveDown, entity);
}

void CEscalator::Update() {
    plugin::CallMethod<0x717D30, CEscalator*>(this);
}
