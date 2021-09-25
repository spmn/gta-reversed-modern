#pragma once

#include "PedAttractor.h"

class CPedLookAtAttractor : public CPedAttractor {
public:
    // 0x5EE960
    CPedLookAtAttractor(C2dEffect* effect, CEntity* entity, int a4)
        : CPedAttractor(effect, entity, a4, 1, 1.0f, 30000.0f, 3000.0f, 0.125f, 0.1f, 0.1f, 0.1f, 0) {
    }

    // 0x5EE9C0
    ePedAttractorType GetType() const override { return PED_ATTRACTOR_LOOK_AT; }
};