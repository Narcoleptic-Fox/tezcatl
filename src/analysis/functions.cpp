#include "analysis/functions.hpp"

#include "metrics/complexity.hpp"
#include "parse/function_tokens.hpp"

#include <algorithm>
#include <iterator>

namespace tezcatl::analysis {

std::vector<FunctionInfo> measure_functions(const parse::ParsedUnit& parsed,
                                            const parse::FileFilter& include_file) {
    const std::vector<parse::FunctionDefinition> definitions =
        parse::find_definitions(parsed, include_file);
    std::vector<FunctionInfo> measured;
    measured.reserve(definitions.size());
    std::ranges::transform(
        definitions, std::back_inserter(measured), [&parsed](const parse::FunctionDefinition& d) {
            const parse::FunctionTokens tokens{parsed.unit.get(), d.cursor};
            return FunctionInfo{.file = d.file,
                                .line = d.line,
                                .column = d.column,
                                .kind = d.kind,
                                .name = d.name,
                                .complexity = metrics::cyclomatic_complexity(tokens),
                                .halstead = metrics::measure_halstead(tokens)};
        });
    return measured;
}

} // namespace tezcatl::analysis
