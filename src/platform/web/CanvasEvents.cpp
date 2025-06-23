#include "CanvasEvents.h"
#include "../coda/app/main.h"
#include <emscripten/bind.h>

namespace coda {
namespace web {

void CanvasEvents::HandleCanvasEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                HandleMouseMotion(event);
                break;
                
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                HandleMouseButton(event);
                break;
                
            case SDL_MOUSEWHEEL:
                HandleMouseWheel(event);
                break;
        }
    }
}

void CanvasEvents::HandleMouseMotion(const SDL_Event& event) {
    coda::Input::Get().OnMouseMove(event.motion.x, event.motion.y);
}

void CanvasEvents::HandleMouseButton(const SDL_Event& event) {
    bool isDown = event.type == SDL_MOUSEBUTTONDOWN;
    coda::Input::Get().OnMouseButton(event.button.button - 1, isDown);
}

void CanvasEvents::HandleMouseWheel(const SDL_Event& event) {
    coda::Input::Get().OnMouseWheel(float(event.wheel.y));
}

void CanvasEvents::OnCanvasResize(int width, int height) {
    resize(width, height);
}

} // namespace web
} // namespace coda 
