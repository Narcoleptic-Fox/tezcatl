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
void read_lcov(std::istream& in, const std::filesystem::path& base, std::string_view source,
               CoverageData& data);

} // namespace tezcatl::coverage
