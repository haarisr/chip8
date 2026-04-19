#include "emulator.hpp"
#include <cstdint>
#include <fstream>
#include <print>

namespace chip8 {

consteval std::array<uint8_t, kMemorySize> initializeMemory() {
    std::array<uint8_t, kMemorySize> memory;
    std::array<uint8_t, 80> fonts = {
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
    for (size_t i = 0; i < fonts.size(); i++) {
        memory[kFontAddress + i] = fonts[i];
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
    m_dispatch_table[0x2] = [this](uint16_t opcode) { op2(opcode); };
    m_dispatch_table[0x3] = [this](uint16_t opcode) { op3(opcode); };
    m_dispatch_table[0x4] = [this](uint16_t opcode) { op4(opcode); };
    m_dispatch_table[0x5] = [this](uint16_t opcode) { op5(opcode); };
    m_dispatch_table[0x6] = [this](uint16_t opcode) { op6(opcode); };
    m_dispatch_table[0x7] = [this](uint16_t opcode) { op7(opcode); };
    m_dispatch_table[0x8] = [this](uint16_t opcode) { op8(opcode); };
    m_dispatch_table[0x9] = [this](uint16_t opcode) { op9(opcode); };
    m_dispatch_table[0xA] = [this](uint16_t opcode) { opA(opcode); };
    m_dispatch_table[0xD] = [this](uint16_t opcode) { opD(opcode); };
    m_dispatch_table[0xE] = [this](uint16_t opcode) { opE(opcode); };
    m_dispatch_table[0xF] = [this](uint16_t opcode) { opF(opcode); };

    m_dispatch_table[0xB] = [](uint16_t) {};
    m_dispatch_table[0xC] = [](uint16_t) {};
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

void Emulator::tick() {
    if (m_delay_timer > 0) m_delay_timer--;
    if (m_sound_timer > 0) m_sound_timer--;
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

void Emulator::op0(uint16_t opcode) {
    switch (opcode) {
        case 0x00E0:
            m_display = {0};
            return;
        case 0x00EE:
            m_pc = m_stack[--m_stack_ptr];
            return;
    }
}

void Emulator::op1(uint16_t opcode) { m_pc = decode<InstructionField::Address12>(opcode); }

void Emulator::op2(uint16_t opcode) {
    m_stack[m_stack_ptr++] = m_pc;
    m_pc = decode<InstructionField::Address12>(opcode);
}

void Emulator::op3(uint16_t opcode) {
    const auto regx = m_registers[decode<InstructionField::RegisterX>(opcode)];
    const auto imm8 = decode<InstructionField::Immediate8>(opcode);
    if (regx == imm8) m_pc += 2;
}

void Emulator::op4(uint16_t opcode) {
    const auto regx = m_registers[decode<InstructionField::RegisterX>(opcode)];
    const auto imm8 = decode<InstructionField::Immediate8>(opcode);
    if (regx != imm8) m_pc += 2;
}

void Emulator::op5(uint16_t opcode) {
    const auto regx = m_registers[decode<InstructionField::RegisterX>(opcode)];
    const auto regy = m_registers[decode<InstructionField::RegisterY>(opcode)];
    if (regx == regy) m_pc += 2;
}

void Emulator::op6(uint16_t opcode) {
    m_registers[decode<InstructionField::RegisterX>(opcode)] =
        decode<InstructionField::Immediate8>(opcode);
}

void Emulator::op7(uint16_t opcode) {
    m_registers[decode<InstructionField::RegisterX>(opcode)] +=
        decode<InstructionField::Immediate8>(opcode);
}

void Emulator::op8(uint16_t opcode) {
    // Make configurable for old ones
    const auto alu_op = decode<InstructionField::Immediate4>(opcode);
    auto& regx = m_registers[decode<InstructionField::RegisterX>(opcode)];
    auto& regy = m_registers[decode<InstructionField::RegisterY>(opcode)];
    switch (alu_op) {
        case 0x0:
            regx = regy;
            return;
        case 0x1:
            regx |= regy;
            return;
        case 0x2:
            regx &= regy;
            return;
        case 0x3:
            regx ^= regy;
            return;
        case 0x4: {
            uint16_t sum = regx + regy;
            regx = sum & 0xFF;
            m_registers[0xF] = sum > 0xFF;
            return;
        }
        case 0x5: {
            uint16_t diff = regx - regy;
            bool result = regx >= regy;
            regx = diff & 0xFF;
            m_registers[0xF] = result;
            return;
        }
        case 0x7: {
            uint16_t diff = regy - regx;
            bool result = regy >= regx;
            regx = diff & 0xFF;
            m_registers[0xF] = result;
            return;
        }
        case 0x6: {
            bool carry = regx & 0x1;
            regx >>= 1;
            m_registers[0xF] = carry;
            return;
        }
        case 0xE: {
            bool carry = regx & 0x80;
            regx <<= 1;
            m_registers[0xF] = carry;
            return;
        }
    }
}

void Emulator::op9(uint16_t opcode) {
    const auto regx = m_registers[decode<InstructionField::RegisterX>(opcode)];
    const auto regy = m_registers[decode<InstructionField::RegisterY>(opcode)];
    if (regx != regy) m_pc += 2;
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

void Emulator::opE(uint16_t opcode) {
    const auto imm8 = decode<InstructionField::Immediate8>(opcode);
    const auto regx = m_registers[decode<InstructionField::RegisterX>(opcode)];
    switch (imm8) {
        case 0x9E:
            if (m_keys[regx]) m_pc += 2;
            return;
        case 0xA1:
            if (!m_keys[regx]) m_pc += 2;
            return;
    }
}

void Emulator::opF(uint16_t opcode) {
    // Make configurable for old ones
    const auto imm8 = decode<InstructionField::Immediate8>(opcode);
    const auto x = decode<InstructionField::RegisterX>(opcode);

    switch (imm8) {
        case 0x55:
            for (uint8_t i = 0; i <= x; i++) {
                m_memory[m_index_register + i] = m_registers[i];
            }
            return;
        case 0x65:
            for (uint8_t i = 0; i <= x; i++) {
                m_registers[i] = m_memory[m_index_register + i];
            }
            return;
        case 0x33: {
            auto number = m_registers[x];
            for (uint8_t i = 0; i <= 2; i++) {
                m_memory[m_index_register + (2 - i)] = number % 10;
                number = number / 10;
            }
        }
            return;
        case 0x1E:
            m_index_register += m_registers[x];
            return;
        case 0x07:
            m_registers[x] = m_delay_timer;
            return;
        case 0x15:
            m_delay_timer = m_registers[x];
            return;
        case 0x18:
            m_sound_timer = m_registers[x];
            return;

        case 0x0A: {
            for (uint8_t i = 0; i < m_keys.size(); i++) {
                if (m_keys[i]) {
                    m_registers[x] = i;
                    return;
                }
            }
            m_pc -= 2;
            return;
        }
        case 0x29:
            m_index_register = kFontAddress + m_registers[x] * 5;
            return;
    }
}

}  // namespace chip8
