#pragma once
#include <emscripten.h>
#include "../../coda/app/main.h"

// Declare C-style functions for JavaScript
extern "C" {
    void EMSCRIPTEN_KEEPALIVE js_onSetBackgroundBrightness(float value);
    void EMSCRIPTEN_KEEPALIVE js_onOtherSliderChange(float value);
    void EMSCRIPTEN_KEEPALIVE js_onAddGeometry();
}

namespace coda {
namespace web {

class PageEvents {
public:
    static void Initialize();
    static void HandlePageEvents();
    
    // Internal C++ functions
    static void OnSetBackgroundBrightness(float value);
    static void OnOtherSliderChange(float value);
    static void OnAddGeometry();
};

} // namespace web
} // namespace coda