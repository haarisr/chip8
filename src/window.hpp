#include <optional>
#include <span>
#include <stdexcept>

#include <SDL3/SDL.h>

namespace chip8 {

class SDLContext {
   public:
    SDLContext() {
        if (!SDL_Init(SDL_INIT_VIDEO)) throw std::runtime_error(SDL_GetError());
    }
    ~SDLContext() { SDL_Quit(); }
};

class Window {
   public:
    static std::optional<Window> create(int w, int h);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void pollEvents(std::span<bool> keys);
    bool isRunning() const { return m_running; }

   private:
    Window(SDL_Window* window) : m_window(window) {}
    int mapKey(SDL_Keycode key);

   private:
    SDL_Window* m_window;
    bool m_running{true};
};

}  // namespace chip8
