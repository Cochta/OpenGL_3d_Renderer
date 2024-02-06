#pragma once

// #include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Direction { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// Default camera values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 10.0f;
constexpr float SENSITIVITY = 0.2f;
constexpr float ZOOM = 45.0f;

class Camera {
 public:
  glm::vec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;
  glm::vec3 right_;
  glm::vec3 world_up_;

  float yaw_;
  float pitch_;

  float speed_;
  float sensitivity_;
  float zoom_;

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH);

  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch);

  glm::mat4 GetViewMatrix();

  void ProcessKeyboard(Direction direction, float deltaTime);

  void ProcessMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true);

  void ProcessMouseScroll(float yoffset);

 private:
  void updateCameraVectors();
};