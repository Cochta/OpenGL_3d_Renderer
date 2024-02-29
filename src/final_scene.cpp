#include "final_scene.h"

#include <thread>

#ifdef TRACY_ENABLE
#include <TracyC.h>

#include <Tracy.hpp>
#endif

void FinalScene::Begin() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  LoadRessources();
}

void FinalScene::Update(float dt) {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  is_frist_frame_ = false;
  while (!are_all_data_loaded_) {
    Job* job = nullptr;

    if (!main_thread_jobs_.empty()) {
      job = main_thread_jobs_.front();
      if (job->AreDependencyDone()) {
        job->Execute();
        main_thread_jobs_.pop();
      } else {
        return;
      }
    } else {
      are_all_data_loaded_ = true;
      break;
    }
  }

  if (!is_initialized_) {
    job_system_.JoinWorkers();
    cube_.SetCube();
    cube_ground_.SetCube(30, {1, 0.1});
    quad_screen_.SetQuad(2);
    sphere_.SetSphere();

    BeginBloom();
    BeginSkyBox();
    CreateIrradianceMap();
    CreatePrefilterMap();
    CreateBRDF();
    BeginLamp();

    BeginGBuffer();
    BeginSSAO();
    BeginPBR();
    BeginShadowMap();

    glViewport(0, 0, Metrics::width_, Metrics::height_);
    camera_ = (glm::vec3(0.0f, 2.0f, 0.0f));

    is_initialized_ = true;
    return;
  }

  view = camera_.GetViewMatrix();
  projection =
      glm::perspective(glm::radians(camera_.zoom_),
                       Metrics::width_ / Metrics::height_, 0.1f, 100.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  UpdateGBuffer();
  UpdateSSAO();
  UpdatePBR();
  glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_fbo_);
  glBlitFramebuffer(0, 0, Metrics::width_, Metrics::height_, 0, 0,
                    Metrics::width_, Metrics::height_, GL_DEPTH_BUFFER_BIT,
                    GL_NEAREST);
  UpdateLamp();
  UpdateSkyBox();
  UpdateBloom();
}
void FinalScene::End() {
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  DeleteLamp();
  DeleteGround();
  DeleteModels();
  DeleteSkyBox();
  DeleteBloom();
  DeletePBR();
  DeleteGBuffer();
  DeleteSSAO();
  DeleteShadowMap();
}

void FinalScene::BeginSkyBox() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  background_pipe_.LoadShader("data/shaders/final/skybox.vert",
                              "data/shaders/final/skybox.frag");
  background_pipe_.LoadProgram();

  background_pipe_.Bind();
  background_pipe_.SetInt("environmentMap", 0);

  // buffers
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 4096, 4096);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, captureRBO);

  hdr_cubemap_ = tm_.LoadHDR("data/textures/final/peter.hdr");

  // pbr: setup cubemap to render to and attach to framebuffer
  // ---------------------------------------------------------

  glGenTextures(1, &env_cubemap_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap_);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 4096, 4096,
                 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // pbr: set up projection and view matrices for capturing data onto the 6
  // cubemap face directions
  // ----------------------------------------------------------------------------------------------
  captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  captureViews = {
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f))};

  // pbr: convert HDR equirectangular environment map to cubemap equivalent
  // ----------------------------------------------------------------------
  cubemap_pipe_.LoadShader("data/shaders/pbr/cubemap.vert",
                           "data/shaders/pbr/cubemap.frag");
  cubemap_pipe_.LoadProgram();

  cubemap_pipe_.Bind();

  cubemap_pipe_.SetInt("equirectangularMap", 0);
  cubemap_pipe_.SetMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdr_cubemap_);

  glViewport(0, 0, 4096, 4096);  // don't forget to configure the viewport to
                                 // the
                                 // capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i) {
    cubemap_pipe_.SetMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cubemap_, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cube_.Draw();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::UpdateSkyBox() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  glDepthFunc(GL_LEQUAL);  // set depth function to less than AND equal for
                           // skybox depth trick.
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  // render skybox (render as last to prevent overdraw)
  background_pipe_.Bind();
  background_pipe_.SetMat4("view", view);
  background_pipe_.SetMat4("projection", projection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap_);

  cube_.Draw();
}

void FinalScene::DeleteSkyBox() { cubemap_pipe_.Delete(); }

void FinalScene::CreateIrradianceMap() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance
  // scale.
  // --------------------------------------------------------------------------------
  irradianceMap;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
                 GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

  // pbr: solve diffuse integral by convolution to create an irradiance
  // (cube)map.
  // -----------------------------------------------------------------------------

  irradiance_pipe_.LoadShader("data/shaders/pbr/cubemap.vert",
                              "data/shaders/pbr/irradiance.frag");

  irradiance_pipe_.LoadProgram();

  irradiance_pipe_.Bind();
  irradiance_pipe_.SetInt("environmentMap", 0);
  irradiance_pipe_.SetMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap_);

  glViewport(
      0, 0, 32,
      32);  // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i) {
    irradiance_pipe_.SetMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap,
                           0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cube_.Draw();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::CreatePrefilterMap() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  // pbr: create a pre-filter cubemap, and re-scale capture FBO to
  // pre-filter scale.
  // --------------------------------------------------------------------------------

  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0,
                 GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);  // be sure to set minification
                                             // filter to mip_linear
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the
  // required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // pbr: run a quasi monte-carlo simulation on the environment lighting to
  // create a prefilter (cube)map.
  // ----------------------------------------------------------------------------------------------------

  prefilter_pipe_.LoadShader("data/shaders/pbr/cubemap.vert",
                             "data/shaders/pbr/prefilter.frag");

  prefilter_pipe_.LoadProgram();

  prefilter_pipe_.Bind();
  prefilter_pipe_.SetInt("environmentMap", 0);
  prefilter_pipe_.SetMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, env_cubemap_);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
    unsigned int mipHeight =
        static_cast<unsigned int>(128 * std::pow(0.5, mip));
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth,
                          mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prefilter_pipe_.SetFloat("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      prefilter_pipe_.SetMat4("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap,
                             mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      cube_.Draw();
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::CreateBRDF() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  // pbr: generate a 2D LUT from the BRDF equations used.
  // ----------------------------------------------------

  glGenTextures(1, &brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1024, 1024, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad
  // with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         brdfLUTTexture, 0);

  glViewport(0, 0, 1024, 1024);

  brdf_pipe_.LoadShader("data/shaders/pbr/brdf.vert",
                        "data/shaders/pbr/brdf.frag");

  brdf_pipe_.LoadProgram();

  brdf_pipe_.Bind();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  quad_screen_.Draw();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::BeginLamp() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  light_cube_.LoadShader("data/shaders/final/lamp.vert",
                         "data/shaders/final/lamp.frag");
  light_cube_.LoadProgram();
}

void FinalScene::UpdateLamp() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  light_cube_.Bind();
  light_cube_.SetMat4("projection", projection);
  light_cube_.SetMat4("view", view);
  model = glm::mat4(1.0f);
  model = glm::translate(model, lamp_pos_);
  model = glm::scale(model, glm::vec3(0.3f));  // a smaller cube
  light_cube_.SetMat4("model", model);

  cube_.Draw();
}

void FinalScene::DeleteLamp() { light_cube_.Delete(); }

void FinalScene::UpdateGround(Pipeline& pipeline) {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  ground_mat_.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0, -2.45, 0));
  model = glm::scale(model, glm::vec3(1, 0.1, 1));
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::transpose(glm::inverse(glm::mat4(view * model))));

  cube_ground_.Draw();
}

void FinalScene::DeleteGround() { ground_mat_.Clear(); }

void FinalScene::BeginGBuffer() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  geom_pipe_.LoadShader("data/shaders/Final/g_buffer.vert",
                        "data/shaders/Final/g_buffer.frag");

  geom_pipe_.LoadProgram();

  geom_pipe_.Bind();
  geom_pipe_.SetInt("albedoMap", 0);
  geom_pipe_.SetInt("normalMap", 1);
  geom_pipe_.SetInt("metallicMap", 2);
  geom_pipe_.SetInt("roughnessMap", 3);
  geom_pipe_.SetInt("aoMap", 4);

  // configure g-buffer framebuffer
  // ------------------------------
  glGenFramebuffers(1, &g_buffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);

  // position color buffer
  glGenTextures(1, &pos_map_);
  glBindTexture(GL_TEXTURE_2D, pos_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Metrics::width_, Metrics::height_,
               0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         pos_map_, 0);

  // normal color buffer
  glGenTextures(1, &normal_map_);
  glBindTexture(GL_TEXTURE_2D, normal_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Metrics::width_, Metrics::height_,
               0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         normal_map_, 0);

  // color + specular color buffer
  glGenTextures(1, &albedo_map_);
  glBindTexture(GL_TEXTURE_2D, albedo_map_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Metrics::width_, Metrics::height_, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         albedo_map_, 0);

  // tell OpenGL which color attachments we'll use (of this framebuffer) for
  // rendering
  static constexpr std::array<GLuint, 3> attachments = {
      GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments.data());

  // create and attach depth buffer (renderbuffer)
  glGenRenderbuffers(1, &depth_rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Metrics::width_,
                        Metrics::height_);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth_rbo_);

  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::UpdateGBuffer() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  geom_pipe_.Bind();
  geom_pipe_.SetMat4("view", view);
  geom_pipe_.SetMat4("projection", projection);

  glBindFramebuffer(GL_FRAMEBUFFER, g_buffer_);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  UpdateGround(geom_pipe_);
  UpdateModels(geom_pipe_);
  UpdateSpheres(geom_pipe_);
}

void FinalScene::DeleteGBuffer() {
  geom_pipe_.Delete();
  glDeleteRenderbuffers(1, &depth_rbo_);
  g_buffer_ = 0;
  pos_map_ = 0;
  normal_map_ = 0;
  albedo_map_ = 0;
  depth_rbo_ = 0;
}

void FinalScene::BeginSSAO() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  // generate sample kernel
  // ----------------------
  std::uniform_real_distribution<GLfloat> random_floats(0.0, 1.0);
  // generates random floats between 0.0 and 1.0
  std::default_random_engine generator;

  for (GLuint i = 0; i < kSsaoKernelSampleCount_; i++) {
    glm::vec3 sample(random_floats(generator) * 2.0 - 1.0,
                     random_floats(generator) * 2.0 - 1.0,
                     random_floats(generator));
    sample = glm::normalize(sample);
    sample *= random_floats(generator);
    float scale = float(i) / kSsaoKernelSampleCount_;

    // scale samples s.t. they're more aligned to center of kernel
    // scale = lerp(0.1f, 1.0f, scale * scale);
    scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
    sample *= scale;
    ssao_kernel_[i] = sample;
  }

  static constexpr auto kSsaoNoiseDimensionXY =
      kSsaoNoiseDimensionX_ * kSsaoNoiseDimensionY_;

  std::array<glm::vec3, kSsaoNoiseDimensionXY> ssao_noise{};
  for (GLuint i = 0; i < kSsaoNoiseDimensionXY; i++) {
    // As the sample kernel is oriented along the positive z direction in
    // tangent space, we leave the z component at 0.0 so we rotate around the
    // z axis.
    glm::vec3 noise(random_floats(generator) * 2.0 - 1.0,
                    random_floats(generator) * 2.0 - 1.0, 0.0f);
    ssao_noise[i] = noise;
  }

  glGenTextures(1, &noise_texture_);
  glBindTexture(GL_TEXTURE_2D, noise_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, kSsaoNoiseDimensionX_,
               kSsaoNoiseDimensionX_, 0, GL_RGB, GL_FLOAT, ssao_noise.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  ssao_pipe_.LoadShader("data/shaders/Final/screen_tex.vert",
                        "data/shaders/Final/ssao.frag");
  ssao_pipe_.LoadProgram();
  ssao_pipe_.Bind();

  ssao_pipe_.SetInt("g_position_metallic", 0);
  ssao_pipe_.SetInt("g_normal_roughness", 1);
  ssao_pipe_.SetInt("texNoise", 2);
  ssao_pipe_.SetFloat("radius", kSsaoRadius);
  ssao_pipe_.SetFloat("biais", kSsaoBiais);

  ssao_blur_pipe_.LoadShader("data/shaders/Final/screen_tex.vert",
                             "data/shaders/Final/ssao_blur.frag");
  ssao_blur_pipe_.LoadProgram();
  ssao_blur_pipe_.Bind();

  ssao_blur_pipe_.SetInt("ssao_tex", 0);

  glGenFramebuffers(1, &ssao_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo_);

  glGenTextures(1, &ssao_tex_);
  glBindTexture(GL_TEXTURE_2D, ssao_tex_);
  // As the ambient occlusion result is a single grayscale value we'll only need
  // a texture's red component, so we set the color buffer's internal format to
  // GL_RED.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Metrics::width_, Metrics::height_, 0,
               GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssao_tex_, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &ssao_blur_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo_);
  glGenTextures(1, &ssao_blur_tex_);
  glBindTexture(GL_TEXTURE_2D, ssao_blur_tex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Metrics::width_, Metrics::height_, 0,
               GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssao_blur_tex_, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::UpdateSSAO() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo_);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ssao_pipe_.Bind();
  // Send kernel + rotation
  for (unsigned int i = 0; i < 64; ++i) {
    ssao_pipe_.SetVec3Position("samples[" + std::to_string(i) + "]",
                               ssao_kernel_[i]);
  }

  ssao_pipe_.SetMat4("projection", projection);
  ssao_pipe_.SetVec2("noiseScale",
                     glm::vec2(Metrics::width_ / 4.f, Metrics::height_ / 4.f));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pos_map_);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal_map_);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, noise_texture_);

  quad_screen_.Draw();

  // SSAO blur.
  // ----------
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo_);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ssao_blur_pipe_.Bind();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ssao_tex_);

  quad_screen_.Draw();
}

void FinalScene::DeleteSSAO() {
  ssao_pipe_.Delete();
  ssao_blur_pipe_.Delete();
  noise_texture_ = 0;
  glDeleteFramebuffers(1, &ssao_fbo_);
  glDeleteFramebuffers(1, &ssao_blur_fbo_);
  ssao_fbo_ = 0;
  ssao_blur_fbo_ = 0;
  ssao_tex_ = 0;
  ssao_blur_tex_ = 0;
}

void FinalScene::BeginShadowMap() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  shadow_map_pipe_.LoadShader("data/shaders/Final/depth.vert",
                              "data/shaders/Final/depth.frag");
  shadow_map_pipe_.LoadProgram();
  shadow_map_pipe_.Bind();

  shadow_map_pipe_.SetVec3Position("lightPos", lamp_pos_);
  shadow_map_pipe_.SetFloat("lightFarPlane", kLightFarPlane);

  // Point Shadow Cubemap Framebuffer.
  // ---------------------------------
  glGenFramebuffers(1, &shadow_fbo_);
  // create depth cubemap texture
  glGenTextures(1, &shadow_tex_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_tex_);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
                 shadow_tex_res_, shadow_tex_res_, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
  GLenum drawBuffers[] = {GL_NONE};
  glDrawBuffers(1, drawBuffers);
  glReadBuffer(GL_NONE);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         GL_TEXTURE_CUBE_MAP_POSITIVE_X, shadow_tex_, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  /////////

  glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, shadow_tex_res_, shadow_tex_res_);

  shadow_map_pipe_.Bind();

  glm::mat4 lightProjection, lightView;

  for (int i = 0; i < 6; ++i) {
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, shadow_tex_, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    lightView = glm::lookAt(lamp_pos_, lamp_pos_ + light_dirs[i], light_ups[i]);
    lightProjection = glm::perspective(glm::radians(90.f), 1.0f,
                                       kLightNearPlane, kLightFarPlane);
    lightSpaceMatrix = lightProjection * lightView;
    shadow_map_pipe_.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    UpdateGround(shadow_map_pipe_);
    UpdateModels(shadow_map_pipe_);
    UpdateSpheres(shadow_map_pipe_);
  }

  glViewport(0, 0, Metrics::width_, Metrics::height_);
}

void FinalScene::DeleteShadowMap() {
  shadow_map_pipe_.Delete();
  glDeleteFramebuffers(1, &shadow_fbo_);
  shadow_fbo_ = 0;
  shadow_tex_ = 0;
}

void FinalScene::BeginPBR() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  pbr_pipe_.LoadShader("data/shaders/Final/screen_tex.vert",
                       "data/shaders/Final/pbr.frag");

  pbr_pipe_.LoadProgram();

  pbr_pipe_.Bind();
  pbr_pipe_.SetInt("irradianceMap", 0);
  pbr_pipe_.SetInt("prefilterMap", 1);
  pbr_pipe_.SetInt("brdfLUT", 2);

  pbr_pipe_.SetInt("g_position_metallic", 3);
  pbr_pipe_.SetInt("g_normal_roughness", 4);
  pbr_pipe_.SetInt("g_albedo_ao", 5);

  pbr_pipe_.SetInt("ssao_tex", 6);

  pbr_pipe_.SetInt("depth_tex", 7);

  pbr_pipe_.SetVec3Position("lightPos", lamp_pos_);
  pbr_pipe_.SetVec3Color("lightColor", light_color_);

  pbr_pipe_.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
  pbr_pipe_.SetFloat("lightFarPlane", kLightFarPlane);
}

void FinalScene::UpdatePBR() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo_);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  pbr_pipe_.Bind();
  pbr_pipe_.SetVec3Position("camPos", camera_.position_);
  pbr_pipe_.SetMat4("inverse_view", glm::inverse(view));
  // set light pos and color todo in futur

  // bind pre-computed IBL data
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, pos_map_);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, normal_map_);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, albedo_map_);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, ssao_blur_tex_);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_tex_);

  quad_screen_.Draw();
}

void FinalScene::DeletePBR() {
  irradiance_pipe_.Delete();
  prefilter_pipe_.Delete();
  brdf_pipe_.Delete();
  pbr_pipe_.Delete();
}

void FinalScene::LoadRessources() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  lamp_model_.Load("data/models/final/lamp/msh_lampadaire_01.obj");
  backpack_model_.Load("data/models/final/backpack/backpack.obj");
  man_model_.Load("data/models/final/man/man1.obj");

  std::array<GLuint*, nb_data> tex_id{
      &lamp_model_.mat.albedo,
      &lamp_model_.mat.normal,
      &lamp_model_.mat.ao,
      &lamp_model_.mat.metallic,
      &lamp_model_.mat.roughness,

      &man_model_.mat.albedo,
      &man_model_.mat.normal,
      &man_model_.mat.ao,
      &man_model_.mat.metallic,
      &man_model_.mat.roughness,

      &steel_.albedo,
      &steel_.normal,
      &steel_.ao,
      &steel_.metallic,
      &steel_.roughness,

      &titanium_.albedo,
      &titanium_.normal,
      &titanium_.ao,
      &titanium_.metallic,
      &titanium_.roughness,

      &ground_mat_.albedo,
      &ground_mat_.normal,
      &ground_mat_.ao,
      &ground_mat_.metallic,
      &ground_mat_.roughness,

      &backpack_model_.mat.albedo,
      &backpack_model_.mat.normal,
      &backpack_model_.mat.ao,
      &backpack_model_.mat.metallic,
      &backpack_model_.mat.roughness,
  };

  std::array<TextureParameters, nb_data> tex_params{
      TextureParameters("data/models/final/lamp/lampBaseColor.png", GL_REPEAT,
                        GL_LINEAR, true, true),
      TextureParameters("data/models/final/lamp/lampNormal.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/models/final/lamp/lampAO.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/models/final/lamp/lampMetallic.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/models/final/lamp/lampRoughness.png", GL_REPEAT,
                        GL_LINEAR, false, true),

      TextureParameters("data/models/final/man/albedo.jpg", GL_REPEAT,
                        GL_LINEAR, true, true),
      TextureParameters("data/models/final/man/normal.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/models/final/man/ao.png", GL_REPEAT, GL_LINEAR,
                        false, true),
      TextureParameters("data/models/final/man/metallic.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/models/final/man/roughness.jpg", GL_REPEAT,
                        GL_LINEAR, false, true),

      TextureParameters("data/textures/pbr/steel/albedo.png", GL_REPEAT,
                        GL_LINEAR, true, true),
      TextureParameters("data/textures/pbr/steel/normal.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/steel/ao.png", GL_REPEAT, GL_LINEAR,
                        false, true),
      TextureParameters("data/textures/pbr/steel/metallic.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/steel/roughness.png", GL_REPEAT,
                        GL_LINEAR, false, true),

      TextureParameters("data/textures/pbr/titanium/albedo.png", GL_REPEAT,
                        GL_LINEAR, true, true),
      TextureParameters("data/textures/pbr/titanium/normal.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/titanium/ao.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/titanium/metallic.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/titanium/roughness.png", GL_REPEAT,
                        GL_LINEAR, false, true),

      TextureParameters("data/textures/pbr/stonework/albedo.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/stonework/normal.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/stonework/ao.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/stonework/metallic.png", GL_REPEAT,
                        GL_LINEAR, false, true),
      TextureParameters("data/textures/pbr/stonework/roughness.png", GL_REPEAT,
                        GL_LINEAR, false, true),

      TextureParameters("data/models/final/backpack/diffuse.jpg", GL_REPEAT,
                        GL_LINEAR, true, false),
      TextureParameters("data/models/final/backpack/normal.png", GL_REPEAT,
                        GL_LINEAR, false, false),
      TextureParameters("data/models/final/backpack/ao.jpg", GL_REPEAT,
                        GL_LINEAR, false, false),
      TextureParameters("data/models/final/backpack/specular.jpg", GL_REPEAT,
                        GL_LINEAR, false, false),
      TextureParameters("data/models/final/backpack/roughness.jpg", GL_REPEAT,
                        GL_LINEAR, false, false),
  };

  read_jobs_.reserve(nb_data);
  decom_jobs_.reserve(nb_data);
  gpu_jobs_.reserve(nb_data);

  for (int i = 0; i < nb_data; ++i) {
    const auto& tex_param = tex_params[i];

    read_jobs_.emplace_back(tex_param.image_file_path, &fbArray[i]);

    decom_jobs_.emplace_back(&fbArray[i], &textures[i], tex_param.flipped_y);
    decom_jobs_[i].AddDependency(&read_jobs_[i]);

    gpu_jobs_.emplace_back(&textures[i], tex_id[i], tex_param);
    gpu_jobs_[i].AddDependency(&decom_jobs_[i]);

    job_system_.AddJob(&read_jobs_[i]);
    job_system_.AddJob(&decom_jobs_[i]);
    main_thread_jobs_.push(&gpu_jobs_[i]);

    // do not push back jobs otherwise it will explode
  }

  job_system_.LaunchWorkers(2);
}

void FinalScene::UpdateModels(Pipeline& pipeline) {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  lamp_model_.mat.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0, 0, -10));
  model = glm::scale(model, glm::vec3(0.05));
  model = glm::translate(model, glm::vec3(0, 10, 0));
  pipeline.Bind();
  pipeline.SetMat4("model", model);

  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  lamp_model_.Draw();

  backpack_model_.mat.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0, 2.2, 0));
  model = glm::rotate(model, glm::radians(180.f), glm::vec3(0, 1, 0));
  pipeline.Bind();
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  backpack_model_.Draw();

  man_model_.mat.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(3, 0.5, -2));
  model = glm::scale(model, glm::vec3(0.025f));
  model = glm::rotate(model, glm::radians(180.f), glm::vec3(0, 1, 0));
  pipeline.Bind();
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  man_model_.Draw();

  steel_.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(6, 0.5, -8));
  model = glm::scale(model, glm::vec3(0.025f));
  model = glm::rotate(model, glm::radians(-110.f), glm::vec3(0, 1, 0));
  pipeline.Bind();
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  man_model_.Draw();

  titanium_.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-6, 0.5, -5));
  model = glm::scale(model, glm::vec3(0.025f));
  model = glm::rotate(model, glm::radians(110.f), glm::vec3(0, 1, 0));
  pipeline.Bind();
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  man_model_.Draw();
}

void FinalScene::DeleteModels() {
  lamp_model_.Clear();
  man_model_.Clear();
  backpack_model_.Clear();

  steel_.Clear();
  titanium_.Clear();
}

void FinalScene::UpdateSpheres(Pipeline& pipeline) {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  steel_.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(5.2, 1.8, -6));
  pipeline.Bind();
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  sphere_.Draw(true);

  titanium_.Set();

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-5, 1.8, -9));

  pipeline.Bind();
  pipeline.SetMat4("model", model);
  pipeline.SetMat4("normalMatrix",
                   glm::mat4(glm::transpose(glm::inverse(view * model))));

  sphere_.Draw(true);
}

void FinalScene::BeginBloom() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  hdr_pipe_.LoadShader("data/shaders/final/screen_tex.vert",
                       "data/shaders/final/hdr.frag");
  hdr_pipe_.LoadProgram();

  hdr_pipe_.Bind();
  hdr_pipe_.SetInt("hdrBuffer", 0);
  hdr_pipe_.SetInt("bloomBlur", 1);

  down_sample_pipe_.LoadShader("data/shaders/final/screen_tex.vert",
                               "data/shaders/final/down_sample.frag");

  down_sample_pipe_.LoadProgram();

  down_sample_pipe_.Bind();
  down_sample_pipe_.SetInt("srcTexture", 0);

  up_sample_pipe_.LoadShader("data/shaders/final/screen_tex.vert",
                             "data/shaders/final/up_sample.frag");

  up_sample_pipe_.LoadProgram();

  up_sample_pipe_.Bind();
  up_sample_pipe_.SetInt("srcTexture", 0);

  constexpr auto mip_chain_length = 5;
  glGenFramebuffers(1, &bloom_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo_);

  glm::vec2 mip_size(Metrics::width_, Metrics::height_);
  glm::ivec2 mip_int_size(Metrics::width_, Metrics::height_);

  bloom_mips_.reserve(mip_chain_length);

  for (unsigned int i = 0; i < mip_chain_length; i++) {
    BloomMip mip;

    mip_size *= 0.5f;
    mip_int_size /= 2;
    mip.size = mip_size;

    glGenTextures(1, &mip.texture);
    glBindTexture(GL_TEXTURE_2D, mip.texture);
    // we are downscaling an HDR color buffer, so we need a float texture
    // format
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mip_size.x, mip_size.y, 0, GL_RGB,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    bloom_mips_.push_back(mip);
  }
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         bloom_mips_[0].texture, 0);

  // setup attachments
  constexpr std::array<GLuint, 1> attachments = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments.data());

  // check completion status
  int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "G-buffer FBO error, status : " << status << '\n';
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &hdr_fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo_);

  glGenTextures(1, &scene_tex_);
  glBindTexture(GL_TEXTURE_2D, scene_tex_);

  // Specify texture storage and parameters
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Metrics::width_, Metrics::height_,
               0, GL_RGBA, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         scene_tex_, 0);

  glGenTextures(1, &bright_tex_);

  glBindTexture(GL_TEXTURE_2D, bright_tex_);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Metrics::width_, Metrics::height_,
               0, GL_RGBA, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         bright_tex_, 0);

  glGenRenderbuffers(1, &hdr_rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, hdr_rbo_);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Metrics::width_,
                        Metrics::height_);

  // Attach renderbuffer to framebuffer
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, hdr_rbo_);

  // Specify the color attachments to be drawn
  constexpr std::array<GLuint, 2> hdr_attachments = {GL_COLOR_ATTACHMENT0,
                                                     GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, hdr_attachments.data());

  // Check framebuffer completeness
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  // Unbind the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalScene::UpdateBloom() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo_);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  down_sample_pipe_.Bind();

  down_sample_pipe_.SetVec2("srcResolution",
                            glm::vec2(Metrics::width_, Metrics::height_));

  // Bind srcTexture (HDR color buffer) as initial texture input
  glBindTexture(GL_TEXTURE_2D, bright_tex_);
  glActiveTexture(GL_TEXTURE0);

  // Progressively downsample through the mip chain.
  for (int i = 0; i < bloom_mips_.size(); i++) {
    const BloomMip& mip = bloom_mips_[i];

    glViewport(0, 0, mip.size.x, mip.size.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mip.texture, 0);

    // Render screen-filled quad of resolution of current mip
    quad_screen_.Draw();

    // Set current mip resolution as srcResolution for next iteration
    down_sample_pipe_.SetVec2("srcResolution", mip.size);
    // Set current mip as texture input for next iteration
    glBindTexture(GL_TEXTURE_2D, mip.texture);
  }

  up_sample_pipe_.Bind();
  up_sample_pipe_.SetFloat("filterRadius", 0.007);

  // Enable additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  for (int i = bloom_mips_.size() - 1; i > 0; i--) {
    const BloomMip& mip = bloom_mips_[i];
    const BloomMip& nextMip = bloom_mips_[i - 1];

    // Bind viewport and texture from where to read
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mip.texture);

    // Set framebuffer render target (we write to this texture)
    glViewport(0, 0, nextMip.size.x, nextMip.size.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           nextMip.texture, 0);

    // Render screen-filled quad of resolution of current mip
    quad_screen_.Draw();
  }

  // Disable additive blending
  // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
  glDisable(GL_BLEND);

  glViewport(0, 0, Metrics::width_, Metrics::height_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER);
  hdr_pipe_.Bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene_tex_);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, bloom_mips_[0].texture);
  hdr_pipe_.SetFloat("bloomStrength", 0.04f);

  quad_screen_.Draw();
}

void FinalScene::DeleteBloom() {
  for (int i = 0; i < bloom_mips_.size(); i++) {
    glDeleteTextures(1, &bloom_mips_[i].texture);
    bloom_mips_[i].texture = 0;
  }
  glDeleteFramebuffers(1, &bloom_fbo_);
  bloom_fbo_ = 0;
  bloom_mips_.clear();
  hdr_pipe_.Delete();
  up_sample_pipe_.Delete();
  down_sample_pipe_.Delete();
}

void FinalScene::DrawImgui() {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif
  if (is_initialized_) {
    ImGui::TextWrapped("CONTROLS:");
    ImGui::TextWrapped("W - move forward");
    ImGui::TextWrapped("S - move backward");
    ImGui::TextWrapped("A - move left");
    ImGui::TextWrapped("D - move right");
    ImGui::Spacing();
    ImGui::TextWrapped("L CTRL - move down");
    ImGui::TextWrapped("SPACE - move up");
    ImGui::Spacing();
    ImGui::TextWrapped("LEFT MOUSE CLICK AND MOVE MOUSE - move camera");
  } else {
    ImGui::TextWrapped("Loading...");
  }
}
