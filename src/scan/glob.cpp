#include "scan/glob.hpp"

#include <cstddef>

namespace tezcatl::scan {

namespace {

// Whether `rest` matches `path` after skipping some run of its leading
// characters, as a wildcard would. A `*` run stops at the first '/'; a `**`
// run crosses them.
bool matches_after_any_run(std::string_view rest, std::string_view path, bool crosses_slashes) {
    for (std::size_t skip = 0; skip <= path.size(); ++skip) {
        if (glob_match(rest, path.substr(skip))) {
            return true;
        }
        if (!crosses_slashes && skip < path.size() && path.at(skip) == '/') {
            return false;
        }
    }
    return false;
}

bool matches_one(char pattern, char path) noexcept {
    return pattern == '?' ? path != '/' : pattern == path;
}

} // namespace

// Recursion depth is bounded by the number of wildcards in the pattern, not
// by the length of the path.
bool glob_match(std::string_view pattern, std::string_view path) {
    while (!pattern.empty()) {
        if (pattern.starts_with("**")) {
            const std::string_view rest = pattern.substr(2);
            // "**/" may also match no directories at all.
            return (rest.starts_with('/') && glob_match(rest.substr(1), path)) ||
                   matches_after_any_run(rest, path, /*crosses_slashes=*/true);
        }
        if (pattern.front() == '*') {
            return matches_after_any_run(pattern.substr(1), path, /*crosses_slashes=*/false);
        }
        if (path.empty() || !matches_one(pattern.front(), path.front())) {
            return false;
        }
        pattern.remove_prefix(1);
        path.remove_prefix(1);
    }
    return path.empty();
}

} // namespace tezcatl::scan
