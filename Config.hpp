// Config.hpp
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Aimbot.hpp"
#include "Camera.hpp"
#include "Glow.hpp"

class Config {
public:
    static Config& GetInstance() {
        static Config instance;
        return instance;
    }

    // Method to initialize the Config instance.
    void Initialize(const std::string& filePath, Aimbot* aimAssistance, Camera* gameCamera, Sense* esp) {
        if (!initialized) {
            configFilePath = filePath;
            AimAssistance = aimAssistance;
            GameCamera = gameCamera;
            ESP = esp;

            Update();
            initialized = true;
        }
    }

    void Update() {
        std::ifstream configFile(configFilePath);
        std::string line;

        if (!configFile.is_open()) {
            std::cerr << "Failed to open config file: " << configFilePath << std::endl;
            return;
        }

        while (getline(configFile, line)) {
            std::istringstream lineStream(line);
            std::string key, value;
            if (getline(lineStream, key, '=') && getline(lineStream, value)) {
                updateVariable(key, value);
            }
        }
    }

    void Save() {
        std::ofstream configFile(configFilePath);

        if (!configFile.is_open()) {
            std::cerr << "无法打开配置文件：" << configFilePath << std::endl;
            return;
        }

        configFile << u8"磁性自瞄=" << (AimAssistance->Sticky ? "true" : "false") << std::endl;
        configFile << u8"瞄准范围=" << AimAssistance->FOV << std::endl;
        configFile << u8"瞄准平滑=" << AimAssistance->Smooth << std::endl;
        configFile << u8"瞄准平滑最大增加量=" << AimAssistance->MaxSmoothIncrease << std::endl;
        configFile << u8"瞄准后坐力补偿=" << AimAssistance->RecoilCompensation << std::endl;
        configFile << u8"自瞄键=" << AimAssistance->AimBotKey << std::endl;
        configFile << u8"自瞄键2=" << AimAssistance->AimBotKey2 << std::endl;
        configFile << u8"扳机键=" << AimAssistance->AimTriggerKey << std::endl;
        configFile << u8"甩枪键=" << AimAssistance->AimFlickKey << std::endl;
        configFile << u8"物品发光=" << (ESP->ItemGlow ? "true" : "false") << std::endl;
        configFile << u8"物品发光等级=" << ESP->MinimumItemRarity << std::endl;
        configFile << u8"不可见发光颜色R=" << ESP->rnot << std::endl;
        configFile << u8"不可见发光颜色G=" << ESP->gnot << std::endl;
        configFile << u8"不可见发光颜色B=" << ESP->bnot << std::endl;
        configFile << u8"可见发光颜色R=" << ESP->rviz << std::endl;
        configFile << u8"可见发光颜色G=" << ESP->gviz << std::endl;
        configFile << u8"可见发光颜色B=" << ESP->bviz << std::endl;
        configFile << u8"权限发光颜色R=" << ESP->radm << std::endl;
        configFile << u8"权限发光颜色G=" << ESP->gadm << std::endl;
        configFile << u8"权限发光颜色B=" << ESP->badm << std::endl;
        configFile << u8"填充风格=" << ESP->insidetype << std::endl;
        configFile << u8"轮廓风格=" << ESP->outlinetype << std::endl;
        configFile << u8"轮廓大小=" << ESP->outlinesize << std::endl;
        configFile << u8"屏幕分辨率X=" << GameCamera->ScreenSize.x << std::endl;
        configFile << u8"屏幕分辨率Y=" << GameCamera->ScreenSize.y << std::endl;
        configFile << u8"游戏视野=" << GameCamera->FOV << std::endl;
        configFile << u8"Kmbox类型=" << AimAssistance->KmboxType << std::endl;
        configFile << u8"KmboxIP地址=" << AimAssistance->KmboxIP << std::endl;
        configFile << u8"Kmbox端口=" << AimAssistance->KmboxPort << std::endl;
        configFile << u8"KmboxUUID=" << AimAssistance->KmboxUUID << std::endl;
        configFile << u8"KmboxCOM端口=" << AimAssistance->KmboxComPort << std::endl;
        configFile << u8"波特率=" << AimAssistance->BaudRate << std::endl;
    }

private:
    std::string configFilePath;
    Aimbot* AimAssistance = nullptr;
    Camera* GameCamera = nullptr;
    Sense* ESP = nullptr;
    bool initialized = false;

    // Private constructor
    Config() {}

    // Existing methods...

    // Delete copy constructor and assignment operator
    Config(Config const&) = delete;
    void operator=(Config const&) = delete;

    void parseColorFromString(const std::string& value, std::array<float, 3>& colorArray) {
        std::stringstream ss(value);
        std::string item;
        int index = 0;
        while (std::getline(ss, item, ',') && index < 3) {
            colorArray[index++] = std::stof(item);
        }
    }

    void updateVariable(const std::string& key, const std::string& value) {
        if (key == u8"磁性自瞄") {
            if (value == "true") {
                AimAssistance->Sticky = true;
            }
            else if (value == "false") {
                AimAssistance->Sticky = false;
            }
		}
        if (key == u8"瞄准范围") {
			AimAssistance->FOV = std::stof(value);
		}
        if (key == u8"瞄准平滑") {
            AimAssistance->Smooth = std::stof(value);
        }
        if (key == u8"瞄准平滑最大增加量") {
            AimAssistance->MaxSmoothIncrease = std::stof(value);
		}
        if (key == u8"瞄准后坐力补偿") {
            AimAssistance->RecoilCompensation = std::stof(value);
        }
        if (key == u8"自瞄键") {
			AimAssistance->AimBotKey = std::stoi(value);
		}
        if (key == u8"自瞄键2") {
            AimAssistance->AimBotKey2 = std::stoi(value);
        }
		if (key == u8"扳机键") {
			AimAssistance->AimTriggerKey = std::stoi(value);
		}
        if (key == u8"甩枪键") {
            AimAssistance->AimFlickKey = std::stoi(value);
        }
		if (key == u8"物品发光") {
            if (value == "true") {
                ESP->ItemGlow = true;
            }
            else if (value == "false") {
                ESP->ItemGlow = false;
            }
		}
        if (key == u8"物品发光等级") {
			ESP->MinimumItemRarity = std::stoi(value);
		}
        if (key == u8"不可见发光颜色R") {
            ESP->rnot = std::stoi(value);
        }
        if (key == u8"不可见发光颜色G") {
            ESP->gnot = std::stoi(value);
        }
        if (key == u8"不可见发光颜色B") {
            ESP->bnot = std::stoi(value);
        }
        if (key == u8"可见发光颜色R") {
            ESP->rviz = std::stoi(value);
        }
        if (key == u8"可见发光颜色G") {
            ESP->gviz = std::stoi(value);
        }
        if (key == u8"可见发光颜色B") {
            ESP->bviz = std::stoi(value);
        }
        if (key == u8"权限发光颜色R") {
            ESP->radm = std::stoi(value);
        }
        if (key == u8"权限发光颜色G") {
            ESP->gadm = std::stoi(value);
        }
        if (key == u8"权限发光颜色B") {
            ESP->badm = std::stoi(value);
        }
        if (key == u8"填充风格") {
            ESP->insidetype = std::stoi(value);
        }
        if (key == u8"轮廓风格") {
            ESP->outlinetype = std::stoi(value);
        }
        if (key == u8"轮廓大小") {
            ESP->outlinesize = std::stoi(value);
        }
        if (key == u8"屏幕分辨率X") {
			GameCamera->ScreenSize.x = std::stoi(value);
		}
        if (key == u8"屏幕分辨率Y") {
            GameCamera->ScreenSize.y = std::stoi(value);
        }
        if (key == u8"游戏视野") {
			GameCamera->FOV = std::stof(value);
		}
        if (key == u8"Kmbox类型") {
			AimAssistance->KmboxType = value;
		}
        if (key == u8"KmboxIP地址") {
            // Ensure we don't exceed the buffer size, including space for the null terminator
            std::strncpy(AimAssistance->KmboxIP, value.c_str(), sizeof(AimAssistance->KmboxIP) - 1);
            // Ensure null termination
            AimAssistance->KmboxIP[sizeof(AimAssistance->KmboxIP) - 1] = '\0';
        }
        if (key == u8"Kmbox端口") {
			// Ensure we don't exceed the buffer size, including space for the null terminator
            std::strncpy(AimAssistance->KmboxPort, value.c_str(), sizeof(AimAssistance->KmboxPort) - 1);
			// Ensure null termination
			AimAssistance->KmboxPort[sizeof(AimAssistance->KmboxPort) - 1] = '\0';
		}
        if (key == u8"KmboxUUID") {
			// Ensure we don't exceed the buffer size, including space for the null terminator
			std::strncpy(AimAssistance->KmboxUUID, value.c_str(), sizeof(AimAssistance->KmboxUUID) - 1);
			// Ensure null termination
			AimAssistance->KmboxUUID[sizeof(AimAssistance->KmboxUUID) - 1] = '\0';
        }
        if (key == u8"KmboxCOM端口") {
            AimAssistance->KmboxComPort = std::stoi(value);
        }
        if (key == u8"波特率") {
            AimAssistance->BaudRate = std::stoi(value);
        }
    }
};
