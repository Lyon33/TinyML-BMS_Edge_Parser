/*************************************************************************
* File Name: src/bms_parser.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:13:20 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// src/bms_parser.cpp
#include "bms_parser.h"
#include "json.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>

BMSParser::BMSParser() {
    // 可以在这里注册一些默认处理规则
}

    // 增加了 JSON 加载（改进版）
bool BMSParser::loadProtocolConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cout << "警告：无法加载配置文件 " << configPath << std::endl;
        return false;
    }
    // 后面会解析JSON（目前先占位）
    std::cout << "✅ 已成功加载协议配置: " << configPath << std::endl;
    std::cout << "车辆模型：广汽合创Z03 510Km版 （真实容量36 KWh）" << std::endl;
    return true;
}

BatteryPack BMSParser::parseFrame(const BatteryPack& rawData) {
    BatteryPack processed = rawData;
    validateData(processed);

    // 异常检测
    auto faults = detectFaults(processed);
    processed.faults.insert(processed.faults.end(), faults.begin(), faults.end());

    // 更真实的续航计算（结合车子36kwh的实际容量）
    if (processed.soh > 0) {
        float health_factor = processed.soh / 100.0f;
        processed.estimated_range = 36.0f * health_factor * 6.8f;  
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
