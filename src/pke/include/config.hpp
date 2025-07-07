#pragma once
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace cfg {

    struct Config {
        bool injectError = false;
        int injectMode = 0;           // 0 = no injection, 1 = real, 2 = imag, 3 = both
        bool secretKeyAttack = false;
        int sdcThresholdBits = 5;     // Default: 5 bits from precision limit

        static Config Load() {
            Config cfg;
            std::ifstream in(defaultConfigPath());
            if (!in.is_open()) {
                std::cerr << "Warning: could not open config file: " << defaultConfigPath() << "\n";
                return cfg;
            }
            try {
                nlohmann::json j;
                in >> j;
                cfg.injectError      = j.value("injectError", 0) > 0;
                cfg.injectMode       = j.value("injectMode", 0);
                cfg.secretKeyAttack  = j.value("secretKeyAttack", 0) > 0;
                cfg.sdcThresholdBits = j.value("sdcThresholdBits", 5);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing config file: " << e.what() << "\n";
            }
            return cfg;
        }

        static std::filesystem::path defaultConfigPath() {
            if (const char* custom = std::getenv("CKKS_CONFIG_PATH")) {
                return std::filesystem::path(custom);
            }
            if (const char* h = std::getenv("HOME")) {
                return std::filesystem::path(h) / "ckksBitFlip" / "openfheBitFlip" / "config.json";
            }
            return std::filesystem::current_path() / "config.json";
        }
    };

    inline void logSDC(bool sdc) {
        std::filesystem::path path = Config::defaultConfigPath().parent_path() / "SKA_crash.txt";
        std::ofstream out(path, std::ios::app);
        if (!out) {
            std::cerr << "Error writing SDC log to " << path << "\n";
            return;
        }
        out << (sdc ? 1 : 0) << ", ";
    }

} // namespace cfg
