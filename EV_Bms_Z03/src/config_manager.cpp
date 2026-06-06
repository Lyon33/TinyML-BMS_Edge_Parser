/*************************************************************************
* File Name: config_manager.cpp
* Author: Lyon
* Mail: 786208769@qq.com
* Created Time: 六  6/ 6 12:20:09 2026
* 🍺🍺🍺 Function is: 
*************************************************************************/
#include "config_manager.h"
#include <fstream>

ConfigManager& ConfigManager::instance() {
    static ConfigManager inst;
    return inst;
}

bool ConfigManager::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    file >> config_;
    std::cout << "✅ 配置加载成功: " << path << std::endl;
    return true;
}

nlohmann::json ConfigManager::get() const {
    return config_;
}
