#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace tezcatl::config {

/// The module of every file no rule matches. Such files are reported under
/// this name, never dropped.
inline constexpr std::string_view unassigned_module = "(unassigned)";

struct ModuleRule {
    std::string module;
    std::string pattern; ///< a scan::glob_match pattern, relative to the root
};

/// Assigns files to named modules. The file format is one rule per line,
///
///     MODULE = GLOB
///
/// e.g. `core = src/core/**`. The first rule whose glob matches a file's
/// path (relative to the project root, with '/' separators) names its
/// module, so specific rules go before general ones. A module may have
/// several rules. Blank lines and lines starting with '#' are ignored.
class ModuleMap {
public:
    /// No rules: every file is unassigned.
    ModuleMap() = default;

    /// Throws std::runtime_error naming `source` (the file the text came
    /// from) and the line of the first malformed rule.
    [[nodiscard]] static ModuleMap parse(std::string_view text,
                                         const std::filesystem::path& source);
    [[nodiscard]] static ModuleMap load(const std::filesystem::path& file);

    [[nodiscard]] std::string module_of(std::string_view relative_path) const;
    [[nodiscard]] const std::vector<ModuleRule>& rules() const noexcept { return rules_; }

private:
    std::vector<ModuleRule> rules_;
};

} // namespace tezcatl::config
