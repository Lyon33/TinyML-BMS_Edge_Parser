/*************************************************************************
* File Name: include/battery_data.h
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 15:58:06 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// include/battery_data.h
#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <optional>

struct CellData {
    int id = 0;
    float voltage = 0.0f;     // 单体电压 (V)
    float temperature = 0.0f; // 温度 (°C)
};

struct BatteryPack {
    std::chrono::system_clock::time_point timestamp;
    
    float total_voltage = 0.0f;      // 总电压 (V)
    float total_current = 0.0f;      // 总电流 (A)
    float soc = 0.0f;                // SOC (%)
    float soh = 0.0f;                // SOH (%)
    
    float max_cell_voltage = 0.0f;
    float min_cell_voltage = 0.0f;
    float max_temperature = 0.0f;
    float min_temperature = 0.0f;
    
    std::vector<CellData> cells;
    std::string charging_status;     // "FastCharging", "SlowCharging", "Idle"
    std::vector<std::string> faults; // 故障码
    
    // 你的车相关
    float estimated_range = 0.0f;    // 预估续航 (km)
};

class BatteryData {
public:
    void update(const BatteryPack& pack);
    const BatteryPack& getLatest() const;
    std::vector<BatteryPack> getHistory() const;
    
private:
    BatteryPack latest;
    std::vector<BatteryPack> history;
};
