/*************************************************************************
* File Name: config_manager.h
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 六  6/ 6 12:14:46 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
#pragma once
#include "json.hpp"
#include <string>
#include <iostream>

class ConfigManager {
public:
    static ConfigManager& instance();
    bool load(const std::string& path);
    nlohmann::json get() const;

private:
    ConfigManager() = default;
    nlohmann::json config_;
};
