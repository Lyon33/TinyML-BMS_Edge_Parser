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
#include "json.hpp"
#include <random>
#include <vector>
#include <string>

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
    float base_voltage = 320.0f;
    float voltage_fluctuation = 8.0f;
    float soc_start = 75.0f;
    float real_soc = 100.0f;    // 新增真实 SOC 状态
    float estimated_range_base = 235.0f;
    float max_cell_v = 3.65f;
    float min_cell_v = 3.28f;
    float max_temp = 42.5f;
    float min_temp = 28.0f;
    int num_cells = 96;
    int frame_count = 0;

    // 从JSON 加载模拟器参数（工业级可配置）
    bool loadConfig(const std::string& configPath = "../config/bms_protocol.json");;

    // 模拟电压波动
    float simulateCellVoltage(int cell_id);
    float simulateTemperature(int cell_id);
};
