#pragma once

#include "coverage/model.hpp"

#include <filesystem>
#include <istream>
#include <string_view>

namespace tezcatl::coverage {

/// Reads an lcov tracefile (geninfo format, lcov 1.x and 2.x) into `data`.
/// Relative SF paths are taken relative to `base`. Totals records (LF, LH,
/// BRF, ...) are ignored: totals are recomputed from the per-line records.
/// Throws std::runtime_error naming `source` and the line of a malformed
/// record.
void read_lcov(std::istream& in, const std::filesystem::path& base,
               const std::filesystem::path& source, CoverageData& data);

/// Reads gcov's JSON intermediate format (gcc 9 and later, `gcov
/// --json-format`, one document or one per line as `--stdout` writes them),
/// uncompressed. Paths are resolved against each document's working
/// directory. Branches are present only if gcov ran with -b.
void read_gcov_json(std::string_view text, const std::filesystem::path& source, CoverageData& data);

/// Reads an `llvm-cov export -format=text` document. llvm-cov computes each
/// file's totals itself, from its own definitions of lines, branches and
/// functions, and they are taken as they are. A file reported by two
/// exports is an error: merge the profiles with llvm-profdata first.
void read_llvm_json(std::string_view text, const std::filesystem::path& source, CoverageData& data);

/// Reads one coverage file of any of the three formats, recognised by its
/// content. Throws std::runtime_error for an unreadable file, an
/// unrecognised format, a file with no coverage records, or data that would
/// mix llvm-cov totals with line data for the same source file.
void read_coverage_file(const std::filesystem::path& file, CoverageData& data);

} // namespace tezcatl::coverage
