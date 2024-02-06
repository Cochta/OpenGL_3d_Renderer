#include "scene_manager.h"

#include "final_scene.h"

void SceneManager::Setup() {
  scenes_.push_back(std::make_unique<FinalScene>());
}

void SceneManager::UpdateScene(const float deltaTime) const noexcept {
  scenes_[sceneIdx_]->Update(deltaTime);
}

void SceneManager::ChangeScene(int index) noexcept {
  EndScene();
  sceneIdx_ = index;
  BeginScene();
}

void SceneManager::NextScene() noexcept {
  EndScene();
  if (sceneIdx_ >= scenes_.size() - 1)
    sceneIdx_ = 0;
  else
    sceneIdx_++;
  BeginScene();
}

void SceneManager::PreviousScene() noexcept {
  EndScene();
  if (sceneIdx_ <= 0)
    sceneIdx_ = scenes_.size() - 1;
  else
    sceneIdx_--;
  BeginScene();
}

void SceneManager::RegenerateScene() noexcept { ChangeScene(sceneIdx_); }

void SceneManager::BeginScene() noexcept { scenes_[sceneIdx_]->Begin(); }

void SceneManager::EndScene() noexcept { scenes_[sceneIdx_]->End(); }

void SceneManager::DrawImGui() noexcept {
  ImGui::Begin("Info");

  ImGui::Spacing();

  ImGui::TextWrapped(scenes_[sceneIdx_]->GetDescription().c_str());

  ImGui::Spacing();

  ImGui::SetCursorPosY(ImGui::GetWindowHeight() -
                       (ImGui::GetFrameHeightWithSpacing()));

  ImGui::End();
}

Camera& SceneManager::GetCamera() noexcept {
  return scenes_[sceneIdx_]->camera_;
}
