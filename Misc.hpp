#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include "LocalPlayer.hpp"
#include "Offsets.hpp"
#include "DMALibrary/Memory/Memory.h"
#include "HidTable.hpp"
#include "Kmboxb.h"
#include "KmboxNet.hpp"
#include "Aimbot.hpp"

struct Misc {
    // Variables
    LocalPlayer* Myself;
	Aimbot* AimAssist;

	const int SuperGlideMinimumDelay = 7;

    Misc(LocalPlayer* Myself) {
        this->Myself = Myself;
    }

    void Initialize() {
        // idk, nothing for now
    }

    void Update() {
		while (mem.GetKeyboard()->IsKeyDown(0x43)) {
			static float startjumpTime = 0;
			static bool startSg = false;
			static float traversalProgressTmp = 0.0;

			float worldtime = mem.Read<float>(Myself->BasePointer + OFF_TIME_BASE); // Current time
			float traversalStartTime = mem.Read<float>(Myself->BasePointer + OFFSET_TRAVERSAL_START_TIME); // Time to start wall climbing
			float traversalProgress = mem.Read<float>(Myself->BasePointer + OFFSET_TRAVERSAL_PROGRESS); // Wall climbing, if > 0.87 it is almost over.
			auto HangOnWall = -(traversalStartTime - worldtime);

			if (HangOnWall > 0.1 && HangOnWall < 0.12)
			{
				if (AimAssist->KmboxType == u8"Net")
				{
					kmNet_keyup(KEY_SPACEBAR);
				}
			}
			if (traversalProgress > 0.87f && !startSg && HangOnWall > 0.1f && HangOnWall < 1.5f)
			{
				//start SG
				startjumpTime = worldtime;
				startSg = true;
			}
			if (startSg)
			{
				// Press Space
				if (AimAssist->KmboxType == u8"Net")
				{
					kmNet_keydown(KEY_SPACEBAR);
				}
				else if (AimAssist->KmboxType == u8"BPro")
				{
					char cmd[1024] = { 0 };
					sprintf_s(cmd, "km.press('spacebar')\r\n");
					Sleep((int)Utils::RandomRange(AimAssist->MinimumDelay, 10));
					AimAssist->comPort.write(cmd);
				}

				// Pressed Space, track time
				auto start = std::chrono::high_resolution_clock::now();
				if (mem.Read<float>(Myself->BasePointer + OFF_TIME_BASE) - startjumpTime < 0.011)
				{
					// Check Current Time
					auto now = std::chrono::high_resolution_clock::now();
					// Check Elapsed Time
					auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
					// Check if Elapsed Time is less than 4ms, then sleep for the remaining time to reach 4ms
					if (elapsed.count() < SuperGlideMinimumDelay) {
						std::this_thread::sleep_for(std::chrono::milliseconds(SuperGlideMinimumDelay - elapsed.count()));
						now = std::chrono::high_resolution_clock::now();
						elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
					}
					// Press Duck
					if (AimAssist->KmboxType == u8"Net")
					{
						kmNet_keydown(KEY_LEFTCONTROL);
					}
					else if (AimAssist->KmboxType == u8"BPro")
					{
						char cmd[1024] = { 0 };
						sprintf_s(cmd, "km.press('leftcontrol')\r\n");
						Sleep((int)Utils::RandomRange(AimAssist->MinimumDelay, 10));
						AimAssist->comPort.write(cmd);
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					// Release Space
					if (AimAssist->KmboxType == u8"Net")
					{
						kmNet_keyup(KEY_SPACEBAR);
					}
					else if (AimAssist->KmboxType == u8"BPro")
					{
						char cmd[1024] = { 0 };
						sprintf_s(cmd, "km.press('spacebar')\r\n");
						Sleep((int)Utils::RandomRange(AimAssist->MinimumDelay, 10));
						AimAssist->comPort.write(cmd);
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(600));
				}

				if (AimAssist->KmboxType == u8"Net")
				{
					kmNet_keydown(KEY_LEFTCONTROL);
				}
				else if (AimAssist->KmboxType == u8"BPro")
				{
					char cmd[1024] = { 0 };
					sprintf_s(cmd, "km.press('leftcontrol')\r\n");
					Sleep((int)Utils::RandomRange(AimAssist->MinimumDelay, 10));
					AimAssist->comPort.write(cmd);
				}
				startSg = false;
				break;
			}
		}
	}
};