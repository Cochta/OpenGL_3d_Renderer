#pragma once

#include <fstream>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

std::string LoadFile(std::string_view path);

enum class JobStatus : std::int8_t {
  kStarted,
  kDone,
  kNone,
};

enum class JobType : std::int8_t {
  kNone = -1,
  kFileReading,
  kFileDecompressing,
  kDownloadingDataToGpu,
  kOther
};

class Job {
 public:
  Job(JobType job_type) : type_(job_type){};

  Job(Job&& other) noexcept;
  Job& operator=(Job&& other) noexcept;
  Job(const Job& other) noexcept = delete;
  Job& operator=(const Job& other) noexcept = delete;

  virtual ~Job() noexcept = default;

  void Execute() noexcept;

  void WaitUntilJobIsDone() const noexcept;

  void AddDependency(Job* dependency) noexcept;

  const bool IsDone() const noexcept { return status_ == JobStatus::kDone; }
  const bool HasStarted() const noexcept {
    return status_ == JobStatus::kStarted;
  }

  const JobType type() const noexcept { return type_; }

 protected:
  std::vector<const Job*> dependencies_;
  mutable std::promise<void> promise_;
  mutable std::future<void> future_ = promise_.get_future();
  JobStatus status_ = JobStatus::kNone;
  JobType type_ = JobType::kNone;

  virtual void Work() noexcept = 0;
};

class Worker {
 public:
  Worker() noexcept = default;
  void Loop(std::vector<Job*>& jobs) noexcept;
  void Join() noexcept;

 private:
  std::thread thread_{};
  bool is_running_ = true;
};

class JobSystem {
 public:
  JobSystem() noexcept = default;
  void AddJob(Job* job) noexcept;
  void LaunchWorkers(int worker_count) noexcept;
  void JoinWorkers() noexcept;
  void RunMainThreadWorkLoop(std::vector<Job*>& jobs) noexcept;

 private:
  std::vector<Worker> workers_{};

  // Use vectors as queues by pushing elements back to the end while
  // erasing those at the front.
  std::vector<Job*> img_reading_jobs_{};
  std::vector<Job*> img_decompressing_jobs_{};
  std::vector<Job*> img_up_gpu_jobs_{};
};
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