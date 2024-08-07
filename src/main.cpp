#include <iostream>
#include <vector>
#include <chrono>
#include <stdexcept>
#include "raylib.h"
#include "screen.hpp"
#include "chip8.hpp"
#include <getopt.h>

constexpr int TEXTURE_WIDTH = 64;
constexpr int TEXTURE_HEIGHT = 32;
constexpr int CPU_CLOCK_SPEED = 600; // 600 Hz 
constexpr float CYCLE_DURATION = 1000.0f / CPU_CLOCK_SPEED; // in milliseconds
constexpr int TIMER_FREQUENCY = 60; // 60 Hz
constexpr float TIMER_DURATION = 1000.0f / TIMER_FREQUENCY; // in milliseconds

using Clock = std::chrono::high_resolution_clock;
using Milliseconds = std::chrono::duration<float, std::chrono::milliseconds::period>;

void RunEmulationLoop(Screen& screen, Chip8Emulator::Chip8& chip8);
void HandleInput(Screen& screen, Chip8Emulator::Chip8& chip8, bool& shouldClose);
void UpdateTimers(Clock::time_point& lastTimerUpdateTime, Chip8Emulator::Chip8& chip8, Screen& screen);

int main(int argc, char* argv[])
{
    // Arguments - Default Values
    int screenWidth = 640;
    int screenHeight = 320;
    int framesPerSecond = 60;
    const char* romFilename = nullptr;
    
    // Command-line options
    static struct option long_options[] = {
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"fps", required_argument, 0, 'f'},
        {"rom", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "w:h:f:r:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'w':
                screenWidth = std::stoi(optarg);
                break;
            case 'h':
                screenHeight = std::stoi(optarg);
                break;
            case 'f':
                framesPerSecond = std::stoi(optarg);
                break;
            default:
                  std::cerr << "Usage: " << argv[0] << " [options] <ROM file>\n"
                          << "Options:\n"
                          << "  --width <width>     Set screen width (default: 640)\n"
                          << "  --height <height>   Set screen height (default: 320)\n"
                          << "  --fps <fps>         Set frames per second (default: 60)\n";
                return 1;
        }
    }

     // Get the ROM file from the last positional argument if not set by the --rom flag
    if (romFilename == nullptr && optind < argc) {
        romFilename = argv[optind];
    }

    if (romFilename == nullptr) {
        std::cerr << "ROM file is required. Usage: " << argv[0] << " [options] <ROM file>\n";
        return 1;
    }

    Screen screen("CHIP-8 Emulator", screenWidth, screenHeight, TEXTURE_WIDTH, TEXTURE_HEIGHT, framesPerSecond);
    Chip8Emulator::Chip8 chip8;

    try {
        chip8.LoadROM(romFilename);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load ROM: " << e.what() << "\n";
        return 1;
    }

    RunEmulationLoop(screen, chip8);
    return 0;
}

void RunEmulationLoop(Screen& screen, Chip8Emulator::Chip8& chip8)
{
    auto lastCycleTime = Clock::now();
    auto lastTimerUpdateTime = lastCycleTime;

    bool shouldClose = false;

    while (!shouldClose)
    {
        HandleInput(screen, chip8, shouldClose);

        auto currentTime = Clock::now();
        auto dt = std::chrono::duration_cast<Milliseconds>(currentTime - lastCycleTime).count();

        // Run multiple CPU cycles per frame to keep up with the desired CPU clock speed
        while (dt >= CYCLE_DURATION)
        {
            chip8.Cycle();
            dt -= CYCLE_DURATION;
            lastCycleTime += std::chrono::milliseconds(static_cast<int>(CYCLE_DURATION));
        }

        UpdateTimers(lastTimerUpdateTime, chip8, screen);

        screen.Update2DTexture(chip8.getDisplay());

        BeginDrawing();
        ClearBackground(BLACK);
        screen.DrawScaledTexture();
        EndDrawing();
    }
}

void HandleInput(Screen& screen, Chip8Emulator::Chip8& chip8, bool& shouldClose)
{
    if (WindowShouldClose() || screen.ProcessInput(chip8.getKeypad())) {
        shouldClose = true;
    }
}

void UpdateTimers(Clock::time_point& lastTimerUpdateTime, Chip8Emulator::Chip8& chip8, Screen& screen)
{
    auto currentTime = Clock::now();
    auto timerDt = std::chrono::duration_cast<Milliseconds>(currentTime - lastTimerUpdateTime).count();

    if (timerDt >= TIMER_DURATION)
    {
        chip8.DecrementTimers([&screen]() { screen.PlayBeep(); });
        lastTimerUpdateTime += std::chrono::milliseconds(static_cast<int>(TIMER_DURATION));
    }
}
