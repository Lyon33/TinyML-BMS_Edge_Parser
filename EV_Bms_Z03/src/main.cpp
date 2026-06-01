/*************************************************************************
* File Name: src/main.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:08:25 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// src/main.cpp
#include "data_simulator.h"
#include "bms_parser.h"
#include "logger.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== EV BMS Battery Data Parser 已启动 ===\n" << std::endl;
    std::cout << "模拟车辆：合创Z03 510km版 (实际容量约36kWh, SOH≈57%)\n\n";

    DataSimulator simulator;
    BMSParser parser;
    Logger logger("bms_log.csv");

    int frameCount = 0;
    const int MAX_FRAMES = 300;   // 先跑300帧演示

    while (frameCount < MAX_FRAMES) {
        // 1. 生成模拟数据
        BatteryPack rawData = simulator.generateFrame();

        // 2. 解析处理
        BatteryPack processed = parser.parseFrame(rawData);

        // 3. 记录日志
        logger.writeFrame(processed);

        // 4. 控制台显示关键信息
        if (frameCount % 30 == 0) {   // 每30帧打印一次
            auto time_t = std::chrono::system_clock::to_time_t(processed.timestamp);
            std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
                      << "SOC: " << std::fixed << std::setprecision(1) << processed.soc << "% | "
                      << "SOH: " << processed.soh << "% | "
                      << "电压: " << processed.total_voltage << "V | "
                      << "电流: " << processed.total_current << "A | "
                      << "续航: " << (int)processed.estimated_range << "km | "
                      << "状态: " << processed.charging_status;

            if (!processed.faults.empty()) {
                std::cout << " | ⚠️ 故障: " << processed.faults[0];
            }
            std::cout << std::endl;
        }

        frameCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟100ms一帧
    }

    std::cout << "\n=== 测试结束 ===\n";
    std::cout << "日志已保存至 bms_log.csv\n";
    std::cout << "你可以基于此项目继续扩展功能。\n";

    return 0;
}
