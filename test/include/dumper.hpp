#pragma once
#include <filesystem>
#include <fstream>

namespace test {

using Path = std::filesystem::path;

/* make it easier to manage file stream */
class Dumper {
  private:
    std::ofstream file;

  public:
    explicit Dumper(const std::string&& path) : file(Path(path)) {}

    ~Dumper() { file.close(); }

    std::ofstream& stream() { return file; }
};

}  // namespace test