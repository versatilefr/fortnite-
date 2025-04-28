#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <cmath>

#define UWorld 0x169641B8       // Your UWorld offset
#define PlayerArray 0x2C0       // Player list offset
#define Mesh 0x328              // Mesh offset
#define BoneArray 0x5A8         // Bone array offset
#define COMPONENT_TO_WORLD 0x1E0 // World position offset
#define SCREEN_WIDTH 1920       // Screen width (adjust as per your resolution)
#define SCREEN_HEIGHT 1080      // Screen height (adjust as per your resolution)

#define TEAM_INDEX 0x1259       // Team index offset
#define DISPLAY_NAME 0x40       // Display name offset

// Struct to hold bone info
struct Bone {
    float x, y, z;
};

// Utility function to read memory (simplified)
template<typename T>
T ReadMemory(uintptr_t address) {
    T value;
    ReadProcessMemory(GetCurrentProcess(), (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
}

// Utility function to write memory (simplified)
template<typename T>
void WriteMemory(uintptr_t address, T value) {
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &value, sizeof(T), nullptr);
}

// Function to get the process ID by the window title
DWORD GetProcessIdByWindowName(const std::wstring& windowName) {
    HWND hwnd = FindWindowW(NULL, windowName.c_str());
    if (!hwnd) return 0;

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

// Function to get the closest player (in this case, using the head bone)
uintptr_t GetClosestPlayer(uintptr_t playerArray, float* myPosition) {
    float closestDist = FLT_MAX;
    uintptr_t closestPlayer = 0;

    for (int i = 0; i < 50; ++i) {  // Assuming 50 players max
        uintptr_t playerBase = ReadMemory<uintptr_t>(playerArray + i * sizeof(uintptr_t));
        if (playerBase == 0) continue;

        uintptr_t meshPtr = ReadMemory<uintptr_t>(playerBase + Mesh);
        uintptr_t boneArray = ReadMemory<uintptr_t>(meshPtr + BoneArray);
        
        Bone headBone = ReadMemory<Bone>(boneArray + 67 * sizeof(Bone));  // Head is usually bone ID 67
        
        float screenPos[2];
        float worldPos[3] = { headBone.x, headBone.y, headBone.z };
        float matrix[16];  // You'll need to read the view matrix (or use your camera info)
        
        if (WorldToScreen(worldPos, screenPos, matrix)) {
            float dist = sqrt(pow(screenPos[0] - SCREEN_WIDTH / 2, 2) + pow(screenPos[1] - SCREEN_HEIGHT / 2, 2));
            if (dist < closestDist) {
                closestDist = dist;
                closestPlayer = playerBase;
            }
        }
    }
    return closestPlayer;
}

// Function to move the mouse to a given screen position
void MoveMouse(int x, int y) {
    SetCursorPos(x, y);  // Set the mouse position
}

// Function to convert world position to 2D screen position (simplified)
bool WorldToScreen(const float* worldPosition, float* screenPosition, float* matrix) {
    // Multiply world position by the view matrix (simplified)
    screenPosition[0] = matrix[0] * worldPosition[0] + matrix[1] * worldPosition[1] + matrix[2] * worldPosition[2] + matrix[3];
    screenPosition[1] = matrix[4] * worldPosition[0] + matrix[5] * worldPosition[1] + matrix[6] * worldPosition[2] + matrix[7];
    return true;
}

// Function to calculate the angle between two points (simplified for 2D)
float CalculateAngle(float x1, float y1, float x2, float y2) {
    return atan2(y2 - y1, x2 - x1) * (180.0f / 3.14159265358979323846f);  // Angle in degrees
}

// Main aimbot function
void Aimbot(uintptr_t uWorld, uintptr_t gameState, uintptr_t playerArray) {
    // Get my own position (this is an example, you'll need to get your player base)
    uintptr_t localPlayer = ReadMemory<uintptr_t>(uWorld + 0x38);  // LocalPlayer
    uintptr_t meshPtr = ReadMemory<uintptr_t>(localPlayer + Mesh);
    uintptr_t boneArray = ReadMemory<uintptr_t>(meshPtr + BoneArray);
    Bone myHead = ReadMemory<Bone>(boneArray + 67 * sizeof(Bone));  // Local player head position

    // Get the closest player to aim at
    uintptr_t closestPlayer = GetClosestPlayer(playerArray, &myHead.x);

    if (closestPlayer != 0) {
        // Get the position of the closest player's head
        uintptr_t meshPtr = ReadMemory<uintptr_t>(closestPlayer + Mesh);
        uintptr_t boneArray = ReadMemory<uintptr_t>(meshPtr + BoneArray);
        Bone headBone = ReadMemory<Bone>(boneArray + 67 * sizeof(Bone));  // Head is usually bone ID 67

        // Calculate the angle to the head position
        float screenPos[2];
        float worldPos[3] = { headBone.x, headBone.y, headBone.z };
        float matrix[16];  // Get view matrix

        if (WorldToScreen(worldPos, screenPos, matrix)) {
            // Calculate angle between the crosshair (screen center) and the target
            float angle = CalculateAngle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, screenPos[0], screenPos[1]);

            // Move mouse to the target (simplified for now)
            MoveMouse(screenPos[0], screenPos[1]);
        }
    }
}

// Function to draw ESP boxes (simplified)
void DrawESP(uintptr_t uWorld, uintptr_t gameState, uintptr_t playerArray) {
    for (int i = 0; i < 50; ++i) {  // Assuming 50 players max
        uintptr_t playerBase = ReadMemory<uintptr_t>(playerArray + i * sizeof(uintptr_t));
        if (playerBase == 0) continue;

        uintptr_t meshPtr = ReadMemory<uintptr_t>(playerBase + Mesh);
        uintptr_t boneArray = ReadMemory<uintptr_t>(meshPtr + BoneArray);

        Bone headBone = ReadMemory<Bone>(boneArray + 67 * sizeof(Bone));  // Head is usually bone ID 67
        
        float screenPos[2];
        float worldPos[3] = { headBone.x, headBone.y, headBone.z };
        float matrix[16];  // Get view matrix

        if (WorldToScreen(worldPos, screenPos, matrix)) {
            // Draw a simple box around the player (this is a simplified version)
            // You would replace this with actual drawing logic (like using DirectX or OpenGL to draw)
            std::cout << "Player at: " << screenPos[0] << ", " << screenPos[1] << std::endl;
        }
    }
}

// Function to inject the aimbot and ESP code
void InjectAimbotAndESP(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process!" << std::endl;
        return;
    }

    // Allocate memory for the aimbot and ESP code in the game process
    LPVOID allocatedMemory = VirtualAllocEx(hProcess, NULL, sizeof(Aimbot), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!allocatedMemory) {
        std::cerr << "Failed to allocate memory in target process!" << std::endl;
        return;
    }

    // Write the aimbot and ESP code into the allocated memory
    BOOL success = WriteProcessMemory(hProcess, allocatedMemory, (LPVOID)&Aimbot, sizeof(Aimbot), NULL);
    if (!success) {
        std::cerr << "Failed to write memory!" << std::endl;
        return;
    }

    // Create a remote thread to run the aimbot and ESP code
    CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)allocatedMemory, NULL, 0, NULL);

    std::cout << "Aimbot and ESP injected!" << std::endl;
    CloseHandle(hProcess);
}

int main() {
    std::wstring gameWindowName = L"GameName";  // Replace with your game's window name
    DWORD pid = GetProcessIdByWindowName(gameWindowName);
    if (pid == 0) {
        std::cerr << "Game window not found!" << std::endl;
        return 1;
    }

    uintptr_t uWorld = ReadMemory<uintptr_t>(UWorld);
    uintptr_t gameState = ReadMemory<uintptr_t>(uWorld + 0x1A0);  // GameState
    uintptr_t playerArray = ReadMemory<uintptr_t>(gameState + PlayerArray);

    // Inject the code
    InjectAimbotAndESP(pid);

    // Main game loop for aimbot and ESP
    while (true) {
        Aimbot(uWorld, gameState, playerArray);  // Aimbot function
        DrawESP(uWorld, gameState, playerArray); // ESP function
        Sleep(1);  // Limit CPU usage
    }

    return 0;
}
