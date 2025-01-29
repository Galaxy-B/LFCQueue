#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>

namespace test {

using Path = std::filesystem::path;

/* make it easier to manage file stream */
class Dumper {
  private:
    std::ofstream file;

  public:
    explicit Dumper(const std::string&& path) : file(Path(path)) {
        if (!file) {
            std::cout << "fail to open or create dump file: " << path << std::endl;
            exit(1);
        }
    }

    ~Dumper() { file.close(); }

    std::ofstream& stream() { return file; }
};

}  // namespace test