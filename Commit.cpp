#include <Windows.h>
#include <iostream>
#include <thread>
#include <atomic>

// === Offsets (YOUR GAME) ===
#define ROOT_COMPONENT 0x1B0
#define VELOCITY 0x180
#define LOCAL_PAWN 0x350
#define UWORLD 0x169641B8
#define GAME_INSTANCE 0x218
#define LOCAL_PLAYERS 0x38
#define PLAYER_CONTROLLER 0x30
#define PLAYER_STATE 0x2C8
#define TEAM_INDEX 0x1259
#define DISPLAY_NAME 0x40
#define MESH 0x328
#define BONE_ARRAY 0x5A8
#define BONE_ARRAY_CACHE 0x5B8
#define RELATIVE_LOCATION 0x138

// === Your Process and ReadMemory/WriteMemory functions ===
uintptr_t GetGameProcessID(const wchar_t* windowName);
uintptr_t GetModuleBaseAddress(uintptr_t procId, const wchar_t* modName);
template <typename T> T ReadMemory(uintptr_t address);
template <typename T> void WriteMemory(uintptr_t address, T value);

// === Global Flags ===
std::atomic<bool> espEnabled(false);
std::atomic<bool> aimbotEnabled(false);
std::atomic<bool> flyEnabled(false);
std::atomic<bool> rapidFireEnabled(false);

// === ESP Code ===
void ESP(uintptr_t baseAddress) {
    while (true) {
        if (!espEnabled) {
            Sleep(100);
            continue;
        }

        // Loop through all players in the game and draw ESP (simplified, depends on your game)
        // For now, you can implement drawing on screen based on player positions

        Sleep(10);
    }
}

// === Aimbot Code ===
void Aimbot(uintptr_t baseAddress) {
    while (true) {
        if (!aimbotEnabled) {
            Sleep(100);
            continue;
        }

        // Aimbot logic (simplified)
        // Find closest player and snap to head (this is where you will calculate bone position)
        // Implement aiming mechanics with memory write for rotation (yaw/pitch)

        Sleep(10);
    }
}

// === Fly Hack Code ===
void FlyVehicle(uintptr_t baseAddress) {
    while (true) {
        if (!flyEnabled) {
            Sleep(100);
            continue;
        }

        uintptr_t localPawn = GetLocalPawn(baseAddress);
        if (localPawn == 0) {
            Sleep(100);
            continue;
        }

        uintptr_t rootComponent = ReadMemory<uintptr_t>(localPawn + ROOT_COMPONENT);
        if (rootComponent == 0) {
            Sleep(100);
            continue;
        }

        float flySpeed = 1500.0f;
        float velocity[3] = { 0, 0, 0 };

        if (GetAsyncKeyState('W') & 0x8000) {
            velocity[1] += flySpeed; // Forward
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            velocity[1] -= flySpeed; // Backward
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            velocity[0] -= flySpeed; // Left
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            velocity[0] += flySpeed; // Right
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            velocity[2] += flySpeed; // Up
        }
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            velocity[2] -= flySpeed; // Down
        }

        WriteMemory(rootComponent + VELOCITY, velocity);

        Sleep(10);
    }
}

// === Rapid Fire Code ===
void RapidFire(uintptr_t baseAddress) {
    while (true) {
        if (!rapidFireEnabled) {
            Sleep(100);
            continue;
        }

        // Rapid fire logic (you would need to write to the weapon firing address)
        // For now, you can simulate the rapid fire by triggering weapon fire more frequently.

        Sleep(10);
    }
}

// === Get LocalPawn ===
uintptr_t GetLocalPawn(uintptr_t baseAddress) {
    uintptr_t uworld = ReadMemory<uintptr_t>(baseAddress + UWORLD);
    if (!uworld) return 0;

    uintptr_t gameInstance = ReadMemory<uintptr_t>(uworld + GAME_INSTANCE);
    uintptr_t localPlayers = ReadMemory<uintptr_t>(gameInstance + LOCAL_PLAYERS);
    uintptr_t playerController = ReadMemory<uintptr_t>(localPlayers + PLAYER_CONTROLLER);
    uintptr_t localPawn = ReadMemory<uintptr_t>(playerController + LOCAL_PAWN);

    return localPawn;
}

// === Main ===
int main() {
    const wchar_t* windowName = L"Fortnite"; // <<< Replace with your game's window name
    uintptr_t processID = GetGameProcessID(windowName);
    uintptr_t baseAddress = GetModuleBaseAddress(processID, L"FortniteClient-Win64-Shipping.exe"); // <<< Replace with your game exe

    if (processID == 0 || baseAddress == 0) {
        std::cout << "Game not found!" << std::endl;
        return 1;
    }

    std::cout << "Loaded! Press the keys to toggle features." << std::endl;

    // Start threads for each feature
    std::thread espThread(ESP, baseAddress);
    std::thread aimbotThread(Aimbot, baseAddress);
    std::thread flyThread(FlyVehicle, baseAddress);
    std::thread rapidFireThread(RapidFire, baseAddress);

    espThread.detach();
    aimbotThread.detach();
    flyThread.detach();
    rapidFireThread.detach();

    // Toggle controls
    while (true) {
        if (GetAsyncKeyState(VK_TAB) & 1) {
            espEnabled = !espEnabled;
            std::cout << (espEnabled ? "ESP Enabled!" : "ESP Disabled!") << std::endl;
        }
        if (GetAsyncKeyState(VK_F7) & 1) {
            aimbotEnabled = !aimbotEnabled;
            std::cout << (aimbotEnabled ? "Aimbot Enabled!" : "Aimbot Disabled!") << std::endl;
        }
        if (GetAsyncKeyState(VK_F2) & 1) {
            flyEnabled = !flyEnabled;
            std::cout << (flyEnabled ? "Fly Enabled!" : "Fly Disabled!") << std::endl;
        }
        if (GetAsyncKeyState(VK_CONTROL) & 1) {
            rapidFireEnabled = !rapidFireEnabled;
            std::cout << (rapidFireEnabled ? "Rapid Fire Enabled!" : "Rapid Fire Disabled!") << std::endl;
        }

        Sleep(50);  // Small delay to avoid high CPU usage
    }

    return 0;
}
