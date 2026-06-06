// src/data_simulator.cpp
#include "data_simulator.h"
#include "json.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <chrono>
#include <fstream>

DataSimulator::DataSimulator() {
    if (loadConfig()) {
        std::cout << "✅ 模拟器参数已从JSON加载" << std::endl;
    } else {
        std::cout << "⚠️ 配置加载失败，使用默认值" << std::endl;
    }
}

bool DataSimulator::loadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) return false;

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("simulator")) {
            auto& sim = j["simulator"];
            current_soh = sim.value("soh_start", 57.0f);
            base_voltage = sim.value("base_voltage", 320.0f);
            voltage_fluctuation = sim.value("voltage_fluctuation", 8.0f);
            max_cell_v = sim.value("max_cell_voltage", 3.65f);
            min_cell_v = sim.value("min_cell_voltage", 3.28f);
            max_temp = sim.value("max_temperature", 42.5f);
            min_temp = sim.value("min_temperature", 28.0f);
            num_cells = sim.value("cells", 96);
            return true;
        }
    } catch (...) {}
    return false;
}

BatteryPack DataSimulator::generateFrame() {
    BatteryPack pack;
    pack.timestamp = std::chrono::system_clock::now();
    frame_count++;

    // 真实工况电流
    float current  = (frame_count % 600 < 300) ? 12.0f : -7.0f;

    pack.total_voltage = base_voltage + (std::sin(frame_count * 0.1f) * voltage_fluctuation);
    pack.soh = current_soh;

    pack.soc = 75.0f + std::sin(frame_count * 0.05f) * 10.0f;
    pack.soc = std::clamp(pack.soc, 5.0f, 95.0f);

    // 只生成物理量，SOC由EKF计算
    /* pack.total_voltage = base_voltage + (std::sin(frame_count * 0.1f) * voltage_fluctuation); */
    pack.total_current = current;
    pack.max_cell_voltage = max_cell_v;
    pack.min_cell_voltage = min_cell_v;
    pack.max_temperature = max_temp;
    pack.min_temperature = min_temp;

    pack.charging_status = (current < 0) ? "SlowCharging" : "Discharging";

    pack.cells.resize(num_cells);
    for (int i = 0; i < num_cells; ++i) {
        pack.cells[i].id = i + 1;
        pack.cells[i].voltage = simulateCellVoltage(i);
        pack.cells[i].temperature = simulateTemperature(i);
    }

    return pack;
}

float DataSimulator::simulateCellVoltage(int cell_id) {
    return 3.45f + (std::sin(cell_id + frame_count * 0.05f) * 0.22f);
}

float DataSimulator::simulateTemperature(int cell_id) {
    return 32.0f + (cell_id % 8) * 1.2f + (std::cos(frame_count * 0.1f) * 4.0f);
}
