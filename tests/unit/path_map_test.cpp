#include "coverage/path_map.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
using namespace tezcatl::coverage;

TEST_CASE("a path mapping is FROM=TO, split at the first '='", "[coverage]") {
    const PathMapping mapping = parse_path_mapping("/ci/work=/home/me/src");
    CHECK(mapping.from == fs::path{"/ci/work"});
    CHECK(mapping.to == fs::path{"/home/me/src"});
    CHECK(parse_path_mapping("/a=b=c").to == fs::path{"b=c"});
    CHECK_THROWS_AS(parse_path_mapping("/ci/work"), std::invalid_argument);
    CHECK_THROWS_AS(parse_path_mapping("=/x"), std::invalid_argument);
    CHECK_THROWS_AS(parse_path_mapping("/x="), std::invalid_argument);
}

TEST_CASE("recorded paths move by whole components, first mapping first", "[coverage]") {
    const std::vector<PathMapping> mappings{{.from = "/ci/work/src", .to = "/local/src"},
                                            {.from = "/ci/work", .to = "/local"}};
    CHECK(map_path("/ci/work/src/a.c", mappings) == fs::path{"/local/src/a.c"}.lexically_normal());
    CHECK(map_path("/ci/work/b.c", mappings) == fs::path{"/local/b.c"}.lexically_normal());
    // "/ci/workshop" is not within "/ci/work".
    CHECK(map_path("/ci/workshop/c.c", mappings) == fs::path{"/ci/workshop/c.c"});
    CHECK(map_path("/elsewhere/d.c", mappings) == fs::path{"/elsewhere/d.c"});
}
