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
    }else{
        std::cout << "⚠️ 模拟器配置加载失败，使用默认值" << std::endl;
        setDegradationTrend(57.0f);
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

/* BatteryPack DataSimulator::generateFrame() { */
/*     BatteryPack pack; */
/*     pack.timestamp = std::chrono::system_clock::now(); */

/*     frame_count++; */

/*     // 模拟你的车真实数据 */
/*     pack.total_voltage = 320.0f + (std::sin(frame_count * 0.1f) * 8.0f);  // 312-328V 波动 */
/*     pack.total_current = (frame_count % 50 < 20) ? -45.0f : 8.0f;        // 模拟充放电 */

/*     pack.soc = 75.0f - (frame_count % 100) * 0.6f;                       // SOC缓慢变化 */
/*     pack.soh = current_soh - (frame_count / 500) * 0.02f;                // 缓慢衰减 */

/*     pack.max_cell_voltage = 3.65f; */
/*     pack.min_cell_voltage = 3.28f; */
/*     pack.max_temperature = 42.5f;                                         // 珠海夏天温度 */
/*     pack.min_temperature = 28.0f; */

/*     pack.estimated_range = 235.0f;                                        // 你的真实续航 */

/*     pack.charging_status = (pack.total_current < 0) ? "SlowCharging" : "Discharging"; */

/*     // 模拟96串电池（常见LFP） */
/*     pack.cells.resize(96); */
/*     for (int i = 0; i < 96; ++i) { */
/*         pack.cells[i].id = i + 1; */
/*         pack.cells[i].voltage = simulateCellVoltage(i); */
/*         pack.cells[i].temperature = simulateTemperature(i); */
/*     } */

/*     // 偶尔加入故障 */
/*     if (frame_count % 180 == 0) { */
/*         pack.faults.push_back("P0A1F_Cell_OverTemperature"); */
/*     } */

/*     return pack; */
/* } */

BatteryPack DataSimulator::generateFrame() {
    BatteryPack pack;
    pack.timestamp = std::chrono::system_clock::now();

    frame_count++;

    // 使用JSON配置的参数
    pack.total_voltage = base_voltage + (std::sin(frame_count * 0.1f) * voltage_fluctuation);
    pack.total_current = (frame_count % 50 < 20) ? -45.0f : 8.0f;

    pack.soc = soc_start - (frame_count % 100) * 0.6f;
    pack.soh = current_soh - (frame_count / 500) * 0.02f;

    pack.max_cell_voltage = max_cell_v;
    pack.min_cell_voltage = min_cell_v;
    pack.max_temperature = max_temp;
    pack.min_temperature = min_temp;

    pack.estimated_range = estimated_range_base;

    pack.charging_status = (pack.total_current < 0) ? "SlowCharging" : "Discharging";

    // 模拟电芯
    pack.cells.resize(num_cells);
    for (int i = 0; i < num_cells; ++i) {
        pack.cells[i].id = i + 1;
        pack.cells[i].voltage = simulateCellVoltage(i);
        pack.cells[i].temperature = simulateTemperature(i);
    }

    // 偶尔加入故障
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
