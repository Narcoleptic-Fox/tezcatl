#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace tezcatl::metrics {

/// The complexity thresholds docs/metrics.md recommends.
inline constexpr unsigned default_flagged_over = 10;
inline constexpr unsigned default_high_over = 20;

/// Complexity thresholds. A value strictly greater than `flagged_over` is
/// flagged, and strictly greater than `high_over` is high.
struct Thresholds {
    unsigned flagged_over = default_flagged_over;
    unsigned high_over = default_high_over;
};

enum class Rating : std::uint8_t { ok, flagged, high };

/// Throws std::invalid_argument unless high_over >= flagged_over, since
/// otherwise a value could be high without being flagged.
void validate(const Thresholds& thresholds);

[[nodiscard]] Rating rate(unsigned value, const Thresholds& thresholds) noexcept;
[[nodiscard]] std::string_view to_string(Rating rating) noexcept;

/// Summary statistics of a set of per-function values.
struct Distribution {
    std::size_t count = 0;
    double mean = 0.0;
    /// The middle value, or the mean of the two middle values if the count
    /// is even.
    double median = 0.0;
    /// Nearest-rank 90th percentile: the smallest value that at least 90% of
    /// the values are less than or equal to.
    unsigned p90 = 0;
    unsigned max = 0;
    std::size_t flagged = 0; ///< values rated flagged or high
    std::size_t high = 0;    ///< values rated high
};

/// All fields are zero for an empty set.
[[nodiscard]] Distribution describe(std::vector<unsigned> values, const Thresholds& thresholds);

} // namespace tezcatl::metrics
