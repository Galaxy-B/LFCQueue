#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>

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

}  // namespace test