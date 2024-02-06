#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "file_utility.h"
#include "mesh.h"

class Pipeline {
 public:
  void Bind();
  void Delete();

  void SetInt(std::string_view name, int value);
  void SetFloat(std::string_view name, float value);
  void SetMat4(std::string_view name, glm::mat4 matrix);
  void SetVec2(std::string_view name, glm::vec2 vec2);
  void SetVec3Color(std::string_view name, glm::vec3 vec3);
  void SetVec3Position(std::string_view name, glm::vec3 vec3);

  void LoadShader(std::string_view vert_path, std::string_view frag_path);

  void LoadProgram();

 private:
  GLuint vertex_shader_ = 0;
  GLuint fragment_shader_ = 0;
  GLuint program_ = 0;

  inline static GLuint current_program_ = 0;
};