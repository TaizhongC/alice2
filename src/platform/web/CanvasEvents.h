#pragma once

#ifdef USE_EMSCRIPTEN
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif

#include "../../coda/app/Input.h"

namespace coda {
namespace web {

class CanvasEvents {
public:
    static void HandleCanvasEvents();
    static void OnCanvasResize(int width, int height);
    
private:
    static void HandleMouseMotion(const SDL_Event& event);
    static void HandleMouseButton(const SDL_Event& event);
    static void HandleMouseWheel(const SDL_Event& event);
};

} // namespace web
} // namespace coda 