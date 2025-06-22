#include "app/application.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

alice2::Application app;

// void main_loop_emscripten() {
//     app.MainLoop();
// }

int main() {
    if (!app.Initialize()) {
        return 1;
    }

    app.Run();

    return 0;
}