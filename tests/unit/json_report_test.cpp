#include "metrics/summary.hpp"
#include "parse/api.hpp"
#include "parse/functions.hpp"
#include "report/json_report.hpp"
#include "report_sample.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

using nlohmann::json;

namespace {

json schema() {
    std::ifstream in{std::filesystem::path{TEZCATL_SOURCE_DIR} / "docs" / "report.schema.json"};
    REQUIRE(in);
    return json::parse(in);
}

json sample_json() {
    std::ostringstream out;
    tezcatl::report::write_report_json(out, tezcatl::test::sample_report(),
                                       tezcatl::test::sample_naming());
    return json::parse(out.str());
}

bool valid(const json& document) {
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(schema());
    try {
        validator.validate(document);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

TEST_CASE("the JSON report validates against docs/report.schema.json", "[report]") {
    CHECK(valid(sample_json()));
}

TEST_CASE("the schema rejects what the report must not contain", "[report]") {
    // A schema that accepts everything would make the test above pass for
    // nothing; these must all fail validation.
    json extra = sample_json();
    extra.emplace("unexpected", 1);
    CHECK_FALSE(valid(extra));

    json missing = sample_json();
    missing.erase("functions");
    CHECK_FALSE(valid(missing));

    json newer = sample_json();
    newer.at("schema_version") = tezcatl::report::report_schema_version + 1;
    CHECK_FALSE(valid(newer));

    json bad_rating = sample_json();
    bad_rating.at("functions").at(0).at("rating") = "fine";
    CHECK_FALSE(valid(bad_rating));
}

TEST_CASE("the JSON report carries the sample's figures", "[report]") {
    const json report = sample_json();
    CHECK(report.at("schema_version") == tezcatl::report::report_schema_version);
    CHECK(report.at("input").at("translation_units") == 3);
    // Worked out in unit/module_summary_test.cpp.
    CHECK(report.at("totals").at("complexity").at("functions") == 3);
    CHECK(report.at("totals").at("complexity").at("p90") == 12);
    CHECK(report.at("totals").at("lines").at("test").at("code") == 4);
    CHECK(report.at("totals").at("includes").is_null());
    REQUIRE(report.at("modules").size() == 2);
    CHECK(report.at("modules").at(1).at("module") == "a");
    CHECK(report.at("modules").at(1).at("includes").at("fan_in") == 1);
    CHECK(report.at("files").at(1).at("role") == "test");
    CHECK(report.at("functions").size() == 4);
    CHECK(report.at("includes").at("edges").at(0).at("from") == "b/y.c");
    CHECK(report.at("settings").at("modules").at(0).at("glob") == "a/**");
}

TEST_CASE("the JSON report is the same bytes every time", "[report]") {
    std::ostringstream first;
    std::ostringstream second;
    tezcatl::report::write_report_json(first, tezcatl::test::sample_report(),
                                       tezcatl::test::sample_naming());
    tezcatl::report::write_report_json(second, tezcatl::test::sample_report(),
                                       tezcatl::test::sample_naming());
    CHECK(first.str() == second.str());
}

namespace {

// Every spelling to_string gives an enum, found by walking its values from 0
// until to_string answers "unknown". A new enumerator is picked up without
// anyone listing it, which is the point: the schema must be told of it.
template <typename Enum> std::set<std::string> spellings() {
    std::set<std::string> result;
    for (unsigned value = 0; value <= std::numeric_limits<std::uint8_t>::max(); ++value) {
        const std::string_view spelling = to_string(static_cast<Enum>(value));
        if (spelling == "unknown") {
            break;
        }
        result.emplace(spelling);
    }
    return result;
}

std::set<std::string> schema_enum(const json& schema, const char* array, const char* field) {
    const json& values =
        schema.at("properties").at(array).at("items").at("properties").at(field).at("enum");
    return {values.begin(), values.end()};
}

} // namespace

TEST_CASE("the schema's enums are exactly what the code writes", "[report]") {
    // Two lists of one fact: a kind added to the code and not to the schema
    // would make every report that uses it fail validation.
    using namespace tezcatl;
    const json document = schema();
    CHECK(schema_enum(document, "functions", "kind") == spellings<parse::FunctionKind>());
    CHECK(schema_enum(document, "functions", "rating") == spellings<metrics::Rating>());
    CHECK(schema_enum(document, "api", "kind") == spellings<parse::ApiKind>());
    CHECK(schema_enum(document, "api", "documentation") == spellings<parse::DocStyle>());
}
