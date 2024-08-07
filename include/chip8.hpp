#ifndef CHIP8_H
#define CHIP8_H

#include <iostream>
#include <chrono>
#include <random>
#include <unordered_map>
#include <array>
#include <functional>
#include <set>

namespace Chip8Emulator{

constexpr unsigned int KEY_COUNT = 16;
constexpr unsigned int MEMORY_SIZE = 4096;
constexpr unsigned int REGISTER_COUNT = 16;
constexpr unsigned int STACK_LEVELS = 16;
constexpr unsigned int VIDEO_HEIGHT = 32;
constexpr unsigned int VIDEO_WIDTH = 64;
constexpr unsigned int START_ADDR = 0x200;
constexpr unsigned int FONTSET_START_ADDR = 0x50;

class Chip8 {
private:
    uint8_t m_Data[MEMORY_SIZE]{};
    uint8_t m_Register[REGISTER_COUNT]{};
    uint16_t m_IndexRegister{};
    uint16_t m_ProgramCounter{};
    uint8_t m_StackPointer{};
    uint16_t m_Stack[STACK_LEVELS]{};
    uint8_t m_DelayTimer{};
    uint8_t m_SoundTimer{};
    uint16_t m_Opcode{};
    uint8_t m_Keypad[KEY_COUNT]{};
    uint32_t m_Display[VIDEO_WIDTH * VIDEO_HEIGHT]{};
    std::default_random_engine m_RandGen;
    std::uniform_int_distribution<uint8_t> m_RandomByte;

    using OpcodeFunc = void (Chip8::*)();
    std::unordered_map<uint8_t, OpcodeFunc> m_OpcodeTable;
    std::unordered_map<uint8_t, OpcodeFunc> m_OpcodeTable0;
    std::unordered_map<uint8_t, OpcodeFunc> m_OpcodeTable8;
    std::unordered_map<uint8_t, OpcodeFunc> m_OpcodeTableE;
    std::unordered_map<uint8_t, OpcodeFunc> m_OpcodeTableF;

    void OP_00E0();
    void OP_00EE();
    void OP_1nnn();
    void OP_2nnn();
    void OP_3xkk();
    void OP_4xkk();
    void OP_5xy0();
    void OP_6xkk();
    void OP_7xkk();
    void OP_8xy0();
    void OP_8xy1();
    void OP_8xy2();
    void OP_8xy3();
    void OP_8xy4();
    void OP_8xy5();
    void OP_8xy6();
    void OP_8xy7();
    void OP_8xyE();
    void OP_9xy0();
    void OP_Annn();
    void OP_Bnnn();
    void OP_Cxkk();
    void OP_Dxyn();
    void OP_Ex9E();
    void OP_ExA1();
    void OP_Fx07();
    void OP_Fx0A();
    void OP_Fx15();
    void OP_Fx18();
    void OP_Fx1E();
    void OP_Fx29();
    void OP_Fx33();
    void OP_Fx55();
    void OP_Fx65();

    void Nibble0();
    void Nibble8();
    void NibbleE();
    void NibbleF();

    void InitializeOpcodeTable();
    void ExecuteOpcode(const std::unordered_map<uint8_t, OpcodeFunc>& byteMap, uint8_t opcode);
    void DecodeOpcode();
    void setOpcodeFunction(uint8_t opcode, void (Chip8::*func)());

public:
    Chip8();
    void LoadROM(const char* filename);
    void Cycle();
    void DecrementTimers(std::function<void()> beepCallback);

    uint8_t* getKeypad() { return m_Keypad; }
    uint32_t* getDisplay() { return m_Display; }

    ~Chip8() = default;
};

} // namespace Chip8Emulator

#endif // CHIP8_H
