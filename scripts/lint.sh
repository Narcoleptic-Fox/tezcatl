#!/usr/bin/env bash
# Static gates: clang-format, clang-tidy, cppcheck. Every gate is fatal.
# Usage: scripts/lint.sh <build dir containing compile_commands.json>
set -euo pipefail

build_dir=$(cd "${1:?usage: scripts/lint.sh <build-dir>}" && pwd)
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"

clang_format=${CLANG_FORMAT:-clang-format}
clang_tidy=${CLANG_TIDY:-clang-tidy}

# tests/fixtures holds inputs for Tezcatl to measure, written to exercise edge
# cases; it is not Tezcatl's own code and is not in the compilation database.
mapfile -t sources < <(find src tests -path tests/fixtures -prune -o \
    -type f \( -name '*.cpp' -o -name '*.hpp' \) -print | sort)
mapfile -t translation_units < <(printf '%s\n' "${sources[@]}" | grep '\.cpp$')

# A gate over zero files passes by checking nothing.
if ((${#translation_units[@]} == 0)); then
    echo "lint: no sources found under src/ or tests/" >&2
    exit 1
fi
echo "lint: ${#sources[@]} files, ${#translation_units[@]} translation units"

echo "== clang-format"
"$clang_format" --dry-run --Werror "${sources[@]}"

echo "== clang-tidy"
"$clang_tidy" -p "$build_dir" --quiet "${translation_units[@]}"

echo "== cppcheck"
cppcheck_log=$(mktemp)
cppcheck --project="$build_dir/compile_commands.json" \
    --file-filter="$root/src/*" --file-filter="$root/tests/*" \
    --enable=warning,style,performance,portability \
    --inline-suppr --suppress=missingIncludeSystem \
    --error-exitcode=1 2>&1 | tee "$cppcheck_log"
checked=$(grep -c '^Checking .*\.cpp \.\.\.$' "$cppcheck_log" || true)
if ((checked != ${#translation_units[@]})); then
    echo "lint: cppcheck checked $checked translation units, expected ${#translation_units[@]}" >&2
    exit 1
fi

echo "lint: all gates passed"
