#include "PageEvents.h"

// Define JavaScript functions we want to call
EM_JS(void, js_registerCallbacks, (), {
    window.Module = window.Module || {};
    
    window.Module.onSetBackgroundBrightness = function(value) {
        _js_onSetBackgroundBrightness(value);
    };
    
    window.Module.onOtherSliderChange = function(value) {
        _js_onOtherSliderChange(value);
    };

    window.Module.onAddGeometry = function() {
        _js_onAddGeometry();
    };
});

namespace coda {
namespace web {

void PageEvents::Initialize() {
    js_registerCallbacks();
}

void PageEvents::HandlePageEvents() {
    // Empty implementation
}

void PageEvents::OnSetBackgroundBrightness(float value) {
    setBackgroundBrightness(value);
}

void PageEvents::OnOtherSliderChange(float value) {
    printf("Other slider value: %f\n", value);
}

void PageEvents::OnAddGeometry() {
    addTestGeometry();
}

} // namespace web
} // namespace coda

// Implement specific C functions for each control
extern "C" {
    void js_onSetBackgroundBrightness(float value) {
        coda::web::PageEvents::OnSetBackgroundBrightness(value);
    }
    
    void js_onOtherSliderChange(float value) {
        coda::web::PageEvents::OnOtherSliderChange(value);
    }

    void js_onAddGeometry() {
        coda::web::PageEvents::OnAddGeometry();
    }
}
 