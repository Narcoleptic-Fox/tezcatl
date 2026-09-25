#include "cli/loc_command.hpp"

#include "metrics/loc.hpp"
#include "report/csv.hpp"
#include "scan/source_files.hpp"

#include <ostream>
#include <string>
#include <string_view>

namespace tezcatl::cli {

namespace {

void write_row(std::ostream& out, std::string_view name, const metrics::LocCounts& counts) {
    out << report::csv_field(name) << ',' << counts.physical << ',' << counts.blank << ','
        << counts.comment << ',' << counts.code << '\n';
}

std::string_view kind_name(metrics::LineKind kind) noexcept {
    switch (kind) {
    case metrics::LineKind::blank:
        return "blank";
    case metrics::LineKind::comment:
        return "comment";
    case metrics::LineKind::code:
        return "code";
    }
    return "unknown";
}

} // namespace

void run_loc(const std::vector<std::filesystem::path>& inputs, std::ostream& out) {
    out << "file,physical,blank,comment,code\n";
    metrics::LocCounts total;
    for (const auto& file : scan::find_source_files(inputs)) {
        const metrics::LocCounts counts = metrics::count_lines(scan::read_file(file));
        write_row(out, file.generic_string(), counts);
        total += counts;
    }
    write_row(out, "TOTAL", total);
}

void run_loc_lines(const std::vector<std::filesystem::path>& inputs, std::ostream& out) {
    out << "file,line,kind\n";
    for (const auto& file : scan::find_source_files(inputs)) {
        const std::string name = file.generic_string();
        std::size_t number = 0;
        for (const metrics::LineKind kind : metrics::classify_lines(scan::read_file(file))) {
            out << report::csv_field(name) << ',' << ++number << ',' << kind_name(kind) << '\n';
        }
    }
}

} // namespace tezcatl::cli
