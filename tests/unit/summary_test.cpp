#include "metrics/summary.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

using namespace tezcatl::metrics;

TEST_CASE("ratings use strict thresholds", "[summary]") {
    const Thresholds defaults{};
    CHECK(rate(10, defaults) == Rating::ok);
    CHECK(rate(11, defaults) == Rating::flagged);
    CHECK(rate(20, defaults) == Rating::flagged);
    CHECK(rate(21, defaults) == Rating::high);
    CHECK(rate(3, Thresholds{.flagged_over = 2, .high_over = 2}) == Rating::high);
}

TEST_CASE("a high threshold below the flagged one is rejected", "[summary]") {
    CHECK_THROWS_AS(validate(Thresholds{.flagged_over = 10, .high_over = 9}),
                    std::invalid_argument);
    CHECK_NOTHROW(validate(Thresholds{.flagged_over = 10, .high_over = 10}));
}

TEST_CASE("the distribution of a set of values", "[summary]") {
    // Hand computed: 12 values summing to 91; the middle two (6th and 7th
    // in order) are 6 and 7; nearest rank for p90 is ceil(0.9 * 12) = 11,
    // the value 11; 11 and 25 are over 10, and 25 is over 20.
    const Distribution d = describe({25, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, Thresholds{});
    CHECK(d.count == 12);
    CHECK(d.mean * 12 == Catch::Approx(91));
    CHECK(d.median * 2 == 13);
    CHECK(d.p90 == 11);
    CHECK(d.max == 25);
    CHECK(d.flagged == 2);
    CHECK(d.high == 1);
}

TEST_CASE("p90 by nearest rank at the boundaries", "[summary]") {
    CHECK(describe({4}, Thresholds{}).p90 == 4);
    // Rank ceil(0.9 * 10) = 9 exactly, not 10.
    CHECK(describe({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, Thresholds{}).p90 == 9);
    // Rank ceil(0.9 * 11) = ceil(9.9) = 10.
    CHECK(describe({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, Thresholds{}).p90 == 10);
    // Rank ceil(0.9 * 6) = ceil(5.4) = 6. The counts above all have a
    // fractional part of .9 or none, where rounding to nearest gives the same
    // rank; this one tells ceiling from rounding and from truncation.
    CHECK(describe({1, 2, 3, 4, 5, 6}, Thresholds{}).p90 == 6);
    CHECK(describe({1, 2, 3}, Thresholds{}).median == 2);
}

TEST_CASE("an empty set has an all-zero distribution", "[summary]") {
    const Distribution d = describe({}, Thresholds{});
    CHECK(d.count == 0);
    CHECK(d.mean == 0.0);
    CHECK(d.max == 0);
}
