/*************************************************************************
* File Name: src/bms_parser.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:13:20 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// src/bms_parser.cpp
#include "bms_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

BMSParser::BMSParser() {
    // 可以在这里注册一些默认处理规则
}

BatteryPack BMSParser::parseFrame(const BatteryPack& rawData) {
    BatteryPack processed = rawData;

    validateData(processed);

    // 异常检测
    auto faults = detectFaults(processed);
    processed.faults.insert(processed.faults.end(), faults.begin(), faults.end());

    // 根据SOH估算续航（模拟你的车）
    if (processed.soh > 0) {
        processed.estimated_range = 510.0f * (processed.soh / 100.0f) * 0.85f + 30.f;  // 510km是原续航
    }

    return processed;
}

void BMSParser::validateData(BatteryPack& pack) {
    // 合理性校验
    if (pack.total_voltage < 200.0f || pack.total_voltage > 420.0f) {
        pack.faults.push_back("P0A00_Voltage_OutOfRange");
    }
    if (pack.max_temperature > 55.0f) {
        pack.faults.push_back("P0A1F_OverTemperature");
    }
    if (pack.soc < 5.0f) {
        pack.faults.push_back("Low_SOC_Warning");
    }
}

std::vector<std::string> BMSParser::detectFaults(const BatteryPack& pack) {
    std::vector<std::string> faults;

    float voltage_diff = pack.max_cell_voltage - pack.min_cell_voltage;
    if (voltage_diff > 0.8f) {
        faults.push_back("P0A02_Cell_Voltage_Imbalance");
    }

    return faults;
}

void BMSParser::registerParser(uint32_t signalId, std::function<void(BatteryPack&, float)> handler) {
    customParsers[signalId] = handler;
}

// 新增JSON 转换
std::string BMSParser::toJson(const BatteryPack& pack) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\n";
    ss << "  \"timestamp\": \"" << std::chrono::system_clock::to_time_t(pack.timestamp) << "\",\n";
    ss << "  \"soc\": " << pack.soc << ",\n";
    ss << "  \"soh\": " << pack.soh << ",\n";
    ss << "  \"total_voltage\": " << pack.total_voltage << ",\n";
    ss << "  \"total_current\": " << pack.total_current << ",\n";
    ss << "  \"estimated_range\": " << pack.estimated_range << ",\n";
    ss << "  \"status\": \"" << pack.charging_status << "\",\n";
    ss << "  \"faults\": " << (pack.faults.empty() ? "[]" : "\"some faults\"") << "\n";
    ss << "}";
    return ss.str();
}
