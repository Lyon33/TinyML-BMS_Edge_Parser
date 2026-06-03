/*************************************************************************
* File Name: src/battery_data.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 一  6/ 1 16:53:06 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
// src/battery_data.cpp
#include "battery_data.h"

void BatteryData::update(const BatteryPack& pack) {
    latest = pack;
    history.push_back(pack);
    if (history.size() > 1000) {
        history.erase(history.begin());
    }
}

const BatteryPack& BatteryData::getLatest() const {
    return latest;
}

std::vector<BatteryPack> BatteryData::getHistory() const {
    return history;
    std::unordered_map<uint32_t, std::function<void(BatteryPack&, float)>> customParsers;

}

