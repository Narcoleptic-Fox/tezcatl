#include "coverage/readers.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace tezcatl::coverage {

namespace fs = std::filesystem;

void read_coverage_file(const fs::path& file, CoverageData& data) {
    std::ifstream stream{file, std::ios::binary};
    if (!stream) {
        throw std::runtime_error("cannot read coverage file " + file.string());
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    const std::string text = buffer.str();
    const std::size_t files_before = data.size();

    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first != std::string::npos && text.at(first) == '{') {
        // Each JSON format names itself: llvm-cov with a "type", gcov with
        // a "format_version" beside its "files".
        if (text.find("\"llvm.coverage.json.export\"") != std::string::npos) {
            read_llvm_json(text, file, data);
        } else if (text.find("\"format_version\"") != std::string::npos) {
            read_gcov_json(text, file, data);
        } else {
            throw std::runtime_error(file.generic_string() +
                                     ": JSON, but neither gcov nor llvm-cov coverage");
        }
    } else {
        std::istringstream lines{text};
        read_lcov(lines, file.parent_path(), file, data);
    }
    // A file of another kind altogether reads as an empty lcov tracefile;
    // no records at all is never a real coverage file.
    if (data.size() == files_before && text.find("SF:") == std::string::npos &&
        text.find("\"files\"") == std::string::npos) {
        throw std::runtime_error(file.generic_string() +
                                 ": no coverage records (not lcov, gcov or llvm-cov data)");
    }
}

} // namespace tezcatl::coverage
