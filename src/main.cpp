#include "app/application.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

alice2::Application app;

void main_loop_emscripten() {
    app.MainLoop();
}

int main() {
    if (!app.Initialize(800, 600)) {
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop_emscripten, 0, true);
#else
    while (app.IsRunning()) {
        app.MainLoop();
    }
#endif

    app.Terminate();

    return 0;
}