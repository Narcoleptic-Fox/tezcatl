#!/usr/bin/env bash
# Tezcatl's report on itself, with its own test coverage imported: the
# self-review CI runs on every push. Run it after the coverage build:
#
#   cmake --preset linux-coverage && cmake --build --preset linux-coverage
#   ctest --preset linux-coverage
#   scripts/self-report.sh OUT_DIR
#
# Writes OUT_DIR/report (the report) and OUT_DIR/coverage.gcov.json.
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
build="$root/build/linux-coverage"
mkdir -p "$1"
out=$(cd "$1" && pwd)
coverage="$out/coverage.gcov.json"

# gcov on every object the build made, run or not: an object the tests never
# ran is 0% covered, not absent. Fetched dependencies are not Tezcatl's.
: > "$coverage"
while IFS= read -r dir; do
    (cd "$dir" && gcov -b --json-format --stdout ./*.gcno 2>/dev/null) >> "$coverage"
done < <(find "$build" -path "$build/_deps" -prune -o -name '*.gcno' -printf '%h\n' | sort -u)

# Receipts, not exit codes: records must exist, and the report must validate.
records=$(wc -l < "$coverage")
if ((records == 0)); then
    echo "self-report: gcov wrote no coverage records under $build" >&2
    exit 1
fi

# Other preset build trees may sit under the root on a shared runner.
"$build/src/tezcatl" report -p "$build" --root "$root" --exclude "build/**" \
    --modules "$root/docs/self-review/modules.txt" --coverage "$coverage" --out "$out/report"
"$build/tests/validate_json" "$root/docs/report.schema.json" "$out/report/report.json"
echo "self-report: $records coverage records; the report is in $out/report"
