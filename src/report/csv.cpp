#include "report/csv.hpp"

namespace tezcatl::report {

std::string csv_field(std::string_view text) {
    if (text.find_first_of(",\"\r\n") == std::string_view::npos) {
        return std::string{text};
    }
    std::string quoted = "\"";
    for (const char c : text) {
        if (c == '"') {
            quoted += '"';
        }
        quoted += c;
    }
    quoted += '"';
    return quoted;
}

} // namespace tezcatl::report
