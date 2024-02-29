#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>

#include <string>
#include <vector>

#include "JobSystem.h"
#include "file_utility.h"

struct FileData {
  int width, height, nr_channels;
  unsigned char* data;
};

class TextureManager {
 public:
  TextureManager() noexcept = default;
  GLuint LoadTexture(std::string_view path, bool flip = true, bool pbr = false);
  GLuint LoadCubeMap(std::string path, std::vector<std::string> faces,
                     bool flip = false);
  GLuint LoadHDR(std::string_view path, bool flip = true);
};

class ReadJob final : public Job {
 public:
  ReadJob(std::string path, FileBuffer* file_buffer) noexcept;

  ReadJob(ReadJob&& other) noexcept;
  ReadJob& operator=(ReadJob&& other) noexcept;
  ReadJob(const ReadJob& other) noexcept = delete;
  ReadJob& operator=(const ReadJob& other) noexcept = delete;

  ~ReadJob() noexcept;

  void Work() noexcept override;

  FileBuffer* file_buffer{};
  std::string file_path{};
};

struct TextureBuffer {
  unsigned char* data;
  int width = 0, height = 0, channels = 0;
};

class DecompressJob final : public Job {
 public:
  DecompressJob(FileBuffer* file_buffer, TextureBuffer* texture,
                bool flip = true) noexcept;

  DecompressJob(DecompressJob&& other) noexcept;
  DecompressJob& operator=(DecompressJob&& other) noexcept;
  DecompressJob(const DecompressJob& other) noexcept = delete;
  DecompressJob& operator=(const DecompressJob& other) noexcept = delete;

  ~DecompressJob() noexcept;

  void Work() noexcept override;

 private:
  FileBuffer* file_buffer_;
  TextureBuffer* texture_{};
  bool flip_ = true;
};

struct TextureParameters {
  TextureParameters() noexcept = default;
  TextureParameters(std::string path, GLint wrap_param, GLint filter_param,
                    bool gamma, bool flip_y, bool hdr = false) noexcept;

  std::string image_file_path{};
  GLint wrapping_param = GL_REPEAT;
  GLint filtering_param = GL_LINEAR;
  bool gamma_corrected = false;
  bool flipped_y = false;
  bool hdr = false;
};

class UploadGpuJob final : public Job {
 public:
  UploadGpuJob(TextureBuffer* image_buffer, GLuint* texture_id,
               const TextureParameters& tex_param) noexcept;

  UploadGpuJob(UploadGpuJob&& other) noexcept;
  UploadGpuJob& operator=(UploadGpuJob&& other) noexcept;
  UploadGpuJob(const UploadGpuJob& other) noexcept = delete;
  UploadGpuJob& operator=(const UploadGpuJob& other) noexcept = delete;

  ~UploadGpuJob() noexcept override;

  void Work() noexcept override;

 private:
  // Shared with the image decompressing job.
  TextureBuffer* image_buffer_ = nullptr;
  GLuint* texture_id_ = nullptr;
  TextureParameters texture_param_;
};
