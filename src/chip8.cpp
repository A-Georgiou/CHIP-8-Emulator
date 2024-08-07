#include "chip8.hpp"
#include "font.hpp"
#include <iostream>
#include <fstream>

namespace Chip8Emulator{

void Chip8::LoadROM(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; i++) {
            m_Data[START_ADDR + i] = buffer[i];
        }

        std::cout << "Loaded ROM size: " << size << " bytes" << std::endl;
        delete[] buffer;
    } else {
        throw  std::runtime_error("Failed to open ROM file.");
    }
}

Chip8::Chip8()
: m_ProgramCounter(START_ADDR),
  m_RandGen(std::chrono::system_clock::now().time_since_epoch().count()),
  m_RandomByte(std::uniform_int_distribution<uint8_t>(0, 255U))
{
    InitializeOpcodeTable();

    for (uint8_t i = 0; i < FONTSET_SIZE; ++i) {
        m_Data[FONTSET_START_ADDR + i] = fontset[i];
    }
}

void Chip8::OP_00E0(){
    memset(m_Display, 0, sizeof(m_Display));
}

void Chip8::OP_00EE(){
    m_ProgramCounter = m_Stack[--m_StackPointer];
}

void Chip8::OP_1nnn(){
    m_ProgramCounter = m_Opcode & 0x0FFF;
}

void Chip8::OP_2nnn(){
    m_Stack[m_StackPointer++] = m_ProgramCounter;
    m_ProgramCounter = m_Opcode & 0x0FFF;
}

void Chip8::OP_3xkk(){
    if (m_Register[(m_Opcode & 0x0F00) >> 8] == (m_Opcode & 0x00FF)) {
        m_ProgramCounter += 2;
    }
}

void Chip8::OP_4xkk(){
    if (m_Register[(m_Opcode & 0x0F00) >> 8] != (m_Opcode & 0x00FF)) {
        m_ProgramCounter += 2;
    }
}

void Chip8::OP_5xy0(){
    if (m_Register[(m_Opcode & 0x0F00) >> 8] == m_Register[(m_Opcode & 0x00F0) >> 4]) {
        m_ProgramCounter += 2;
    }
}

void Chip8::OP_6xkk(){
    m_Register[(m_Opcode & 0x0F00) >> 8] = m_Opcode & 0x00FF;
}

void Chip8::OP_7xkk(){
    m_Register[(m_Opcode & 0x0F00) >> 8] += m_Opcode & 0x00FF;
}

void Chip8::OP_8xy0(){
    m_Register[(m_Opcode & 0x0F00) >> 8] = m_Register[(m_Opcode & 0x00F0) >> 4];
}

void Chip8::OP_8xy1(){
    m_Register[(m_Opcode & 0x0F00) >> 8] |= m_Register[(m_Opcode & 0x00F0) >> 4];
}

void Chip8::OP_8xy2(){
    m_Register[(m_Opcode & 0x0F00) >> 8] &= m_Register[(m_Opcode & 0x00F0) >> 4];
}

void Chip8::OP_8xy3(){
    m_Register[(m_Opcode & 0x0F00) >> 8] ^= m_Register[(m_Opcode & 0x00F0) >> 4];
}

void Chip8::OP_8xy4(){
    uint16_t sum = m_Register[(m_Opcode & 0x0F00) >> 8] + m_Register[(m_Opcode & 0x00F0) >> 4];
    m_Register[(m_Opcode & 0x0F00) >> 8] = sum & 0xFF;
    m_Register[0xF] = sum > 0xFF;
}

void Chip8::OP_8xy5(){
    m_Register[0xF] = m_Register[(m_Opcode & 0x0F00) >> 8] > m_Register[(m_Opcode & 0x00F0) >> 4];
    m_Register[(m_Opcode & 0x0F00) >> 8] -= m_Register[(m_Opcode & 0x00F0) >> 4];
}

void Chip8::OP_8xy6(){
    m_Register[0xF] = m_Register[(m_Opcode & 0x0F00) >> 8] & 0x1;
    m_Register[(m_Opcode & 0x0F00) >> 8] >>= 1;
}

void Chip8::OP_8xy7(){
    m_Register[0xF] = m_Register[(m_Opcode & 0x00F0) >> 4] > m_Register[(m_Opcode & 0x0F00) >> 8];
    m_Register[(m_Opcode & 0x0F00) >> 8] = m_Register[(m_Opcode & 0x00F0) >> 4] - m_Register[(m_Opcode & 0x0F00) >> 8];
}

void Chip8::OP_8xyE(){
    m_Register[0xF] = m_Register[(m_Opcode & 0x0F00) >> 8] >> 7;
    m_Register[(m_Opcode & 0x0F00) >> 8] <<= 1;
}

void Chip8::OP_9xy0(){
    if (m_Register[(m_Opcode & 0x0F00) >> 8] != m_Register[(m_Opcode & 0x00F0) >> 4]) {
        m_ProgramCounter += 2;
    }
}

void Chip8::OP_Annn(){
    m_IndexRegister = m_Opcode & 0x0FFFu;
}

void Chip8::OP_Bnnn(){
    m_ProgramCounter = (m_Opcode & 0x0FFF) + m_Register[0];
}

void Chip8::OP_Cxkk(){
    m_Register[(m_Opcode & 0x0F00) >> 8] = m_RandomByte(m_RandGen) & (m_Opcode & 0x00FF);
}

void Chip8::OP_Dxyn(){
    uint8_t Vx = (m_Opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_Opcode & 0x00F0u) >> 4u;
	uint8_t height = m_Opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = m_Register[Vx] % VIDEO_WIDTH;
	uint8_t yPos = m_Register[Vy] % VIDEO_HEIGHT;

	m_Register[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = m_Data[m_IndexRegister + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &m_Display[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					m_Register[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_Ex9E(){
    if (m_Keypad[m_Register[(m_Opcode & 0x0F00) >> 8]]) {
        m_ProgramCounter += 2;
    }
}

void Chip8::OP_ExA1(){
    if (!m_Keypad[m_Register[(m_Opcode & 0x0F00) >> 8]]) {
        m_ProgramCounter += 2;
    }
}

void Chip8::OP_Fx07(){
    m_Register[(m_Opcode & 0x0F00) >> 8] = m_DelayTimer;
}

void Chip8::OP_Fx0A() {
    std::cout << "Waiting for key press..." << std::endl;
    bool keyPress = false;

    for (int i = 0; i < 16; i++) {
        if (m_Keypad[i] != 0) {
            std::cout << "Key pressed: " << i << std::endl;
            m_Register[(m_Opcode & 0x0F00u) >> 8u] = i;
            keyPress = true;
            break;
        }
    }

    if (!keyPress) {
        m_ProgramCounter -= 2;  // Ensure it doesn't move to the next instruction
    }
}

void Chip8::OP_Fx15(){
    m_DelayTimer = m_Register[(m_Opcode & 0x0F00) >> 8];
}

void Chip8::OP_Fx18(){
    m_SoundTimer = m_Register[(m_Opcode & 0x0F00) >> 8];
}

void Chip8::OP_Fx1E()
{
    m_IndexRegister += m_Register[(m_Opcode & 0x0F00) >> 8];
}

void Chip8::OP_Fx29(){
    m_IndexRegister = m_Register[(m_Opcode & 0x0F00) >> 8] * 0x5;
}

void Chip8::OP_Fx33(){
    m_Data[m_IndexRegister] = m_Register[(m_Opcode & 0x0F00) >> 8] / 100;
    m_Data[m_IndexRegister + 1] = (m_Register[(m_Opcode & 0x0F00) >> 8] / 10) % 10;
    m_Data[m_IndexRegister + 2] = (m_Register[(m_Opcode & 0x0F00) >> 8] % 100) % 10;
}

void Chip8::OP_Fx55(){
    for (int i = 0; i <= ((m_Opcode & 0x0F00) >> 8); i++) {
        m_Data[m_IndexRegister + i] = m_Register[i];
    }
}

void Chip8::OP_Fx65(){
    for (int i = 0; i <= ((m_Opcode & 0x0F00) >> 8); i++) {
        m_Register[i] = m_Data[m_IndexRegister + i];
    }
}

void Chip8::Nibble0(){ ExecuteOpcode(m_OpcodeTable0, m_Opcode & 0x00FF);}
void Chip8::Nibble8(){ ExecuteOpcode(m_OpcodeTable8, m_Opcode & 0x000F);}
void Chip8::NibbleE(){ ExecuteOpcode(m_OpcodeTableE, m_Opcode & 0x00FF);}
void Chip8::NibbleF(){ ExecuteOpcode(m_OpcodeTableF, m_Opcode & 0x00FF);}

void Chip8::InitializeOpcodeTable() {
        // Primary opcodes
        m_OpcodeTable = {
            {0x0, &Chip8::Nibble0},
            {0x1, &Chip8::OP_1nnn},
            {0x2, &Chip8::OP_2nnn},
            {0x3, &Chip8::OP_3xkk},
            {0x4, &Chip8::OP_4xkk},
            {0x5, &Chip8::OP_5xy0},
            {0x6, &Chip8::OP_6xkk},
            {0x7, &Chip8::OP_7xkk},
            {0x8, &Chip8::Nibble8},
            {0x9, &Chip8::OP_9xy0},
            {0xA, &Chip8::OP_Annn},
            {0xB, &Chip8::OP_Bnnn},
            {0xC, &Chip8::OP_Cxkk},
            {0xD, &Chip8::OP_Dxyn},
            {0xE, &Chip8::NibbleE},
            {0xF, &Chip8::NibbleF}
        };

        // 0x0XXX opcodes
        m_OpcodeTable0 = {
            {0xE0, &Chip8::OP_00E0},
            {0xEE, &Chip8::OP_00EE}
        };

        // 0x8XXX opcodes
        m_OpcodeTable8 = {
            {0x0, &Chip8::OP_8xy0},
            {0x1, &Chip8::OP_8xy1},
            {0x2, &Chip8::OP_8xy2},
            {0x3, &Chip8::OP_8xy3},
            {0x4, &Chip8::OP_8xy4},
            {0x5, &Chip8::OP_8xy5},
            {0x6, &Chip8::OP_8xy6},
            {0x7, &Chip8::OP_8xy7},
            {0xE, &Chip8::OP_8xyE}
        };

        // 0xEXXX opcodes
        m_OpcodeTableE = {
            {0x9E, &Chip8::OP_Ex9E},
            {0xA1, &Chip8::OP_ExA1}
        };

        // 0xFXXX opcodes
        m_OpcodeTableF = {
            {0x07, &Chip8::OP_Fx07},
            {0x0A, &Chip8::OP_Fx0A},
            {0x15, &Chip8::OP_Fx15},
            {0x18, &Chip8::OP_Fx18},
            {0x1E, &Chip8::OP_Fx1E},
            {0x29, &Chip8::OP_Fx29},
            {0x33, &Chip8::OP_Fx33},
            {0x55, &Chip8::OP_Fx55},
            {0x65, &Chip8::OP_Fx65}
        };
    }

void Chip8::ExecuteOpcode(const std::unordered_map<uint8_t, OpcodeFunc>& byteMap, uint8_t byteCode) {
    auto it = byteMap.find(byteCode);
    if (it != byteMap.end()) {
        (this->*(it->second))();
    } else {
        throw std::runtime_error("Unknown opcode");
    }
}

void Chip8::Cycle(){
    m_Opcode = (m_Data[m_ProgramCounter] << 8u) | m_Data[m_ProgramCounter + 1];
    m_ProgramCounter += 2;
    ExecuteOpcode(m_OpcodeTable, ((m_Opcode & 0xF000) >> 12));
}

void Chip8::DecrementTimers(std::function<void()> beepCallback){
    if (m_DelayTimer > 0) {
        m_DelayTimer--;
    }

    if (m_SoundTimer > 0) {
        if (m_SoundTimer == 1) 
            beepCallback();
        m_SoundTimer--;
    }
}

} // namespace Chip8Emulator