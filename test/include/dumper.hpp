#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <utility>

namespace test {

using Path = std::filesystem::path;

/* make it easier to manage file stream */
class Dumper {
  private:
    std::ofstream file_;

  public:
    explicit Dumper(const std::string&& path) : file_(Path(path)) {
        if (!file_) {
            std::cout << "fail to open or create dump file: " << path << std::endl;
            exit(1);
        }
    }

    ~Dumper() { file_.close(); }

    template <typename... Args>
    void dump(Args&&... args) {
        (file_ << ... << std::forward<Args>(args)) << std::endl;
    }
};

/* thread-safe shared dumper for multiple writers */
class SyncDumper {
  private:
    Dumper dumper_;
    std::mutex mtx_;

  public:
    explicit SyncDumper(const std::string&& path) : dumper_(std::move(path)) {}

    template <typename... Args>
    void dump(Args&&... args) {
        std::lock_guard<std::mutex> lock(mtx_);
        dumper_.dump(std::forward<Args>(args)...);
    }
};

}  // namespace test