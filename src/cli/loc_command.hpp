#pragma once

#include <filesystem>
#include <iosfwd>
#include <vector>

namespace tezcatl::cli {

/// `tezcatl loc PATH...`: writes one CSV row per source file and a final
/// TOTAL row, columns file,physical,blank,comment,code. Paths are written with
/// forward slashes so output is identical across platforms.
void run_loc(const std::vector<std::filesystem::path>& inputs, std::ostream& out);

/// `tezcatl loc --lines PATH...`: writes one CSV row per physical line,
/// columns file,line,kind, so any count can be audited line by line.
void run_loc_lines(const std::vector<std::filesystem::path>& inputs, std::ostream& out);

} // namespace tezcatl::cli
