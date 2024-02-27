#include "texture_manager.h"

#include <stb_image.h>

#include <iostream>

#include "file_utility.h"

#ifdef TRACY_ENABLE
#include <TracyC.h>

#include <Tracy.hpp>
#endif

TextureManager::TextureManager() {}

GLuint TextureManager::LoadTexture(std::string_view path, bool flip, bool pbr) {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif

  // Set STBI flip_ option
  stbi_set_flip_vertically_on_load(flip);

  // Load image data
  int width, height, nr_channels;
  unsigned char* data =
      stbi_load(path.data(), &width, &height, &nr_channels, 0);
  if (data == nullptr) {
    std::cerr << "Failed to load image " << path.data() << "\n";
    return 0;  // No need to assign texture before returning
  }

  // Log image information
  std::cout << path << " width : " << width << " height : " << height
            << " Channels : " << nr_channels << "\n";

  // Generate texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Determine internal format and format based on number of channels
  GLint internal_format, format;

  switch (nr_channels) {
    case 1:
      internal_format = GL_RED;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG;
      format = GL_RG;
      break;
    case 3:
      internal_format = pbr ? GL_SRGB : GL_RGB;
      format = GL_RGB;
      break;
    case 4:
      internal_format = pbr ? GL_SRGB_ALPHA : GL_RGBA;
      format = GL_RGBA;
      break;
    default:
      std::cerr << "Unsupported number of channels: " << nr_channels << "\n";
      stbi_image_free(data);
      return 0;
  }

  // Set texture data
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free image data
  stbi_image_free(data);

  return texture;
}

// GLuint TextureManager::LoadTextureAsync(std::string_view path, bool flip,
//                                         bool pbr) {
// #ifdef TRACY_ENABLE
//   ZoneScoped;
// #endif
//
//   // Set STBI flip_ option
//   stbi_set_flip_vertically_on_load(flip);
//
//   auto fileBuffer = LoadFile(path);
//
//   auto fileData = Decompress(fileBuffer);
//
//   return UpGPU(fileData, flip, pbr);
// }

GLuint TextureManager::LoadCubeMap(std::string path,
                                   std::vector<std::string> faces, bool flip) {
  stbi_set_flip_vertically_on_load(flip);
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (GLuint i = 0; i < faces.size(); i++) {
    std::string face = path + faces[i];
    unsigned char* data =
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
  float* data = stbi_loadf(path.data(), &width, &height, &nrComponents, 0);
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
//
// FileBuffer TextureManager::LoadFile(std::string_view path) {
// #ifdef TRACY_ENABLE
//  ZoneScoped;
// #endif
//  // Read the image file into memory
//  FILE* file = fopen(path.data(), "rb");
//  if (!file) {
//    std::cerr << "Error: Failed to open file." << std::endl;
//  }
//
//  fseek(file, 0, SEEK_END);
//  long fileSize = ftell(file);
//  fseek(file, 0, SEEK_SET);
//
//  std::vector<unsigned char> fileData(fileSize);
//  fread(fileData.data(), 1, fileSize, file);
//  fclose(file);
//  return {fileData, fileSize};
//}
//
// FileData TextureManager::Decompress(FileBuffer fileBuffer) {
// #ifdef TRACY_ENABLE
//  ZoneScoped;
// #endif
//  // Decode the image
//  FileData fd;
//  fd.data =
//      stbi_load_from_memory(fileBuffer.fileData.data(), fileBuffer.fileSize,
//                            &fd.width, &fd.height, &fd.nr_channels, 0);
//  return fd;
//}
//
// GLuint TextureManager::UpGPU(FileData fileData, bool flip, bool pbr) {
//  if (!fileData.data) {
//    std::cerr << "Error: Failed to decode image." << std::endl;
//    return 1;
//  }
//
//  // Generate texture
//  GLuint texture;
//  glGenTextures(1, &texture);
//  glBindTexture(GL_TEXTURE_2D, texture);
//
//  // Set texture parameters
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//                  GL_LINEAR_MIPMAP_LINEAR);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//  // Determine internal format and format based on number of channels
//  GLint internal_format, format;
//
//  switch (fileData.nr_channels) {
//    case 1:
//      internal_format = GL_RED;
//      format = GL_RED;
//      break;
//    case 2:
//      internal_format = GL_RG;
//      format = GL_RG;
//      break;
//    case 3:
//      internal_format = pbr ? GL_SRGB : GL_RGB;
//      format = GL_RGB;
//      break;
//    case 4:
//      internal_format = pbr ? GL_SRGB_ALPHA : GL_RGBA;
//      format = GL_RGBA;
//      break;
//    default:
//      std::cerr << "Unsupported number of channels: " << fileData.nr_channels
//                << "\n";
//      stbi_image_free(fileData.data);
//      return 0;
//  }
//
//  // Set texture data
//  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, fileData.width,
//               fileData.height, 0, format, GL_UNSIGNED_BYTE, fileData.data);
//  glGenerateMipmap(GL_TEXTURE_2D);
//
//  // Free image data
//  stbi_image_free(fileData.data);
//
//  return texture;
//}

ImageFileReadingJob::ImageFileReadingJob(
    std::string path, std::shared_ptr<FileBuffer> file_buffer) noexcept
    : Job(JobType::kFileReading), file_path(std::move(path)), file_buffer(file_buffer) {}

ImageFileReadingJob::ImageFileReadingJob(ImageFileReadingJob&& other) noexcept
    : Job(std::move(other)) {
  file_buffer = std::move(other.file_buffer);
  file_path = std::move(other.file_path);

  other.file_buffer = nullptr;
}
ImageFileReadingJob& ImageFileReadingJob::operator=(
    ImageFileReadingJob&& other) noexcept {
  Job::operator=(std::move(other));
  file_buffer = std::move(other.file_buffer);
  file_path = std::move(other.file_path);

  other.file_buffer = nullptr;

  return *this;
}

ImageFileReadingJob::~ImageFileReadingJob() noexcept { file_buffer = nullptr; }

void ImageFileReadingJob::Work() noexcept {
#ifdef TRACY_ENABLE
  ZoneScoped;
  ZoneText(file_path.data(), file_path.size());
#endif  // TRACY_ENABLE

  LoadFileInBuffer(file_path.data(), file_buffer.get());
}

ImageFileDecompressingJob::ImageFileDecompressingJob(
    std::shared_ptr<FileBuffer> file_buffer,
    std::shared_ptr<TextureGpu> texture, bool flip) noexcept
    : Job(JobType::kFileDecompressing),
      file_buffer_(file_buffer),
      texture_(texture),
      flip_(flip) {}

ImageFileDecompressingJob::ImageFileDecompressingJob(
    ImageFileDecompressingJob&& other) noexcept
    : Job(std::move(other)) {
  file_buffer_ = std::move(other.file_buffer_);
  texture_ = std::move(other.texture_);

  other.file_buffer_ = nullptr;
  other.texture_ = nullptr;
}

ImageFileDecompressingJob& ImageFileDecompressingJob::operator=(
    ImageFileDecompressingJob&& other) noexcept {
  Job::operator=(std::move(other));
  file_buffer_ = std::move(other.file_buffer_);
  texture_ = std::move(other.texture_);

  other.file_buffer_ = nullptr;
  other.texture_ = nullptr;

  return *this;
}

ImageFileDecompressingJob::~ImageFileDecompressingJob() noexcept {
  file_buffer_ = nullptr;
  texture_ = nullptr;
}

void ImageFileDecompressingJob::Work() noexcept {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif  // TRACY_ENABLE
  stbi_set_flip_vertically_on_load(flip_);

  texture_->data = stbi_load_from_memory(
      file_buffer_.get()->data, file_buffer_.get()->size, &texture_->width,
      &texture_->height, &texture_->channels, 0);
}

TextureParameters::TextureParameters(std::string path, GLint wrap_param,
                                     GLint filter_param, bool gamma,
                                     bool flip_y, bool hdr) noexcept
    : image_file_path(path),
      wrapping_param(wrap_param),
      filtering_param(filter_param),
      gamma_corrected(gamma),
      flipped_y(flip_y),
      hdr(hdr){};

LoadTextureToGpuJob::LoadTextureToGpuJob(
    std::shared_ptr<TextureGpu> image_buffer, GLuint* texture_id,
    const TextureParameters& tex_param) noexcept
    : Job(JobType::kDownloadingDataToGpu),
      image_buffer_(image_buffer),
      texture_id_(texture_id),
      texture_param_(tex_param) {}

LoadTextureToGpuJob::LoadTextureToGpuJob(LoadTextureToGpuJob&& other) noexcept
    : Job(std::move(other)) {
  image_buffer_ = std::move(other.image_buffer_);
  texture_id_ = std::move(other.texture_id_);
  texture_param_ = other.texture_param_;
}

LoadTextureToGpuJob& LoadTextureToGpuJob::operator=(
    LoadTextureToGpuJob&& other) noexcept {
  Job::operator=(std::move(other));
  image_buffer_ = std::move(other.image_buffer_);
  texture_id_ = std::move(other.texture_id_);
  texture_param_ = other.texture_param_;

  return *this;
}

LoadTextureToGpuJob::~LoadTextureToGpuJob() noexcept {
  image_buffer_ = nullptr;
  texture_id_ = nullptr;
}

void LoadTextureToGpu(std::shared_ptr<TextureGpu> image_buffer, GLuint* id,
                      const TextureParameters& tex_param) noexcept {
  glGenTextures(1, id);
  glBindTexture(GL_TEXTURE_2D, *id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex_param.wrapping_param);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex_param.wrapping_param);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  tex_param.filtering_param);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  tex_param.filtering_param);

  GLint internal_format = GL_RGB;
  GLenum format = GL_RGB;

  switch (image_buffer->channels) {
    case 1:
      internal_format = GL_RED;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG;
      format = GL_RG;
      break;
    case 3:
      if (tex_param.hdr) {
        internal_format = GL_RGB16F;
      } else {
        internal_format = tex_param.gamma_corrected ? GL_SRGB : GL_RGB;
      }
      format = GL_RGB;
      break;
    case 4:
      internal_format = tex_param.gamma_corrected ? GL_SRGB_ALPHA : GL_RGBA;
      format = GL_RGBA;
      break;
    default:
      break;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image_buffer->width,
               image_buffer->height, 0, format, GL_UNSIGNED_BYTE,
               image_buffer->data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(image_buffer->data);
}

void LoadTextureToGpuJob::Work() noexcept {
#ifdef TRACY_ENABLE
  ZoneScoped;
#endif  // TRACY_ENABLE
  LoadTextureToGpu(image_buffer_, texture_id_, texture_param_);
}
