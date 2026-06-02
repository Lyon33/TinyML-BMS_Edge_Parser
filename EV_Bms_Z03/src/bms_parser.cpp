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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

BMSParser::BMSParser() {
    // 可以在这里注册一些默认处理规则
}

BMSParser::~BMSParser(){
    stopUdpReceiver();
}

// ========================= 核心解析 =========================
BatteryPack BMSParser::parseFrame(const BatteryPack& rawData) {
    BatteryPack processed = rawData;

    validateData(processed);

    auto faults = detectFaults(processed);
    processed.faults.insert(processed.faults.end(), faults.begin(), faults.end());

    // 基于实际容量计算续航
    if (processed.soh > 0.0f) {

        processed.estimated_range = 36.0f * (processed.soh / 100.0f) * 6.8f;
        /* 当 SOH = 57% 时：
        // 36 × 0.57 × 6.8 ≈ 139.5 km
        // 所以不管 SOC 如何变化，续航只会跟着 SOH 变化，而模拟器里
        // SOH 目前是固定57%，因此续航就会一直显示139km。
        */
    }
    return processed;
}

    // 增加了 JSON 加载（改进版）
bool BMSParser::loadProtocolConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cout << "警告：无法加载配置文件 " << configPath << std::endl;
        return false;
    }

    nlohmann::json j;
    file >> j;

    std::cout << "✅ 已成功加载配置协议: " << configPath << std::endl;
    std::cout << "   车辆型号: " << j["vehicle_model"] << std::endl;
    std::cout << "   电池容量: " << j["nominal_capacity_kwh"] << "kWh" << std::endl;
    std::cout << "   标称续航里程: " << j["original_range_km"] << "km" << std::endl;
    std::cout << "   现实续航里程: " << j["current_range_km"] << "km" << std::endl;
    return true;
}

std::string BMSParser::getVehicleInfo() const {
        return "合创Z03 510km版 | 实际容量: 36kWh | SOH≈57%";
}

void BMSParser::validateData(BatteryPack& pack) {
    pack.faults.clear();
    if (pack.total_voltage < 200.0f || pack.total_voltage > 420.0f) {
        pack.faults.push_back("P0A00_Voltage_OutOfRange");
    }

    if (pack.max_temperature > 55.0f) {
        pack.faults.push_back("P0A1F_OverTemperature");
    }

    if (pack.soc < 8.0f) {
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

// ========================= 工业级UDP接收模块 =========================
bool BMSParser::startUdpReceiver(int port){
    if(udp_running) return true;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket < 0){
        std::cerr << "❌ 创建UDP Socket失败" << std::endl;
        return false;
    }

    int reuse = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0){
        std::cerr << "❌ UDP绑定端口 " << port << " 失败" << std::endl;
        close(udp_socket);
        return false;
    }

    udp_running = true;
    udp_thread = std::thread(&BMSParser::udpReceiveLoop, this, port);

    std::cout << "✅ 工业级UDP接收器已启动，监听端口: " << port << std::endl;
    std::cout << "   等待外部数据发送... (可使用Python/Netcat测试)" << std::endl;
    
    return true;
}

void BMSParser::stopUdpReceiver(){
    if (!udp_running) return;
    udp_running = false;

    if (udp_socket >= 0) {
        close(udp_socket);
        udp_socket = -1;
    }

    if (udp_thread.joinable()) {
        udp_thread.join();
    }
}

void BMSParser::udpReceiveLoop(int port) {
    uint8_t buffer[1024];
    sockaddr_in sender{};
    socklen_t slen = sizeof(sender);
    uint64_t packet_count = 0;
    auto last_print = std::chrono::steady_clock::now();
    auto last_heartbeat = std::chrono::steady_clock::now();

    while (udp_running) {
        ssize_t len = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (sockaddr*)&sender, &slen);

        if (len > 0) {
            packet_count++;
            last_heartbeat = std::chrono::steady_clock::now();

            BatteryPack pack = parseUdpData(buffer, len);
            BatteryPack processed = parseFrame(pack);

            if (packet_count % 8 == 0) {
                auto now = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_print).count();
                std::cout << "[UDP统计] 已接收 " << packet_count << " 包 | 速率 ≈ " 
                    << (8.0 / (duration > 0 ? duration : 1)) << "包/秒 | 来源: " 
                    << inet_ntoa(sender.sin_addr) << std::endl;
                last_print = now;
            }
        }
    }

    std::cout << "[UDP] 接收线程已正常退出" << std::endl;
}

BatteryPack BMSParser::parseUdpData(const uint8_t* buffer, size_t length) {
    BatteryPack pack;
    pack.timestamp = std::chrono::system_clock::now();

    // 简单二进制协议解析示例（工业常用格式）
    if (length >= 32) {
        pack.total_voltage = *(float*)(buffer + 0);
        pack.total_current = *(float*)(buffer + 4);
        pack.soc = *(float*)(buffer + 8);
        pack.soh = *(float*)(buffer + 12);
        pack.max_temperature = *(float*)(buffer + 16);
        pack.estimated_range = *(float*)(buffer + 20);
    } else {
        // 如果数据太短(或者收到空包、测试包时)，使用默认模拟值
        pack.total_voltage = 320.0f;
        pack.total_current = -30.0f;
        pack.soc = 65.0f;
        pack.soh = 57.0f;
        pack.max_temperature = 41.0f;
    }
    pack.charging_status = (pack.total_current < 0) ? "Charging" : "Discharging";
    return pack;
}
