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

struct CellData {             // 单个电芯的数据
    int id = 0;               // 第几号电芯
    float voltage = 0.0f;     // 单体电压 (V)
    float temperature = 0.0f; // 温度 (°C)
};

struct BatteryPack {                 // 电池包的数据
    std::chrono::system_clock::time_point timestamp; // 当前时间
    
    float total_voltage = 0.0f;      // 总电压 (V)
    float total_current = 0.0f;      // 当前电流 (A), 正=放电，负=充电
    float soc = 0.0f;                // 电量百分比SOC (%)
    float soh = 0.0f;                // 电池健康度SOH (%)
    
    float max_cell_voltage = 0.0f;   //最高单体电压
    float min_cell_voltage = 0.0f;   //最低单体电压
    float max_temperature = 0.0f;
    float min_temperature = 0.0f;
    
    std::vector<CellData> cells;     // 所有96个电芯的数据
    std::string charging_status;     // "FastCharging", "SlowCharging", "Idle"
    std::vector<std::string> faults; // 故障码列表
    
    float estimated_range = 0.0f;    // 预估续航 (km)
};

class BatteryData {
public:
    void update(const BatteryPack& pack);
    const BatteryPack& getLatest() const;
    std::vector<BatteryPack> getHistory() const;
    
private:
    BatteryPack latest;                 // 当前最新的电池数据
    std::vector<BatteryPack> history;   // 存放历史记录的
};
