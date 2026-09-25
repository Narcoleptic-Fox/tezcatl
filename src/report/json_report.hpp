#pragma once

#include "report/data.hpp"
#include "report/naming.hpp"

#include <iosfwd>

namespace tezcatl::report {

/// The schema version of the JSON report, as docs/report.schema.json states
/// it. It changes whenever a field is added, removed or changes meaning.
inline constexpr int report_schema_version = 1;

/// Writes the whole report as one JSON document that validates against
/// docs/report.schema.json. Nothing in it depends on when it was written,
/// so the same inputs give the same bytes. Numbers are not rounded.
void write_report_json(std::ostream& out, const ReportData& data, const FileNaming& naming);

} // namespace tezcatl::report
