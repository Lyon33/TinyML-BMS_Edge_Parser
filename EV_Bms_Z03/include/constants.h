/*************************************************************************
* File Name: constants.h
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 六  6/ 6 12:15:21 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
#pragma once
#include <string>

namespace bms {

constexpr float VOLTAGE_MIN = 200.0f;
constexpr float VOLTAGE_MAX = 420.0f;
constexpr float TEMP_MAX = 55.0f;
constexpr float CELL_VOLTAGE_DIFF_THRESHOLD = 0.8f;
constexpr int   MAX_HISTORY = 1000;
constexpr int   NUM_CELLS = 96;

constexpr uint32_t UDP_MAGIC = 0xB5A7C0DE;

} // namespace bms
