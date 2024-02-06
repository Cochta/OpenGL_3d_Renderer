#pragma once

#include <imgui.h>

#include <memory>
#include <vector>

#include "scene.h"

class SceneManager {
 public:
  std::vector<std::unique_ptr<Scene>> scenes_;

  std::size_t sceneIdx_ = 0;

  void Setup();

  void UpdateScene(float deltaTime) const noexcept;

  void ChangeScene(int index) noexcept;

  void NextScene() noexcept;

  void PreviousScene() noexcept;

  void RegenerateScene() noexcept;

  void BeginScene() noexcept;

  void EndScene() noexcept;

  void DrawImGui() noexcept;

  Camera& GetCamera() noexcept;
};