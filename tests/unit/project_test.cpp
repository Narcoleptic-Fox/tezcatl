#include "cli/project.hpp"
#include "parse/translation_unit.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
using tezcatl::cli::Project;
using tezcatl::cli::ProjectOptions;

namespace {

ProjectOptions options(const fs::path& root, const fs::path& build, unsigned jobs = 1) {
    return {.build_directory = build,
            .root = root,
            .resource_directory = tezcatl::parse::default_resource_directory(),
            .module_map = {},
            .test_globs = {},
            .exclude_globs = {},
            .allow_parse_errors = false,
            .jobs = jobs};
}

// The coverage fixture's database: three units.
Project coverage_project(unsigned jobs) {
    return Project{options(fs::path{TEZCATL_FIXTURES_DIR} / "coverage",
                           fs::path{TEZCATL_FIXTURE_DBS} / "coverage", jobs)};
}

} // namespace

TEST_CASE("a build directory inside the root is not the project", "[project]") {
    const fs::path root = fs::absolute("/p");
    const Project project{options(root, root / "build")};
    CHECK(project.in_project()(root / "src/a.c"));
    CHECK_FALSE(project.in_project()(root / "build/_deps/lib/b.c"));
    CHECK_FALSE(project.in_project()(fs::absolute("/elsewhere/c.c")));
}

TEST_CASE("files matching an exclude glob are not the project", "[project]") {
    const fs::path root = fs::absolute("/p");
    ProjectOptions with_excludes = options(root, root / "build/release");
    with_excludes.exclude_globs = {"build/**", "third_party/**"};
    const Project project{with_excludes};
    CHECK(project.in_project()(root / "src/a.c"));
    // Another build tree next to the one given, and vendored code.
    CHECK_FALSE(project.in_project()(root / "build/debug/_deps/b.c"));
    CHECK_FALSE(project.in_project()(root / "third_party/zlib/inflate.c"));
    // A glob matches whole root-relative paths: src/build/ is still the project.
    CHECK(project.in_project()(root / "src/build/c.c"));
}

TEST_CASE("an in-source build excludes nothing", "[project]") {
    // Make with bear writes compile_commands.json at the top of the tree:
    // the build directory is the root, or above it.
    const fs::path root = fs::absolute("/p");
    const Project at_root{options(root, root)};
    CHECK(at_root.in_project()(root / "src/a.c"));
    const Project above_root{options(root / "src", root)};
    CHECK(above_root.in_project()(root / "src/a.c"));
}

TEST_CASE("every unit is visited once, whatever the number of threads", "[project]") {
    constexpr unsigned threads = 4;
    const Project project = coverage_project(threads);
    const tezcatl::cli::ScanPlan plan = project.plan();
    REQUIRE(plan.units.size() == 3);
    std::vector<std::atomic<int>> visits(plan.units.size());
    std::ostringstream err;
    const tezcatl::cli::ScanTotals totals = project.scan(
        plan, [&](std::size_t unit, const tezcatl::parse::ParsedUnit&) { ++visits.at(unit); }, err);
    CHECK(totals.units == 3);
    CHECK(totals.units_with_errors == 0);
    for (const std::atomic<int>& count : visits) {
        CHECK(count.load() == 1);
    }
}

TEST_CASE("an exception on a parsing thread reaches the caller", "[project]") {
    // Swallowed, it would leave a unit's results out of an otherwise
    // complete-looking report.
    constexpr unsigned threads = 3;
    const Project project = coverage_project(threads);
    const tezcatl::cli::ScanPlan plan = project.plan();
    std::ostringstream err;
    CHECK_THROWS_WITH(project.scan(
                          plan,
                          [](std::size_t unit, const tezcatl::parse::ParsedUnit&) {
                              if (unit == 1) {
                                  throw std::runtime_error("unit 1 failed");
                              }
                          },
                          err),
                      "unit 1 failed");
}
