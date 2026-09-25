#pragma once

#include "report/data.hpp"
#include "report/naming.hpp"

#include <iosfwd>

namespace tezcatl::report {

/// Writes the report for people: a summary table per module, then one
/// section per metric with its per-module figures and the functions,
/// declarations and files worth a reviewer's attention first. Every list
/// that is cut short says so and names the CSV file with all of it.
void write_report_markdown(std::ostream& out, const ReportData& data, const FileNaming& naming);

} // namespace tezcatl::report
