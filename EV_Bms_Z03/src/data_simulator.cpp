/*************************************************************************
* File Name: src/data_simulator.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:00:59 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// src/data_simulator.cpp
#include "data_simulator.h"
#include "json.hpp"
#include <iostream>
#include <chrono>
#include <fstream>

DataSimulator::DataSimulator() {
    /* if(!loadConfig("../config/bms_protocol.json")){ */
    /*     setDegradationTrend(57.0f); */
    if(loadConfig()){
        setDegradationTrend(current_soh);
        real_soc = 100.0f;      // 从满电开始
    }else{
        std::cout << "⚠️ 模拟器配置加载失败，使用默认值" << std::endl;
        setDegradationTrend(57.0f);
        real_soc = 100.0f;
    }
}

void DataSimulator::setDegradationTrend(float soh_start) {
    current_soh = soh_start;
}

bool DataSimulator::loadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "⚠️ 模拟器配置加载失败，使用默认值: " << configPath << std::endl;
        return false;
    }
    try {

        nlohmann::json j;
        file >> j;
        
        if (j.contains("simulator")) {
            auto& sim = j["simulator"];
            current_soh = sim.value("soh_start", 57.0f);
            base_voltage = sim.value("base_voltage", 320.0f);
            voltage_fluctuation = sim.value("voltage_fluctuation", 8.0f);
            soc_start = sim.value("soc_start", 75.0f);
            estimated_range_base = sim.value("estimated_range", 235.0f);
            max_cell_v = sim.value("max_cell_voltage", 3.65f);
            min_cell_v = sim.value("min_cell_voltage", 3.28f);
            max_temp = sim.value("max_temperature", 42.5f);
            min_temp = sim.value("min_temperature", 28.0f);
            num_cells = sim.value("cells", 96);

            /* std::cout << "✅ 模拟器参数已从JSON加载" << std::endl; */
            std::cout << "✅ 模拟器参数已从JSON加载 (SOH=" << current_soh 
                << "%, 电压基值=" << base_voltage << "V)" << std::endl;
            return true;
        }
    } catch (const std::exception& e) {

        std::cerr << "❌ 模拟器配置解析失败: " << e.what() << std::endl;
    }
    return false;
}

BatteryPack DataSimulator::generateFrame() {
    BatteryPack pack;
    pack.timestamp = std::chrono::system_clock::now();

    frame_count++;

    // ========== 1. 真车工况机（负充正放）==========
    static int phase = 0;      // 0=充电, 1=放电, 2=静置
    static int phase_timer = 0;

    phase_timer++;
    if (phase_timer > 400) {   // 约 20 秒换一次工况
        phase = (phase + 1) % 3;
        phase_timer = 0;
    }

    float current = 0.0f;
    switch (phase) {
    case 0: current = -7.0f;  break;   // ✅ 你实测的充电电流
    case 1: current = 12.0f;  break;   // ✅ 推算出的放电电流（10小时用车）
    case 2: current =  0.0f;  break;   // 静置
    }

    // ========== 2. 安时积分（真实 SOC）==========
    float dt = 0.05f;                   // 20Hz
    float capacity = 37.0f / 320.0f;    // 37kWh / 320V ≈ 115.6Ah
    float soh = current_soh / 100.0f;

    float delta_soc = (current * dt) / (capacity * soh) * 100.0f;
    real_soc -= delta_soc;
    real_soc = std::clamp(real_soc, 0.0f, 100.0f);

    // ========== 3. 电压 ==========
    float voltage = base_voltage;
    voltage += (real_soc - 50.0f) * 0.01f; // SOC 越高电压越高
    voltage += current * 0.05f;             // 负载压降
    voltage += std::sin(frame_count * 0.1f) * voltage_fluctuation;

    // ========== 4. 填包 ==========
    pack.total_current = current;
    pack.total_voltage = voltage;
    pack.soc = real_soc;
    pack.soh = current_soh;
    pack.estimated_range = real_soc * 2.35f;  // 100% → 235km

    pack.max_cell_voltage = max_cell_v;
    pack.min_cell_voltage = min_cell_v;
    pack.max_temperature = max_temp;
    pack.min_temperature = min_temp;

    pack.charging_status =
        (current < 0) ? "SlowCharging" : "Discharging";

    // ========== 5. 电芯 ==========
    pack.cells.resize(num_cells);
    for (int i = 0; i < num_cells; ++i) {
        pack.cells[i].id = i + 1;
        pack.cells[i].voltage = simulateCellVoltage(i);
        pack.cells[i].temperature = simulateTemperature(i);
    }

    // 故障（保留）
    if (frame_count % 180 == 0) {
        pack.faults.push_back("P0A1F_Cell_OverTemperature");
    }

    return pack;
}

float DataSimulator::simulateCellVoltage(int cell_id) {
    return 3.45f + (std::sin(cell_id + frame_count * 0.05f) * 0.22f);
}

float DataSimulator::simulateTemperature(int cell_id) {
    return 32.0f + (cell_id % 8) * 1.2f + (std::cos(frame_count * 0.1f) * 4.0f);
}
