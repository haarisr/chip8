#include "emulator.hpp"

#include <print>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::println(stderr, "Usage: {} <rom>", argv[0]);
        return 1;
    }

    chip8::Emulator emulator;
    const char* name = argv[1];
    if (!emulator.loadRom(name)) {
        std::println(stderr, "Failed to load rom: {}", name);
        return 1;
    }
    while (true) {
        emulator.execute();
        emulator.draw();
    }
}
