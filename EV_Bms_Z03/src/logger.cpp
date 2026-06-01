/*************************************************************************
* File Name: src/logger.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:03:46 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// src/logger.cpp
#include "logger.h"
#include <iostream>
#include <iomanip>

Logger::Logger(const std::string& filename) {
    file.open(filename, std::ios::app);
    if (first_write) {
        file << "Timestamp,SOC,SOH,TotalVoltage,TotalCurrent,MaxTemp,Range,Status,Faults\n";
        first_write = false;
    }
}

Logger::~Logger() {
    if (file.is_open()) file.close();
}

void Logger::writeFrame(const BatteryPack& pack) {
    auto time_t = std::chrono::system_clock::to_time_t(pack.timestamp);
    file << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << ","
         << pack.soc << "," << pack.soh << ","
         << pack.total_voltage << "," << pack.total_current << ","
         << pack.max_temperature << "," << pack.estimated_range << ","
         << pack.charging_status << ",";

    for (const auto& fault : pack.faults) {
        file << fault << ";";
    }
    file << "\n";
}

void Logger::writeAlert(const std::string& message) {
    std::cout << "[ALERT] " << message << std::endl;
}
