#pragma once

#include "config/modules.hpp"
#include "parse/compilation_database.hpp"
#include "parse/functions.hpp"
#include "parse/translation_unit.hpp"
#include "report/naming.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::cli {

/// What every command that parses a project needs to know.
struct ProjectOptions {
    std::filesystem::path build_directory;    ///< holds compile_commands.json; may be empty
                                              ///< for commands that parse nothing
    std::filesystem::path root;               ///< only files under here are the project's
    std::filesystem::path resource_directory; ///< clang's built-in headers
    std::filesystem::path module_map;         ///< empty: every file is unassigned
    std::vector<std::string> test_globs;      ///< empty: the default test globs
    bool allow_parse_errors = false;
    unsigned jobs = 0; ///< parsing threads; 0: one per hardware thread
};

/// Where a command writes: its table to `out` (stdout), and parse errors and
/// the closing status line to `err` (stderr). A struct with named members
/// rather than two stream parameters, which could be passed in either order:
/// a table written to stderr would still pass any test that reads both.
struct Streams {
    std::reference_wrapper<std::ostream> out;
    std::reference_wrapper<std::ostream> err;
};

/// The compilation database entries a scan parses, in database order, and
/// how many it leaves out and why.
struct ScanPlan {
    std::vector<parse::CompileCommand> units;
    std::size_t skipped = 0;            ///< entries for other languages
    std::size_t in_build_directory = 0; ///< dependency and generated units
};

/// How many translation units a scan parsed, and how many had errors.
struct ScanTotals {
    std::size_t units = 0;
    std::size_t units_with_errors = 0;
    /// Database entries for other languages (Fortran, assembly), which a
    /// database made by intercepting a build records alongside C and C++.
    std::size_t skipped = 0;
    /// Units whose main file is under the build directory inside the root:
    /// fetched dependencies and generated code. Nothing in them is the
    /// project's, so they are not parsed at all.
    std::size_t in_build_directory = 0;
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
    /// that is strictly inside the root (fetched dependencies, generated
    /// code). A build directory that is the root, or above it, is an
    /// in-source build and excludes nothing.
    [[nodiscard]] const parse::FileFilter& in_project() const noexcept { return in_project_; }
    /// How files are named, and assigned to modules and roles.
    [[nodiscard]] const report::FileNaming& naming() const noexcept { return naming_; }

    /// The units to parse: every C and C++ entry of the compilation
    /// database, except units under a build directory inside the root.
    [[nodiscard]] ScanPlan plan() const;

    /// Parses the plan's units on several threads, each with its own
    /// libclang index, and hands each unit libclang could parse to `visit`
    /// with its position in `scan_plan.units`. `visit` runs on several threads at
    /// once, never twice for one position: it must write only to state kept
    /// for that position. Parse errors are written to `err` after all units
    /// are parsed, in database order, so a run's output does not depend on
    /// which thread finished first. An exception from any unit is rethrown
    /// here once every thread has stopped.
    ScanTotals scan(const ScanPlan& scan_plan,
                    const std::function<void(std::size_t, const parse::ParsedUnit&)>& visit,
                    std::ostream& err) const;

    /// Writes the closing status line, "tezcatl: parsed N translation units,
    /// M with errors; " (then "skipped K entries that are not C or C++; " and
    /// "skipped B units under the build directory; " if any were) then
    /// `detail`, and returns the process exit code: a
    /// unit that failed to parse yields incomplete results that would
    /// otherwise pass for complete ones, so it fails the run unless allowed.
    [[nodiscard]] int finish(const ScanTotals& totals, std::string_view detail,
                             std::ostream& err) const;

private:
    ProjectOptions options_;
    std::filesystem::path root_;
    std::filesystem::path build_;
    std::filesystem::path excluded_; ///< the build directory, if strictly inside the root
    parse::FileFilter in_project_;
    report::FileNaming naming_;
};

} // namespace tezcatl::cli
