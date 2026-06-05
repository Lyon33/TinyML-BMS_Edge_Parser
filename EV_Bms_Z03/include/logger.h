/*************************************************************************
* File Name: include/logger.h
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:03:13 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// include/logger.h
#pragma once
#include "battery_data.h"
#include <string>
#include <fstream>

class Logger {
public:
    Logger(const std::string& filename = "bms_log.csv");
    ~Logger();

    void writeFrame(const BatteryPack& pack);
    void writeAlert(const std::string& message);

private:
    std::ofstream file;
    static bool first_write;
};
