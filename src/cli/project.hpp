#pragma once

#include "config/modules.hpp"
#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>

namespace tezcatl::cli {

/// What every command that parses a project needs to know.
struct ProjectOptions {
    std::filesystem::path build_directory;    ///< holds compile_commands.json; may be empty
                                              ///< for commands that parse nothing
    std::filesystem::path root;               ///< only files under here are the project's
    std::filesystem::path resource_directory; ///< clang's built-in headers
    std::filesystem::path module_map;         ///< empty: every file is unassigned
    bool allow_parse_errors = false;
};

/// Where a command writes: its table to `out` (stdout), and parse errors and
/// the closing status line to `err` (stderr). A struct with named members
/// rather than two stream parameters, which could be passed in either order:
/// a table written to stderr would still pass any test that reads both.
struct Streams {
    std::reference_wrapper<std::ostream> out;
    std::reference_wrapper<std::ostream> err;
};

/// How many translation units a scan parsed, and how many had errors.
struct ScanTotals {
    std::size_t units = 0;
    std::size_t units_with_errors = 0;
};

/// A project resolved from its options: absolute directories, the filter
/// for its own files, and its modules.
class Project {
public:
    explicit Project(const ProjectOptions& options);
    ~Project() = default;

    // The file filter refers to this object's own directories.
    Project(const Project&) = delete;
    Project& operator=(const Project&) = delete;
    Project(Project&&) = delete;
    Project& operator=(Project&&) = delete;

    /// True for files under the root and not under the build directory, if
    /// there is one (fetched dependencies, generated code).
    [[nodiscard]] const parse::FileFilter& in_project() const noexcept { return in_project_; }
    /// `file` relative to the root, with '/' separators.
    [[nodiscard]] std::string relative(const std::filesystem::path& file) const;
    [[nodiscard]] std::string module_of(const std::filesystem::path& file) const;
    [[nodiscard]] const config::ModuleMap& modules() const noexcept { return modules_; }

    /// Parses every compilation database entry and hands each unit that
    /// libclang could parse to `visit`. Parse errors are written to `err`.
    ScanTotals scan(const std::function<void(const parse::ParsedUnit&)>& visit,
                    std::ostream& err) const;

    /// Writes the closing status line, "tezcatl: parsed N translation units,
    /// M with errors; " then `detail`, and returns the process exit code: a
    /// unit that failed to parse yields incomplete results that would
    /// otherwise pass for complete ones, so it fails the run unless allowed.
    [[nodiscard]] int finish(const ScanTotals& totals, std::string_view detail,
                             std::ostream& err) const;

private:
    ProjectOptions options_;
    std::filesystem::path root_;
    std::filesystem::path build_;
    parse::FileFilter in_project_;
    config::ModuleMap modules_;
};

} // namespace tezcatl::cli
