#pragma once

#include <string>
#include <string_view>

namespace tezcatl::report {

/// A CSV field per RFC 4180: quoted when it contains a comma, a double quote
/// or a line break, with embedded quotes doubled. Function names such as
/// "area(size_t, size_t)" contain commas.
[[nodiscard]] std::string csv_field(std::string_view text);

} // namespace tezcatl::report
