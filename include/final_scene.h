#pragma once

#include <array>
#include <random>
#include <vector>

#include "scene.h"
#include "texture_manager.h"

struct BloomMip {
  glm::vec2 size;
  GLuint texture;
};

class FinalScene final : public Scene {
 private:
  TextureManager tm_;

  Mesh cube_;
  Mesh cube_ground_;
  Mesh quad_screen_;
  Mesh sphere_;

  Pipeline light_cube_;

  Pipeline pbr_pipe_;
  Pipeline cubemap_pipe_;
  Pipeline irradiance_pipe_;
  Pipeline background_pipe_;
  Pipeline prefilter_pipe_;
  Pipeline brdf_pipe_;
  Pipeline geom_pipe_;
  Pipeline ssao_pipe_;
  Pipeline ssao_blur_pipe_;

  Pipeline down_sample_pipe_;
  Pipeline up_sample_pipe_;
  Pipeline hdr_pipe_;

  GLuint captureFBO = 0;

  GLuint captureRBO = 0;

  GLuint hdr_cubemap_ = 0;
  GLuint env_cubemap_ = 0;
  GLuint irradianceMap = 0;
  GLuint brdfLUTTexture = 0;
  GLuint prefilterMap = 0;

  Model lamp_model_;
  Model backpack_model_;
  Model man_model_;

  Material ground_mat_;

  Material steel_;
  Material titanium_;

  glm::mat4 captureProjection = glm::mat4(1.0f);
  std::array<glm::mat4, 6> captureViews{};

  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
  glm::mat4 model = glm::mat4(1.0f);

  glm::vec3 lamp_pos_ = glm::vec3(0.077, 5.3, -10);
  glm::vec3 light_color_ = glm::vec3(10);

  GLuint bloom_fbo_;
  GLuint hdr_fbo_;
  GLuint hdr_rbo_;

  GLuint g_buffer_;
  GLuint pos_map_;
  GLuint normal_map_;
  GLuint albedo_map_;
  GLuint depth_rbo_;

  GLuint bright_tex_;
  GLuint scene_tex_;

  std::vector<BloomMip> bloom_mips_;

  static constexpr GLuint kSsaoKernelSampleCount_ = 64;
  std::array<glm::vec3, kSsaoKernelSampleCount_> ssao_kernel_{};
  static constexpr int kSsaoNoiseDimensionX_ = 4, kSsaoNoiseDimensionY_ = 4;

  static constexpr float kSsaoRadius = 0.5f;
  static constexpr float kSsaoBiais = 0.025f;
  static constexpr float kCombiendAoFactor = 1.f;

  GLuint noise_texture_;
  GLuint ssao_fbo_;
  GLuint ssao_blur_fbo_;
  GLuint ssao_tex_;
  GLuint ssao_blur_tex_;

  Pipeline shadow_map_pipe_;
  GLuint shadow_fbo_;
  GLuint shadow_tex_;

  glm::mat4 lightSpaceMatrix;

  static constexpr int shadow_tex_res_ = 4096;

  static constexpr float kLightNearPlane = 4.5f;
  static constexpr float kLightFarPlane = 100.f;

  static constexpr std::array<glm::vec3, 6> light_dirs = {
      glm::vec3(1, 0, 0),  glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0),
      glm::vec3(0, -1, 0), glm::vec3(0, 0, 1),  glm::vec3(0, 0, -1),
  };
  static constexpr std::array<glm::vec3, 6> light_ups = {
      glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1),
      glm::vec3(0, 0, -1), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0),
  };

 public:
  void Begin() override;
  void End() override;
  void Update(float dt) override;

 private:
  void BeginSkyBox();
  void UpdateSkyBox();
  void DeleteSkyBox();

  void CreateIrradianceMap();
  void CreatePrefilterMap();
  void CreateBRDF();

  void BeginLamp();
  void UpdateLamp();
  void DeleteLamp();

  void BeginGBuffer();
  void UpdateGBuffer();
  void DeleteGBuffer();

  void BeginSSAO();
  void UpdateSSAO();
  void DeleteSSAO();

  void BeginShadowMap();
  void DeleteShadowMap();

  void BeginPBR();
  void UpdatePBR();
  void DeletePBR();

  void BeginGround();
  void UpdateGround(Pipeline &pipeline);
  void DeleteGround();

  void BeginModels();
  void UpdateModels(Pipeline &pipeline);
  void DeleteModels();

  void UpdateSpheres(Pipeline &pipeline);

  void BeginBloom();
  void UpdateBloom();
  void DeleteBloom();
};