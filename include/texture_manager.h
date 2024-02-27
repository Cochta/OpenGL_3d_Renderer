#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "file_utility.h"

//struct FileBuffer {
//  std::vector<unsigned char> fileData;
//  long fileSize;
//};

struct FileData {
  int width, height, nr_channels;
  unsigned char* data;
};

class TextureManager {
 public:
  TextureManager();
  GLuint LoadTexture(std::string_view path, bool flip = true, bool pbr = false);
  GLuint LoadTextureAsync(std::string_view path, bool flip = true,
                          bool pbr = false);
  GLuint LoadCubeMap(std::string path, std::vector<std::string> faces,
                     bool flip = false);
  GLuint LoadHDR(std::string_view path, bool flip = true);

  //FileBuffer LoadFile(std::string_view path);
  //FileData Decompress(FileBuffer fileBuffer);
  //GLuint UpGPU(FileData fileData, bool flip, bool pbr);
};

class ImageFileReadingJob final : public Job {
 public:
  ImageFileReadingJob(std::string path, std::shared_ptr<FileBuffer> file_buffer) noexcept;

  ImageFileReadingJob(ImageFileReadingJob&& other) noexcept;
  ImageFileReadingJob& operator=(ImageFileReadingJob&& other) noexcept;
  ImageFileReadingJob(const ImageFileReadingJob& other) noexcept = delete;
  ImageFileReadingJob& operator=(const ImageFileReadingJob& other) noexcept =
      delete;

  ~ImageFileReadingJob() noexcept;

  void Work() noexcept override;

  std::shared_ptr<FileBuffer> file_buffer{};
  std::string file_path{};
};

struct TextureGpu {
  unsigned char* data;
  int width = 0, height = 0, channels = 0;
};

class ImageFileDecompressingJob final : public Job {
 public:
  ImageFileDecompressingJob(std::shared_ptr<FileBuffer> file_buffer,
                            std::shared_ptr<TextureGpu> texture,
                            bool flip = true) noexcept;

  ImageFileDecompressingJob(ImageFileDecompressingJob&& other) noexcept;
  ImageFileDecompressingJob& operator=(
      ImageFileDecompressingJob&& other) noexcept;
  ImageFileDecompressingJob(const ImageFileDecompressingJob& other) noexcept =
      delete;
  ImageFileDecompressingJob& operator=(
      const ImageFileDecompressingJob& other) noexcept = delete;

  ~ImageFileDecompressingJob() noexcept;

  void Work() noexcept override;

 private:
  std::shared_ptr<FileBuffer> file_buffer_;
  std::shared_ptr<TextureGpu> texture_{};
  bool flip_ = true;
};

struct TextureParameters {
  TextureParameters() noexcept = default;
  TextureParameters(std::string path, GLint wrap_param, GLint filter_param,
                    bool gamma, bool flip_y, bool hdr = false) noexcept;

  std::string image_file_path{};
  GLint wrapping_param = GL_CLAMP_TO_EDGE;
  GLint filtering_param = GL_LINEAR;
  bool gamma_corrected = false;
  bool flipped_y = false;
  bool hdr = false;
};

class LoadTextureToGpuJob final : public Job {
 public:
  LoadTextureToGpuJob(std::shared_ptr<TextureGpu> image_buffer,
                      GLuint* texture_id,
                      const TextureParameters& tex_param) noexcept;

  LoadTextureToGpuJob(LoadTextureToGpuJob&& other) noexcept;
  LoadTextureToGpuJob& operator=(LoadTextureToGpuJob&& other) noexcept;
  LoadTextureToGpuJob(const LoadTextureToGpuJob& other) noexcept = delete;
  LoadTextureToGpuJob& operator=(const LoadTextureToGpuJob& other) noexcept =
      delete;

  ~LoadTextureToGpuJob() noexcept override;

  void Work() noexcept override;

 private:
  // Shared with the image decompressing job.
  std::shared_ptr<TextureGpu> image_buffer_ = nullptr;
  GLuint* texture_id_ = nullptr;
  TextureParameters texture_param_;
};

