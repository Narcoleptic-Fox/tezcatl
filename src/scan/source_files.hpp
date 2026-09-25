#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace tezcatl::scan {

/// True for the file extensions Tezcatl treats as C or C++ source:
/// .c .h .cc .cp .cpp .cxx .c++ .hh .hpp .hxx .h++ .inl .ipp .tpp .cppm
/// (any case).
[[nodiscard]] bool is_source_file(const std::filesystem::path& path);

/// Expands each input into the C/C++ source files it names: a file is taken
/// as given (whatever its extension, since the caller asked for it), a
/// directory is walked recursively. The result is sorted and free of
/// duplicates so that reports are reproducible. Throws std::runtime_error if
/// an input does not exist.
[[nodiscard]] std::vector<std::filesystem::path>
find_source_files(const std::vector<std::filesystem::path>& inputs);

/// The C/C++ source files under `directory`, sorted. A subdirectory for
/// which `descend` is false is not entered at all: a build tree under the
/// project root can hold more files than the project, and walking it only
/// to drop them costs time.
[[nodiscard]] std::vector<std::filesystem::path>
find_source_files_under(const std::filesystem::path& directory,
                        const std::function<bool(const std::filesystem::path&)>& descend);

/// The whole file as bytes. Throws std::runtime_error naming the path if it
/// cannot be read; a file that cannot be read is never silently skipped.
[[nodiscard]] std::string read_file(const std::filesystem::path& path);

} // namespace tezcatl::scan
