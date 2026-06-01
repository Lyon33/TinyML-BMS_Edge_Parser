/*************************************************************************
* File Name: include/data_simulator.h
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 15:59:53 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// include/data_simulator.h
#pragma once
#include "battery_data.h"
#include <random>
#include <vector>

class DataSimulator {
public:
    DataSimulator();

    // 生成一帧模拟BMS数据
    BatteryPack generateFrame();

    // 模拟真实衰减趋势（SOH缓慢下降）
    void setDegradationTrend(float soh_start = 57.0f);

private:
    std::mt19937 gen{std::random_device{}()};
    float current_soh = 57.0f;        // 默认模拟你的车当前SOH ≈57%
    /* float base_capacity_kwh = 36.0f;  // 你实际充满一次36度电 */
    int frame_count = 0;

    // 模拟电压波动
    float simulateCellVoltage(int cell_id);
    float simulateTemperature(int cell_id);
};
