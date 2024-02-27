#include "engine.h"

#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <cassert>
#include <chrono>
#include <glm/vec2.hpp>

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyC.h"
#endif 

void Engine::Run() {
  Begin();
  bool isOpen = true;

  std::chrono::time_point<std::chrono::system_clock> clock =
      std::chrono::system_clock::now();
  while (isOpen) {
    const auto start = std::chrono::system_clock::now();
    using seconds = std::chrono::duration<float, std::ratio<1, 1>>;
    const auto dt = std::chrono::duration_cast<seconds>(start - clock);
    clock = start;

    float lastX = 0;  // Replace with your initial mouse X position
    float lastY = 0;  // Replace with your initial mouse Y position
    bool firstMouse = true;
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    bool IsKeyboardActive = false;
    // Manage SDL event
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          isOpen = false;
          break;
        case SDL_WINDOWEVENT: {
          switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
              isOpen = false;
              break;
            default:
              break;
          }
          break;
        }
        case SDL_KEYDOWN:
          IsKeyboardActive = true;
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:
              SDL_SetRelativeMouseMode(SDL_TRUE);
              break;
            default:
              break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:
              SDL_SetRelativeMouseMode(SDL_FALSE);
              break;
            default:
              break;
          }
          break;
        case SDL_MOUSEMOTION: {
          if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            float xpos = static_cast<float>(event.motion.x);
            float ypos = static_cast<float>(event.motion.y);

            if (firstMouse) {
              lastX = xpos;
              lastY = ypos;
              firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset =
                lastY -
                ypos;  // reversed since y-coordinates go from bottom to top

            lastX = xpos;
            lastY = ypos;

            sm_.GetCamera().ProcessMouseMovement(xoffset, yoffset);
          }
        } break;
        case SDL_MOUSEWHEEL:
          sm_.GetCamera().ProcessMouseScroll(event.wheel.y);
          break;
        default:
          break;
      }
      // sm_.scenes_[0]->OnEvent(event);
      if (!IsKeyboardActive) {
        ImGui_ImplSDL2_ProcessEvent(&event);
      }
    }
    if (keys[SDL_SCANCODE_A]) {
      sm_.GetCamera().ProcessKeyboard(Direction::LEFT, dt.count());
    }
    if (keys[SDL_SCANCODE_D]) {
      sm_.GetCamera().ProcessKeyboard(Direction::RIGHT, dt.count());
    }
    if (keys[SDL_SCANCODE_W]) {
      sm_.GetCamera().ProcessKeyboard(Direction::FORWARD, dt.count());
    }
    if (keys[SDL_SCANCODE_S]) {
      sm_.GetCamera().ProcessKeyboard(Direction::BACKWARD, dt.count());
    }
    if (keys[SDL_SCANCODE_SPACE]) {
      sm_.GetCamera().ProcessKeyboard(Direction::UP, dt.count());
    }
    if (keys[SDL_SCANCODE_LCTRL]) {
      sm_.GetCamera().ProcessKeyboard(Direction::DOWN, dt.count());
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    sm_.UpdateScene(dt.count());

    // Generate new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window_);
    ImGui::NewFrame();

    sm_.DrawImGui();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window_);
#ifdef TRACY_ENABLE
    FrameMark;
#endif 
  }
  End();
}

void Engine::Begin() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
  // Set our OpenGL version.
#if true
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  window_ = SDL_CreateWindow("Scenes", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, Metrics::width_,
                             Metrics::height_, SDL_WINDOW_OPENGL);
  glRenderContext_ = SDL_GL_CreateContext(window_);
  // setting vsync
  SDL_GL_SetSwapInterval(1);

  if (GLEW_OK != glewInit()) {
    assert(false && "Failed to initialize OpenGL context");
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Gamepad
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window_, glRenderContext_);
  ImGui_ImplOpenGL3_Init("#version 300 es");

  sm_.Setup();
  sm_.BeginScene();
}

void Engine::End() {
  sm_.EndScene();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(glRenderContext_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}
