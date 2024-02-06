#pragma once
#include "scene.h"
#include "scene_manager.h"

class Engine {
 public:
  Engine() = default;
  void Run();

 private:
  SceneManager sm_;

  void Begin();
  void End();
  SDL_Window* window_ = nullptr;
  SDL_GLContext glRenderContext_{};
};
