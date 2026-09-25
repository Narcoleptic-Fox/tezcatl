#include "cli/project.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace fs = std::filesystem;
using tezcatl::cli::Project;
using tezcatl::cli::ProjectOptions;

namespace {

ProjectOptions options(const fs::path& root, const fs::path& build) {
    return {.build_directory = build,
            .root = root,
            .resource_directory = {},
            .module_map = {},
            .test_globs = {},
            .allow_parse_errors = false};
}

} // namespace

TEST_CASE("a build directory inside the root is not the project", "[project]") {
    const fs::path root = fs::absolute("/p");
    const Project project{options(root, root / "build")};
    CHECK(project.in_project()(root / "src/a.c"));
    CHECK_FALSE(project.in_project()(root / "build/_deps/lib/b.c"));
    CHECK_FALSE(project.in_project()(fs::absolute("/elsewhere/c.c")));
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
