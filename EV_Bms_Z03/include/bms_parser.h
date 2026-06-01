/*************************************************************************
* File Name: include/bms_parser.h
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:12:42 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// include/bms_parser.h
#pragma once
#include "battery_data.h"
#include "data_simulator.h"
#include <functional>
#include <unordered_map>

class BMSParser {
public:
    BMSParser();

    // 解析一帧数据（当前主要处理模拟器生成的数据）
    BatteryPack parseFrame(const BatteryPack& rawData);

    // 注册自定义解析规则（可扩展）
    void registerParser(uint32_t signalId, std::function<void(BatteryPack&, float)> handler);

    // 异常检测
    std::vector<std::string> detectFaults(const BatteryPack& pack);

    // 新增：转换为JSON字符串（用于后续扩展）
    std::string toJson(const BatteryPack& pack);

    // JSON 配置
    bool loadProtocolConfig(const std::string& configPath);
    std::string getVehicleInfo() const;

private:
    std::unordered_map<uint32_t, std::function<void(BatteryPack&, float)>> customParsers;

    // 内部校验和处理
    void validateData(BatteryPack& pack);
};
