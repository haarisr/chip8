#include "window.hpp"

#include <print>

namespace chip8 {

std::optional<Window> Window::create(int w, int h) {
    SDL_Window* window = SDL_CreateWindow("Chip 8 Emulator", w, h, 0);
    if (!window) {
        std::println(stderr, "Failed to create SDL window: {}", SDL_GetError());
        return std::nullopt;
    }
    return Window(window);
}

Window::Window(Window&& other) noexcept : m_window(other.m_window), m_running(other.m_running) {
    other.m_window = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this == &other) return *this;
    m_window = other.m_window;
    m_running = other.m_running;
    other.m_window = nullptr;
    return *this;
}

Window::~Window() { SDL_DestroyWindow(m_window); }

void Window::pollEvents(std::span<bool> keys) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            const SDL_KeyboardEvent key_event = event.key;
            auto key = mapKey(key_event.key);
            if (key != -1) keys[key] = key_event.down;
        }
    }
}

int Window::mapKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_1:
            return 0x1;
        case SDLK_2:
            return 0x2;
        case SDLK_3:
            return 0x3;
        case SDLK_4:
            return 0xC;
        case SDLK_Q:
            return 0x4;
        case SDLK_W:
            return 0x5;
        case SDLK_E:
            return 0x6;
        case SDLK_R:
            return 0xD;
        case SDLK_A:
            return 0x7;
        case SDLK_S:
            return 0x8;
        case SDLK_D:
            return 0x9;
        case SDLK_F:
            return 0xE;
        case SDLK_Z:
            return 0xA;
        case SDLK_X:
            return 0x0;
        case SDLK_C:
            return 0xB;
        case SDLK_V:
            return 0xF;
    }
    return -1;
}
}  // namespace chip8
