#include "emulator.hpp"
#include "window.hpp"

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

    chip8::SDLContext ctx;

    auto window_opt = chip8::Window::create(640, 480);
    if (!window_opt) {
        std::println(stderr, "Failed to create window");
        return 1;
    }

    auto window = std::move(*window_opt);
    while (window.isRunning()) {
        window.pollEvents(emulator.keys());
        emulator.execute();
        emulator.draw();
    }
}
