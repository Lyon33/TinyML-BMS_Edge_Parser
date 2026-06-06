// src/main.cpp
#include "data_simulator.h"
#include "bms_parser.h"
#include "logger.h"
#include "config_manager.h"
#include "constants.h"
#include "battery_data.h"
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
    std::signal(SIGINT, signalHandler);

    std::cout << "=== TinyML-BMS Edge Parser 已启动 ===\n" << std::endl;
    std::cout << "模拟车辆：合创Z03 510km版 (实际容量约36kWh, SOH≈57%)\n";
    std::cout << "UDP接收端口: 8888 | Ctrl+C 退出\n\n";

    BMSParser parser;
    std::string configPath = "../config/bms_protocol.json";
    /* parser.loadProtocolConfig("../config/bms_protocol.json"); */
    if(!parser.loadProtocolConfig(configPath)){
        std::cerr << "❌ 配置加载失败!\n";
        return 1;
    }

    DataSimulator simulator;
    Logger logger("bms_log.csv");
    parser.startUdpReceiver(8888);

    BatteryData historyRecorder;

    // ==================== EKF 核心 ====================
    SocEKF ekf(115.6f, 0.57f, 0.05f);

    int frameCount = 0;
    while (g_running) {

        BatteryPack rawData = simulator.generateFrame();

        // ==================== EKF 更新SOC（核心） ====================
        float soc = ekf.update(rawData.total_current, 
                               rawData.total_voltage,
                               rawData.max_temperature
        );

        rawData.soc = soc;

        BatteryPack processed = parser.parseFrame(rawData);

        // EKF 更新后再做最终校验
        if(processed.soc < 8.0f){
            processed.faults.push_back("Low_SOC_Warning");
        }

        parser.updateBatteryData(processed);
        logger.writeFrame(processed);

        //TinyML 推理先注释掉
        /* parser.runTinyMLInference(processed); */

        if (frameCount % 25 == 0) {
            std::cout << parser.toJson(processed) << "\n";  // 美观JSON输出
        }

        /* if (frameCount % 50 == 0) { */
        /*     std::cout << "[历史记录] 已保存 " << historyRecorder.getHistory().size() */ 
        /*         << " 帧数据" << std::endl; */
        /* } */

        frameCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    parser.stopUdpReceiver();
    std::cout << "\n=== 程序已退出 ===\n";
    return 0;
}
