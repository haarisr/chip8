#include <array>
#include <cstdint>

namespace chip8 {

static constexpr uint32_t kMemorySize = 4 * 1024;  // 4kB

class Emulator {
   public:
    Emulator();

   private:
    std::array<uint8_t, kMemorySize> m_memory;
};

}  // namespace chip8
