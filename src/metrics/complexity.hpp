#pragma once

#include "parse/function_tokens.hpp"

namespace tezcatl::metrics {

/// McCabe cyclomatic complexity of a function definition: 1 plus one for
/// each decision point among its own tokens, so the decision points of
/// functions nested in it (lambdas, member functions of local classes) count
/// towards those functions instead.
///
/// A decision point is an `if`, `for` (either form), `while`, `do`, `case`,
/// `catch`, `&&`, `||` or `?:` that is both written as a token in the
/// definition and confirmed by the AST: `else if` counts once, the `while`
/// of a do-while is not a second decision, `&&` in `int&& r` is not an
/// operator, an overloaded `operator&&` is a call and does not short-circuit,
/// and code disabled by the preprocessor has no AST. Decisions inside a
/// macro's body are not written in the function and are not counted;
/// decisions written in a macro's arguments are. docs/metrics.md has the
/// full definition.
///
/// The unit must have been parsed with the detailed preprocessing record,
/// or decisions in macro arguments are missed.
[[nodiscard]] unsigned cyclomatic_complexity(const parse::FunctionTokens& function);

} // namespace tezcatl::metrics
