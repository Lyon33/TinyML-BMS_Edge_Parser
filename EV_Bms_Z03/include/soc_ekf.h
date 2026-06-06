#pragma once
#include "ocv_table.h"

class SocEKF {
public:
    SocEKF(float capacity, float soh, float dt);
    float update(float current, float voltage, float temperature = 25.0f);

private:
    OcTable ocTable;
    bool isFull = false;
    float lastValidSoc = 100.0f;
    float x, P, Q, R;
    float capacity_ah;
    float soh;
    float dt;
};
