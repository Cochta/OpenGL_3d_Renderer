#pragma once

#include <SDL.h>

#include "camera.h"
#include "metrics.h"
#include "pipeline.h"

class Scene {
 public:
  Camera camera_;

  virtual ~Scene() = default;
  virtual void OnEvent(const SDL_Event &event) {}

  virtual void Begin() = 0;
  virtual void Update(float dt) = 0;
  virtual void End() = 0;
};
