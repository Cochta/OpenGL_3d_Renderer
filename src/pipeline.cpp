#include "pipeline.h"

void Pipeline::Bind() {
  if (program_ == 0) {
    std::cerr << "Error while loading Pipeline\n";
    return;
  }
  glUseProgram(program_);
  current_program_ = program_;
}

void Pipeline::Delete() {
  glDeleteProgram(program_);
  glDeleteShader(vertex_shader_);
  glDeleteShader(fragment_shader_);
}

void Pipeline::SetInt(std::string_view name, int value) {
  if (program_ != current_program_) {
    std::cerr << "Wrong Pipeline binded to set int\n";
    return;
  }
  glUniform1i(glGetUniformLocation(program_, name.data()), value);
}

void Pipeline::SetFloat(std::string_view name, float value) {
  if (program_ != current_program_) {
    std::cerr << "Wrong Pipeline binded to set float\n";
    return;
  }
  glUniform1f(glGetUniformLocation(program_, name.data()), value);
}
void Pipeline::SetMat4(std::string_view name, glm::mat4 matrix) {
  if (program_ != current_program_) {
    std::cerr << "Wrong Pipeline binded to set matrix 4\n";
    return;
  }
  glUniformMatrix4fv(glGetUniformLocation(program_, name.data()), 1, GL_FALSE,
                     glm::value_ptr(matrix));
}
void Pipeline::SetVec2(std::string_view name, glm::vec2 vec2) {
  if (program_ != current_program_) {
    std::cerr << "Wrong Pipeline binded to set vector 2\n";
    return;
  }
  glUniform2f(glGetUniformLocation(program_, name.data()), vec2.x, vec2.y);
}
void Pipeline::SetVec3Color(std::string_view name, glm::vec3 vec3) {
  if (program_ != current_program_) {
    std::cerr << "Wrong Pipeline binded to set vector 3 color\n";
    return;
  }
  glUniform3f(glGetUniformLocation(program_, name.data()), vec3.r, vec3.g,
              vec3.b);
}
void Pipeline::SetVec3Position(std::string_view name, glm::vec3 vec3) {
  if (program_ != current_program_) {
    std::cerr << "Wrong Pipeline binded to set vector 3 position\n";
    return;
  }
  glUniform3f(glGetUniformLocation(program_, name.data()), vec3.x, vec3.y,
              vec3.z);
}

void Pipeline::LoadShader(std::string_view vert_path,
                          std::string_view frag_path) {
  const auto vertexContent = LoadFile(vert_path);
  const auto *ptr = vertexContent.data();
  vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_, 1, &ptr, nullptr);
  glCompileShader(vertex_shader_);

  // Check success status of shader compilation
  GLint success;
  glGetShaderiv(vertex_shader_, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::cerr << "Error while loading vertex shader\n";
    return;
  }

  const auto fragmentContent = LoadFile(frag_path);
  ptr = fragmentContent.data();

  fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_, 1, &ptr, nullptr);
  glCompileShader(fragment_shader_);

  glGetShaderiv(fragment_shader_, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::cerr << "Error while loading fragment shader\n";
  }
}

void Pipeline::LoadProgram() {
  // Load program/pipeline
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader_);
  glAttachShader(program_, fragment_shader_);
  glLinkProgram(program_);
  // Check if shader program was linked correctly
  GLint success;
  glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    std::cerr << "Error while linking shader program\n";
  }
}
