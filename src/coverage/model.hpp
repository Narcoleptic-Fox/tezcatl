#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <utility>

namespace tezcatl::coverage {

/// Coverage totals: how many lines, branches and functions have data, and
/// how many of each were executed at least once.
struct Counts {
    std::size_t lines = 0;
    std::size_t lines_covered = 0;
    std::size_t branches = 0;
    std::size_t branches_covered = 0;
    std::size_t functions = 0;
    std::size_t functions_covered = 0;

    Counts& operator+=(const Counts& other) noexcept;
    friend bool operator==(const Counts&, const Counts&) = default;
};

/// The coverage of one source file, merged across every input that reports
/// it: a line, branch or function reported several times (by several
/// translation units, or several template instantiations) counts once, with
/// the sum of its hit counts, as gcovr merges them.
class FileRecord {
public:
    void add_line(unsigned line, std::uint64_t hits);
    /// `branch` identifies the branch within its line, as the format numbers it.
    void add_branch(unsigned line, const std::string& branch, std::uint64_t hits);
    void add_function(const std::string& name, std::uint64_t hits);
    /// Totals already computed by the tool that wrote the data (llvm-cov),
    /// added as they are.
    void add_totals(const Counts& totals) noexcept;

    [[nodiscard]] Counts counts() const;

private:
    std::map<unsigned, std::uint64_t> lines_;
    std::map<std::pair<unsigned, std::string>, std::uint64_t> branches_;
    std::map<std::string, std::uint64_t> functions_;
    Counts totals_;
};

/// Coverage by source file, keyed by the file's absolute path as recorded
/// in the data.
using CoverageData = std::map<std::filesystem::path, FileRecord>;

} // namespace tezcatl::coverage
