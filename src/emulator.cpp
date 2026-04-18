#include "emulator.hpp"
#include <cstdint>
#include <fstream>
#include <print>

namespace chip8 {

consteval std::array<uint8_t, kMemorySize> initializeMemory() {
    std::array<uint8_t, kMemorySize> memory;
    uint8_t fonts[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F};
    };
    uint8_t start = 0x50;
    for (const auto font : fonts) {
        memory[start++] = font;
    }
    return memory;
}

Emulator::Emulator() {
    m_memory = initializeMemory();
    static_assert(decode<InstructionField::Opcode>(0xA1EF) == 0xA);
    static_assert(decode<InstructionField::RegisterX>(0xA1EF) == 0x1);
    static_assert(decode<InstructionField::RegisterY>(0xA1EF) == 0xE);
    static_assert(decode<InstructionField::Immediate4>(0xA1EF) == 0xF);
    static_assert(decode<InstructionField::Immediate8>(0xA1EF) == 0xEF);
    static_assert(decode<InstructionField::Address12>(0xA1EF) == 0x1EF);

    m_dispatch_table[0x0] = [this](uint16_t opcode) { op0(opcode); };
    m_dispatch_table[0x1] = [this](uint16_t opcode) { op1(opcode); };
    m_dispatch_table[0x6] = [this](uint16_t opcode) { op6(opcode); };
    m_dispatch_table[0x7] = [this](uint16_t opcode) { op7(opcode); };
    m_dispatch_table[0xA] = [this](uint16_t opcode) { opA(opcode); };
    m_dispatch_table[0xD] = [this](uint16_t opcode) { opD(opcode); };
}

bool Emulator::loadRom(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size < 0) return false;

    if (kStartAddress + static_cast<size_t>(size) > m_memory.size()) return false;

    file.read(reinterpret_cast<char*>(&m_memory[kStartAddress]), size);
    m_pc = kStartAddress;
    return true;
}

uint16_t Emulator::fetch() {
    uint16_t high = m_memory[m_pc++];
    uint16_t low = m_memory[m_pc++];
    return (high << 8) | low;
}

void Emulator::execute() {
    auto opcode = fetch();
    m_dispatch_table[decode<InstructionField::Opcode>(opcode)](opcode);
}

void Emulator::draw() {
    std::println("\x1B[2J\x1B[H");
    for (size_t y = 0; y < kHeight; y++) {
        for (size_t x = 0; x < kWidth; x++) {
            m_display[y * kWidth + x] ? std::print("#") : std::print(" ");
        }
        std::println();
    }
}

void Emulator::op0(uint16_t) { m_display = {0}; }

void Emulator::op1(uint16_t opcode) { m_pc = decode<InstructionField::Address12>(opcode); }

void Emulator::op6(uint16_t opcode) {
    m_registers[decode<InstructionField::RegisterX>(opcode)] =
        decode<InstructionField::Immediate8>(opcode);
}
void Emulator::op7(uint16_t opcode) {
    m_registers[decode<InstructionField::RegisterX>(opcode)] +=
        decode<InstructionField::Immediate8>(opcode);
}

void Emulator::opA(uint16_t opcode) {
    m_index_register = decode<InstructionField::Address12>(opcode);
}
void Emulator::opD(uint16_t opcode) {
    uint8_t x_coord = m_registers[decode<InstructionField::RegisterX>(opcode)] & (kWidth - 1);
    uint8_t y_coord = m_registers[decode<InstructionField::RegisterY>(opcode)] & (kHeight - 1);

    m_registers[0xF] = 0;

    uint8_t height = decode<InstructionField::Immediate4>(opcode);
    for (size_t row = 0; row < height; row++) {
        const uint8_t y = y_coord + row;
        if (y >= kHeight) break;

        auto sprite = m_memory[m_index_register + row];
        const auto row_offset = y * kWidth;

        for (size_t bit = 0; bit < 8; bit++) {
            const uint8_t x = x_coord + bit;
            if (x >= kWidth) break;

            if ((sprite >> (7 - bit)) & 1) {
                auto index = row_offset + x;
                m_registers[0xF] |= m_display[index];
                m_display[index] ^= 1;
            }
        }
    }
}

}  // namespace chip8
