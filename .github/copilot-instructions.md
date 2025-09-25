# Copilot Instructions for CHIP-8 Emulator

Welcome to the CHIP-8 Emulator project! These instructions are here to help Copilot and contributors generate consistent, high-quality code.

## General Guidelines

- **Primary Language:** C++ (use modern C++17 or higher; keep code idiomatic and use RAII where possible).
- **Graphics Library:** Use [Raylib](https://www.raylib.com/) for all rendering and input.
- **Build System:** Use CMake for builds and project structure.
- **Platform:** Target cross-platform compatibility (Windows, Linux, macOS).

## Code Style

- Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
- Use 4 spaces for indentation, no tabs.
- Name classes with `CamelCase` and variables/methods with `snake_case`.
- Keep functions concise and focused on a single responsibility.
- Prefer smart pointers over raw pointers.

## Emulator Logic

- Emulate CHIP-8 instructions accurately.
- Keep CPU, graphics, memory, and input logic modular (separate header/source files).
- Document tricky emulator logic with comments.

## Testing

- Add unit tests for new features and instructions, where practical.
- Prefer CMake-compatible test frameworks (e.g., Google Test).
- Place test files in a `/tests` directory.

## Pull Request Checklist

- Code compiles without errors or warnings.
- All tests pass.
- Code is formatted according to the style guide.
- Add/update documentation as needed.

## Copilot Prompts

- To add a new CHIP-8 instruction, prompt:  
  `Implement the opcode 0xFX65 (LD Vx, [I]) for the CHIP-8 CPU.`

- To add a new test, prompt:  
  `Write a Google Test case for the opcode 0x6XNN (LD Vx, NN).`

---

Thank you for contributing!