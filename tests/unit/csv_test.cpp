#include "report/csv.hpp"

#include <catch2/catch_test_macros.hpp>

using tezcatl::report::csv_field;

TEST_CASE("CSV fields are quoted only when they must be", "[csv]") {
    CHECK(csv_field("plain") == "plain");
    CHECK(csv_field("").empty());
    CHECK(csv_field("area(size_t, size_t)") == "\"area(size_t, size_t)\"");
    CHECK(csv_field("say \"hi\"") == "\"say \"\"hi\"\"\"");
    CHECK(csv_field("two\nlines") == "\"two\nlines\"");
}
