#include <array>
#include <cstdint>
#include <utility>

namespace chip8 {

static constexpr uint32_t kMemorySize = 4 * 1024;  // 4kB

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

   private:
    uint16_t fetch();

    template <InstructionField field>
    constexpr uint16_t decode(uint16_t opcode);

   private:
    uint16_t m_pc;
    std::array<uint8_t, kMemorySize> m_memory;
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
