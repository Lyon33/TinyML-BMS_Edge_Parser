/*************************************************************************
 *   File Name: src/bms_parser.cpp
 *   Author: Lyon
 *   Mail: 786208769@qq.com
 *   Created Time: 一  6/ 1 16:13:20 2026
 **************************************************************************/
#include "bms_parser.h"
#include "json.hpp"
#include "constants.h"
#include "spdlog/spdlog.h"
#include "config_manager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>


BMSParser::BMSParser() {
        last_heartbeat = std::chrono::steady_clock::now();
        // 可以在这里注册一些默认处理规则
}

BMSParser::~BMSParser(){
        stopUdpReceiver();
}

const BatteryPack& BMSParser::getLatestData() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    return batteryData.getLatest();
}

std::vector<BatteryPack> BMSParser::getHistoryData() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    return batteryData.getHistory();
}

// =============== toJson =================
/* std::string BMSParser::toJson(const BatteryPack& pack) const { */
/*     nlohmann::json j; */
/*     j["timestamp"] = std::chrono::system_clock::to_time_t(pack.timestamp); */
/*     j["soc"] = pack.soc; */
/*     j["soh"] = pack.soh; */
/*     j["total_voltage"] = pack.total_voltage; */
/*     j["total_current"] = pack.total_current; */
/*     j["estimated_range"] = pack.estimated_range; */
/*     j["charging_status"] = pack.charging_status; */
/*     j["faults"] = pack.faults; */
/*     return j.dump(2);       //  缩进2格 */
/* } */

// ====================== 友好可读输出（替换原来的 toJson） ======================
std::string BMSParser::toJson(const BatteryPack& pack) const {
    std::ostringstream oss;

    auto time_t = std::chrono::system_clock::to_time_t(pack.timestamp);
    auto tm = *std::localtime(&time_t);

    // 工业级 static_cast<int> 转换(约束数据，边界管理)
    int range_km = static_cast<int>(std::round(pack.estimated_range));
    range_km = std::clamp(range_km, 0, 999);

    // ============== 时间戳对齐 ==================
    oss << "["
        << std::setfill('0')
        << std::setw(2) << tm.tm_hour << ":"
        << std::setw(2) << tm.tm_min << ":"
        << std::setw(2) << tm.tm_sec
        << "]";

    // ===== 数值：切回空格填充 =====
    oss << std::setfill(' ');
    
    // ===== SOC / SOH =====
    oss << "SOC: "
        << std::setw(6) << std::fixed << std::setprecision(2) << pack.soc << "% | "

        << "SOH: "
        << std::setw(5) << std::fixed << std::setprecision(2) << pack.soh << "% | "

        // ===== 电压 / 电流 =====
        << "电压: "
        << std::setw(6) << std::fixed << std::setprecision(1) << pack.total_voltage << "V | "

        << "电流: "
        << std::setw(6) << std::fixed << std::setprecision(1) << pack.total_current << "A | "

        // ===== 温度 =====
        << "温度: "
        << std::setw(5) << std::fixed << std::setprecision(1) << pack.max_temperature << "℃ | "

        // ===== 续航 =====
        << "续航: "
        << std::setw(3) << range_km << "km";

    // ===== 状态 =====
    if (!pack.charging_status.empty()) {
        oss << " | " << std::setw(12) << std::left << pack.charging_status;
    }

    // ===== 故障 =====
    if (!pack.faults.empty()) {
        oss << " | ⚠️ " << pack.faults[0];
    }

    return oss.str();
}

bool BMSParser::loadProtocolConfig(const std::string& configPath) {
        return ConfigManager::instance().load(configPath);
}

// ========================= 核心解析（完全保留你的原始实现） =========================
BatteryPack BMSParser::parseFrame(const BatteryPack& rawData) {
    BatteryPack processed = rawData;    //先复制一份原始数据

    validateData(processed);            // 先校验数据（此时SOC可能还未更新）
    auto faults = detectFaults(processed);  //检查有没有故障
    processed.faults.insert(processed.faults.end(), faults.begin(), faults.end());

    // 合法性保护（只限制范围，不破坏值）
    processed.soc = std::clamp(processed.soc, 0.0f, 100.0f);
    processed.soh = std::clamp(processed.soh, 0.0f, 100.0f);

    // 计算续航
    if (processed.soh > 0.0f) {
        float soh_factor = processed.soh / 100.0f;
        float soc_factor = processed.soc / 100.0f;
        float full_range = 510.0f * soh_factor * 0.92f;
        processed.estimated_range = full_range * soc_factor;
    }

    return processed;       // 返回处理后的数据
}

/* bool BMSParser::loadProtocolConfig(const std::string& configPath) { */
/*     std::ifstream file(configPath); */
/*     if (!file.is_open()) { */
/*         spdlog << "❌ 无法打开配置文件: " << configPath << std::endl; */
/*         return false; */
/*     } */
/*     try { */
/*         nlohmann::json j; */
/*         file >> j; */

/*         spdlog << "✅ 已成功加载配置协议: " << configPath << std::endl; */

/*         std::string vehicle = j.value("vehicle_model", std::string("未知车型")); */
/*         spdlog << "   车辆型号: " << vehicle << std::endl; */

/*         int capacity = j.value("nominal_capacity_kwh", 0); */
/*         spdlog << "   电池容量: " << capacity << "kWh" << std::endl; */

/*         int range_km = j.value("original_range_km", 0); */
/*         spdlog << "   标称续航: " << range_km << "km" << std::endl; */

/*         return true; */

/*     } catch (const std::exception& e) { */
/*         spdlog << "❌ 配置加载失败: " << e.what() << std::endl; */
/*         return false; */
/*     } */
/* } */

std::string BMSParser::getVehicleInfo() const {
    return "合创Z03 510km版 | 实际容量: 36kWh | SOH≈57%";
}

// 延迟 SOC 检查
void BMSParser::validateData(BatteryPack& pack) {
    pack.faults.clear();
    if (pack.total_voltage < bms::VOLTAGE_MIN || pack.total_voltage > bms::VOLTAGE_MAX) {
        pack.faults.push_back("P0A00_Voltage_OutOfRange");
    }
    if (pack.max_temperature > bms::TEMP_MAX) {
        pack.faults.push_back("P0A1F_OverTemperature");
    }
}

// 故障诊断函数（专门检查电芯之间是否平衡）
std::vector<std::string> BMSParser::detectFaults(const BatteryPack& pack) {
    std::vector<std::string> faults;
    float voltage_diff = pack.max_cell_voltage - pack.min_cell_voltage;
    if (voltage_diff > 0.8f) {              // 压差大于0.8V，记录故障
        faults.push_back("P0A02_Cell_Voltage_Imbalance");
    }
    return faults;
}

void BMSParser::registerParser(uint32_t signalId, std::function<void(BatteryPack&, float)> handler) {
    customParsers[signalId] = handler;
}

// ========================= UDP 接收模块（优化后） =========================
bool BMSParser::startUdpReceiver(int port){
    if(udp_running.load()) return true;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);    // 创建一个“收邮件的信箱”
    if(udp_socket < 0){
        spdlog::error("❌ 创建UDP Socket失败");
        return false;
    }

    // ⚠️  设置端口复用，防止端口被占用
    int reuse = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 绑定端口 8888
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0){
        spdlog::error("UDP绑定端口 {} 失败", port);
        close(udp_socket);
        udp_socket = -1;
        return false;
    }

    udp_running = true;
    // 启动一个新线程专门接收数据
    udp_thread = std::thread(&BMSParser::udpReceiveLoop, this, port);
    spdlog::info("✅ 工业级UDP接收器已启动，监听端口: {}", port);
    return true;
}

void BMSParser::stopUdpReceiver(){
    if (!udp_running.load()) return;
    udp_running = false;

    if (udp_socket >= 0) {
        close(udp_socket);
        udp_socket = -1;
    }

    if (udp_thread.joinable()) {
        udp_thread.join();
    }
    spdlog::info("🛑 UDP接收器已停止");
}

bool BMSParser::isUdpRunning() const {
        return udp_running.load();
}

// UDP 模块核心工作线程(循环接收数据)
void BMSParser::udpReceiveLoop(int port) {
    uint8_t buffer[1024];           // 准备一个1024字节的“收件箱”
    sockaddr_in sender{};
    socklen_t slen = sizeof(sender);
    uint64_t packet_count = 0;
    auto last_print = std::chrono::steady_clock::now();
    /* auto last_heartbeat = std::chrono::steady_clock::now(); */

    //设置非堵塞
    int flags = fcntl(udp_socket, F_GETFL, 0);
    fcntl(udp_socket, F_SETFL, flags | O_NONBLOCK);

    while (udp_running) {
        ssize_t len = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (sockaddr*)&sender, &slen);

        if (len > 0) {              // 如果收到数据
            packet_count++;
            last_heartbeat = std::chrono::steady_clock::now();
            auto data_span = std::span<const uint8_t>(buffer,len);
            BatteryPack pack = parseUdpData(data_span);   // 解析收到的二进制数据
            BatteryPack processed = parseFrame(pack);       // 处理数据

            updateBatteryData(processed);       // 线程安全更新

            // 打印统计信息（按时间，不再按包）
            /* static auto last_print = std::chrono::steady_clock::now(); */
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_print).count() >= 1) {
                std::string ip = inet_ntoa(sender.sin_addr);
                spdlog::info("[UDP统计] 已接收 {} 包 ｜速率 = {} 包/秒 ｜来源：{}", packet_count, packet_count, ip);

                packet_count = 0;
                last_print = now;
            }
        }
        else if(len < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            if(udp_running){    // 只有还在运行时打印错误
                spdlog::error("UDP接收失败：{}", strerror(errno));
            }
        }

        //心跳超时检测
        if (packet_count > 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>
                (now - last_heartbeat).count() > 8) {
                spdlog::info("[UDP警告] 心跳超时，可能数据源已断开");
            }
        }
        // 降低cpu 占用
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// 解析外部发来的UDP数据 (拆包)
BatteryPack BMSParser::parseUdpData(std::span<const uint8_t> buffer) {
    BatteryPack pack;
    pack.timestamp = std::chrono::system_clock::now();

    constexpr size_t KMinSize = 6 * sizeof(float);
    if (buffer.size() >= KMinSize) {
        auto read_float = [&](size_t offset) -> float {
            float value = 0.0f;
            if (offset + sizeof(float) <= buffer.size()) {
                std::memcpy(&value, buffer.data() + offset, sizeof(float));
            }
            return value;
        };

        pack.total_voltage    = read_float(0);
        pack.total_current    = read_float(4);
        pack.soc              = read_float(8);
        pack.soh              = read_float(12);
        pack.max_temperature  = read_float(16);
        pack.estimated_range  = read_float(20);

    } else {
        // 默认安全值
        pack.total_voltage = 320.0f;
        pack.total_current = -30.0f;
        pack.soc = 65.0f;
        pack.soh = 57.0f;
        pack.max_temperature = 41.0f;
    }

    pack.charging_status = (pack.total_current < 0) ? "Charging" : "Discharging";

    return pack;
}

// 新增 BatteryData接口实现
void BMSParser::updateBatteryData(const BatteryPack& pack) {
    std::lock_guard<std::mutex> lock(data_mutex);
    batteryData.update(pack);
}
