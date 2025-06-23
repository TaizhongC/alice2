#ifdef USE_EMSCRIPTEN
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengles2.h>
#else
    #include <SDL.h>
    #include <SDL_opengles2.h>
#endif
#include "../../coda/app/Application.h"
#include "../../coda/app/Scene.h"
#include "../../coda/app/main.h"
#include "../../coda/app/Input.h"
#include "CanvasEvents.h"
#include "PageEvents.h"

SDL_Window* window = nullptr;
SDL_GLContext glContext = nullptr;
int windowWidth;   // Will be set based on canvas size
int windowHeight;  // Will be set based on canvas size

bool initSDL() {
    // Get the canvas size from the DOM
    windowWidth = EM_ASM_INT({
        return document.getElementById('canvas').width;
    });
    windowHeight = EM_ASM_INT({
        return document.getElementById('canvas').height;
    });

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);  // 4x MSAA

    window = SDL_CreateWindow(
        "CODA",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("OpenGL context creation failed: %s\n", SDL_GetError());
        return false;
    }

    // In WebGL, multisampling is enabled automatically if available
    
    glViewport(0, 0, windowWidth, windowHeight);
    coda::Scene::Get().GetCamera().SetAspect(float(windowWidth) / float(windowHeight));

    return true;
}

void cleanup() {
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void frame() {
    coda::web::CanvasEvents::HandleCanvasEvents();
    coda::web::PageEvents::HandlePageEvents();
    
    // Check if canvas size has changed
    int currentWidth = EM_ASM_INT({
        return document.getElementById('canvas').width;
    });
    int currentHeight = EM_ASM_INT({
        return document.getElementById('canvas').height;
    });
    
    if (currentWidth != windowWidth || currentHeight != windowHeight) {
        windowWidth = currentWidth;
        windowHeight = currentHeight;
        SDL_SetWindowSize(window, windowWidth, windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);
        resize(windowWidth, windowHeight);
    }

    update();
    SDL_GL_SwapWindow(window);
}

int main() {
    printf("Platform initialization started\n");
    
    if (!initSDL()) {
        cleanup();
        return 1;
    }

    if (!initApplication(windowWidth, windowHeight)) {
        cleanup();
        return 1;
    }

    coda::web::PageEvents::Initialize();
    setup();
    
    // Set up the main loop
    emscripten_set_main_loop(frame, 0, true);

    return 0;
} 