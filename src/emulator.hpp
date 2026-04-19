#include <array>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <utility>

namespace chip8 {

static constexpr uint32_t kMemorySize = 4 * 1024;  // 4kB
static constexpr uint32_t kStackSize = 16;
static constexpr uint16_t kStartAddress = 0x200;
static constexpr uint32_t kWidth = 64;
static constexpr uint32_t kHeight = 32;

enum class InstructionField {
    Opcode,      // first nibble
    RegisterX,   // second nibble
    RegisterY,   // third nibble
    Immediate4,  // fourth nibble (N)
    Immediate8,  // NN
    Address12    // NNN
};

class Emulator {
   public:
    Emulator();
    bool loadRom(const std::string& path);
    void execute();
    void tick();
    void draw();
    std::span<bool> keys() { return m_keys; }

   private:
    uint16_t fetch();

    template <InstructionField field>
    constexpr uint16_t decode(uint16_t opcode);

    void op0(uint16_t opcode);
    void op1(uint16_t opcode);
    void op2(uint16_t opcode);
    void op3(uint16_t opcode);
    void op4(uint16_t opcode);
    void op5(uint16_t opcode);
    void op6(uint16_t opcode);
    void op7(uint16_t opcode);
    void op8(uint16_t opcode);
    void op9(uint16_t opcode);
    void opA(uint16_t opcode);
    void opD(uint16_t opcode);
    void opE(uint16_t opcode);
    void opF(uint16_t opcode);

   private:
    uint8_t m_delay_timer;
    uint8_t m_sound_timer;
    uint16_t m_pc;
    uint16_t m_index_register;
    std::array<uint16_t, 16> m_stack;
    std::array<bool, 16> m_keys;
    uint8_t m_stack_ptr{0};
    std::array<bool, kWidth * kHeight> m_display;
    std::array<uint8_t, kMemorySize> m_memory;
    std::array<uint8_t, 16> m_registers;
    std::array<std::function<void(uint16_t)>, 16> m_dispatch_table;
};

template <InstructionField field>
constexpr uint16_t Emulator::decode(uint16_t opcode) {
    if constexpr (field == InstructionField::Opcode) {
        return (opcode >> 12) & 0xF;
    } else if constexpr (field == InstructionField::RegisterX) {
        return (opcode >> 8) & 0xF;
    } else if constexpr (field == InstructionField::RegisterY) {
        return (opcode >> 4) & 0xF;
    } else if constexpr (field == InstructionField::Immediate4) {
        return opcode & 0xF;
    } else if constexpr (field == InstructionField::Immediate8) {
        return opcode & 0xFF;
    } else if constexpr (field == InstructionField::Address12) {
        return opcode & 0xFFF;
    }
    std::unreachable();
}

}  // namespace chip8
