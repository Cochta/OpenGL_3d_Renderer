#pragma once

#include <fstream>

#include <string>
#include <string_view>

#include <vector>

std::string LoadFile(std::string_view path);


struct FileBuffer {
  FileBuffer() noexcept = default;
  FileBuffer(FileBuffer&& other) noexcept;
  FileBuffer& operator=(FileBuffer&& other) noexcept;
  FileBuffer(const FileBuffer& other) = delete;
  FileBuffer& operator=(const FileBuffer&) = delete;
  ~FileBuffer();

  unsigned char* data = nullptr;
  int size = 0;
};
inline FileBuffer::FileBuffer(FileBuffer&& other) noexcept {
  std::swap(data, other.data);
  std::swap(size, other.size);

  other.data = nullptr;
  other.size = 0;
}

inline FileBuffer& FileBuffer::operator=(FileBuffer&& other) noexcept {
  std::swap(data, other.data);
  std::swap(size, other.size);

  other.data = nullptr;
  other.size = 0;

  return *this;
}

inline FileBuffer::~FileBuffer() {
  if (data != nullptr) {
    delete[] data;
    data = nullptr;
    size = 0;
  }
}
inline void LoadFileInBuffer(std::string_view path, FileBuffer* file_buffer) {
  std::ifstream t(path.data(), std::ios::binary);
  if (!t.is_open()) {
    file_buffer->data = nullptr;
    file_buffer->size = 0;
  }

  t.seekg(0, std::ios::end);
  file_buffer->size = static_cast<int>(t.tellg());
  t.seekg(0, std::ios::beg);

  file_buffer->data = new unsigned char[file_buffer->size];

  t.read(reinterpret_cast<char*>(file_buffer->data), file_buffer->size);
}