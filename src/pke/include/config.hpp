#pragma once
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>


namespace cfg {

    struct Config {
        bool injectError = false;
        int injectMode = 0;           // 0 = no injection, 1 = real, 2 = imag, 3 = both
        bool secretKeyAttackDisable = false;
        int sdcThresholdBits = 5;     // Default: 5 bits from precision limit
        static Config Load() {
            Config cfg;

            std::ifstream in(defaultConfigPath());
            if (!in.is_open()) {
                std::cerr << "Warning: could not open config file: " << defaultConfigPath() << "\n";
                return cfg;
            }

            std::string line;
            while (std::getline(in, line)) {
                // Quitar comentarios
                size_t commentPos = line.find_first_of("#/");
                if (commentPos != std::string::npos)
                    line = line.substr(0, commentPos);

                // Quitar espacios
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);

                if (line.empty())
                    continue;

                // Parsear clave = valor
                size_t eqPos = line.find('=');
                if (eqPos == std::string::npos)
                    continue;

                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);

                // Quitar espacios extra
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                // Setear los valores
                if (key == "injectError")
                    cfg.injectError = std::stoi(value) > 0;
                else if (key == "injectMode")
                    cfg.injectMode = std::stoi(value);
                else if (key == "secretKeyAttackDisable")
                    cfg.secretKeyAttackDisable = std::stoi(value) > 0;
                else if (key == "sdcThresholdBits")
                    cfg.sdcThresholdBits = std::stoi(value);
            }

            return cfg;
        }
        
        static std::filesystem::path defaultConfigPath() {
            if (const char* custom = std::getenv("CKKS_CONFIG_PATH")) {
                return std::filesystem::path(custom)/ "config.conf";;
            }
            return std::filesystem::current_path() / "config.conf";
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
