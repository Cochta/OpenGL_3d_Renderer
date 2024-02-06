#include "texture_manager.h"

#include <stb_image.h>

#include <iostream>

TextureManager::TextureManager() {}

GLuint TextureManager::LoadTexture(std::string_view path, bool flip, bool pbr) {
  stbi_set_flip_vertically_on_load(flip);
  GLuint texture = 0;
  int width, height, nr_channels;
  unsigned char *data =
      stbi_load(path.data(), &width, &height, &nr_channels, 0);
  if (data == nullptr) {  // early exit
    std::cerr << "Failed to load image " << path.data() << "\n";
    return texture;
  }

  std::cout << path << " width : " << width << " height : " << height
            << " Channels : " << nr_channels << "\n";

  // Generate texture
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int internal_format, format;
  if (nr_channels == 3) {
    if (pbr) {
      internal_format = GL_SRGB;
    } else {
      internal_format = GL_RGB;
    }
    format = GL_RGB;
  } else if (nr_channels == 4) {
    if (pbr) {
      internal_format = GL_SRGB_ALPHA;
    } else {
      internal_format = GL_RGBA;
    }
    format = GL_RGBA;
  } else if (nr_channels == 1) {
    internal_format = GL_RED;
    format = GL_RED;
  } else if (nr_channels == 2) {
    internal_format = GL_RG;
    format = GL_RG;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
  return texture;
}

GLuint TextureManager::LoadCubeMap(std::string path,
                                   std::vector<std::string> faces, bool flip) {
  stbi_set_flip_vertically_on_load(flip);
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (GLuint i = 0; i < faces.size(); i++) {
    std::string face = path + faces[i];
    unsigned char *data =
        stbi_load(face.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      auto format = nrChannels == 4 ? GL_RGBA : GL_RGB;
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height,
                   0, format, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap tex failed to load at path: " << face << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

GLuint TextureManager::LoadHDR(std::string_view path, bool flip) {
  // pbr: load the HDR environment map
  // ---------------------------------
  stbi_set_flip_vertically_on_load(flip);
  int width, height, nrComponents;
  float *data = stbi_loadf(path.data(), &width, &height, &nrComponents, 0);
  GLuint hdrTexture;
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT,
        data);  // note how we specify the texture's data value to be float

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Failed to load HDR image." << std::endl;
  }
  return hdrTexture;
}
