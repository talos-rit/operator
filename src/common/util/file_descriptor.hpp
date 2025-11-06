#include <unistd.h>
class FileDescriptor {
 public:
  FileDescriptor() = default;
  explicit FileDescriptor(int fd) : fd_(fd) {}
  ~FileDescriptor() { reset(); }

  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;

  FileDescriptor(FileDescriptor &&other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
  }

  FileDescriptor &operator=(FileDescriptor &&other) noexcept {
    if (this != &other) {
      reset();
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  int get() noexcept { return fd_; }
  bool valid() const noexcept { return fd_ != -1; }
  void reset() noexcept {
    if (fd_ != -1) {
      ::close(fd_);
      fd_ = -1;
    }
  }
  operator int() const noexcept { return fd_; }

 private:
  int fd_{-1};
};
