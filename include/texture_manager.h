#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>

#include <string>
#include <vector>

struct Texture {
  GLuint id;
  std::string type;
  std::string path;
};

class TextureManager {
 public:
  TextureManager();
  GLuint LoadTexture(std::string_view path, bool flip = true, bool pbr = false);
  GLuint LoadCubeMap(std::string path, std::vector<std::string> faces,
                     bool flip = false);
  GLuint LoadHDR(std::string_view path, bool flip = true);

  std::vector<Texture> loaded_textures;
};