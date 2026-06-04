/*************************************************************************
 * * File Name: include/bms_parser.h
 * * Author: Lyon
 * * Mail: 786208769@qq.com
 * * Created Time: 一  6/ 1 16:12:42 2026
 * * 🍺🍺🍺 Function is: 
 * *************************************************************************/
// include/bms_parser.h
#pragma once
#include "battery_data.h"
#include "data_simulator.h"
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>

class BMSParser {
public:
    BMSParser();
    ~BMSParser();

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
    std::string getVehicleInfo() const;     // 返回车辆信息
                                            
    // BatteryData 相关接口
    void updateBatteryData(const BatteryPack& pack);
    const BatteryPack& getLatestData() const; 
    std::vector<BatteryPack> getHistoryData() const;

    // 新增：UDP接收
    bool startUdpReceiver(int port = 8888);
    void stopUdpReceiver();
    bool isUdpRunning() const;

    //新增CAN 协议解析
    BatteryPack parseCanFrame(const uint8_t* canData, uint8_t dlc, uint32_t canId);

    // 新增：故障日志上报（模拟低功耗场景）
    void reportFaultToCloud(const std::string& faultCode);

    // 新增：模拟TinyML推理（展示AI部署能力）
    float runTinyMLInference(const BatteryPack& pack);

private:
    // 内部存储自定义解析规则
    std::unordered_map<uint32_t, std::function<void(BatteryPack&, float)>> customParsers;

    // 内部校验和处理
    void validateData(BatteryPack& pack);

    BatteryData batteryData;

    // UDP 相关成员
    std::atomic<bool> udp_running{false};   // 原子变量，控制线程是否运行
    int udp_socket = -1;
    std::thread udp_thread;                 // UDP接收线程
    std::mutex data_mutex;                  // 互斥锁，保证线程安全

    std::atomic<uint64_t> received_packets{0};
    /* std::atomic<uint64_t> last_heartbeat{0}; */
    std::chrono::steady_clock::time_point last_heartbeat;
    const uint64_t HEARTBEAT_TIMEOUT_MS = 5000;

    void udpReceiveLoop(int port);
    BatteryPack parseUdpData(const uint8_t* buffer, size_t length);
};
