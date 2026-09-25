#include "metrics/summary.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <string>

namespace tezcatl::metrics {

void validate(const Thresholds& thresholds) {
    if (thresholds.high_over < thresholds.flagged_over) {
        throw std::invalid_argument("the high threshold (" + std::to_string(thresholds.high_over) +
                                    ") is below the flagged threshold (" +
                                    std::to_string(thresholds.flagged_over) + ")");
    }
}

Rating rate(unsigned value, const Thresholds& thresholds) noexcept {
    if (value > thresholds.high_over) {
        return Rating::high;
    }
    if (value > thresholds.flagged_over) {
        return Rating::flagged;
    }
    return Rating::ok;
}

std::string_view to_string(Rating rating) noexcept {
    switch (rating) {
    case Rating::ok:
        return "ok";
    case Rating::flagged:
        return "flagged";
    case Rating::high:
        return "high";
    }
    return "unknown";
}

Distribution describe(std::vector<unsigned> values, const Thresholds& thresholds) {
    Distribution result;
    if (values.empty()) {
        return result;
    }
    std::ranges::sort(values);
    const std::size_t count = values.size();
    result.count = count;
    // Summed as double: a sum of unsigned values can overflow, and the mean
    // is a double anyway.
    result.mean = std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(count);
    const std::size_t middle = count / 2;
    result.median = (count % 2 == 1) ? static_cast<double>(values.at(middle))
                                     : std::midpoint(static_cast<double>(values.at(middle - 1)),
                                                     static_cast<double>(values.at(middle)));
    // Nearest rank: ceil(0.9 * count), 1-based, in integers so that a count
    // of 10 gives rank 9 exactly rather than 9.000000000000002.
    const std::size_t rank = ((count * 9) + 9) / 10;
    result.p90 = values.at(rank - 1);
    result.max = values.back();
    for (const unsigned value : values) {
        const Rating rating = rate(value, thresholds);
        if (rating != Rating::ok) {
            ++result.flagged;
        }
        if (rating == Rating::high) {
            ++result.high;
        }
    }
    return result;
}

} // namespace tezcatl::metrics
