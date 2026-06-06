/*************************************************************************
 * * File Name: src/main.cpp
 * * Author: Lyon
 * * Mail: 786208769@qq.com
 * * Created Time: 一  6/ 1 16:08:25 2026
 * * 🍺🍺🍺 Function is: 
 * *************************************************************************/
// src/main.cpp
#include "data_simulator.h"
#include "bms_parser.h"
#include "logger.h"
#include "soc_ekf.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <csignal>
#include <atomic>

std::atomic<bool> g_running{true};

void signalHandler(int) {
    std::cout << "\n\n接收到终止信号，正在优雅退出..." << std::endl;
    g_running = false;
}

int main() {
    std::signal(SIGINT, signalHandler);   // 支持 Ctrl+C 退出

    std::cout << "=== EV BMS Battery Data Parser 已启动 ===\n" << std::endl;
    std::cout << "模拟车辆：合创Z03 510km版 (实际容量约36kWh, SOH≈57%)\n";
    std::cout << "UDP接收端口: 8888 | 按 Ctrl+C 退出\n\n";

    BMSParser parser;

    // 创建 EKF: 37KWh / 320V = 115.6Ah 电池，SOH = 57%，50ms 一帧
    SocEKF ekf(115.6, 0.57f, 0.05f);

    if(!parser.loadProtocolConfig("../config/bms_protocol.json"))
    {
        std::cerr << "❌ 配置加载失败，程序退出！" << std::endl;
        return 1;
    };

    DataSimulator simulator;            // 创建模拟器
    Logger logger("bms_log.csv");       // 创建日志记录器

    // 启动工业级UDP接收
    parser.startUdpReceiver(8888);

    int frameCount = 0;

    std::cout << "开始运行... (模拟数据 + UDP接收同时进行)\n" << std::endl;

    auto next_wake = std::chrono::steady_clock::now();
    while (g_running) {
        // 模拟器生成数据
        BatteryPack rawData = simulator.generateFrame();    // 生成模拟数据
        BatteryPack processed = parser.parseFrame(rawData); // 解析处理
        
        parser.updateBatteryData(processed);                // 更新到管理器
                                                            
        /* processed.soc = ekf.update (processed.total_current, processed.total_voltage); */

        logger.writeFrame(processed);                       // 记录日志

        // 每25帧打印一次模拟数据
        if (frameCount % 25 == 0) {

            processed.soc = ekf.update(processed.total_current, 
                                       processed.total_voltage,
                                       processed.temperature);

            auto time_t = std::chrono::system_clock::to_time_t(processed.timestamp);
            std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
                << "SOC: " << std::fixed << std::setprecision(1) << processed.soc << "% | "
                << "SOH: " << processed.soh << "% | "
                << "电压: " << processed.total_voltage << "V | "
                << "电流: " << processed.total_current << "A | "
                << "续航: " << processed.soc * 2.35f << "km |"
                << "历史记录数：" << parser.getHistoryData().size() << std::endl;
        }

        frameCount++;

        // 定时唤醒
        next_wake += std::chrono::milliseconds(50);
        std::this_thread::sleep_until(next_wake);
        /* std::this_thread::sleep_for(std::chrono::milliseconds(50));    //20Hz */ 
    }

    parser.stopUdpReceiver();       // 退出前停止UDP
    std::cout << "\n=== 程序已退出 ===\n";
    std::cout << "日志已保存至 bms_log.csv\n";
    return 0;
}
