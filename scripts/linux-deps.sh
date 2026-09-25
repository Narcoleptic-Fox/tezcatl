#!/usr/bin/env bash
# Installs the Linux build and lint dependencies (Ubuntu 24.04), then verifies
# them. Used by hosted CI, by local containers, and to provision the
# self-hosted runner, so none of them can drift apart.
#
#   scripts/linux-deps.sh           install, then verify (root or sudo)
#   scripts/linux-deps.sh --check   verify only; self-hosted runners run this
#                                   on every job instead of installing
set -euo pipefail

llvm_version=22
clang_format_version=22.1.3

install() {
    local sudo_cmd=""
    if [[ $(id -u) -ne 0 ]]; then sudo_cmd="sudo"; fi

    $sudo_cmd apt-get update -q
    $sudo_cmd apt-get install -y -q --no-install-recommends \
        ca-certificates cmake cppcheck g++ git gnupg lsb-release ninja-build \
        pipx software-properties-common wget

    # Ubuntu's own LLVM is older than the one Tezcatl is developed against;
    # take the release from apt.llvm.org so CI and development run the same
    # clang-tidy.
    wget -q -O /tmp/llvm.sh https://apt.llvm.org/llvm.sh
    $sudo_cmd bash /tmp/llvm.sh "$llvm_version"
    $sudo_cmd apt-get install -y -q --no-install-recommends \
        "libclang-${llvm_version}-dev" "clang-tidy-${llvm_version}"

    # clang-format output can change between patch releases; pin it exactly.
    $sudo_cmd env PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin \
        pipx install --force "clang-format==${clang_format_version}"
}

# Receipts, not exit codes: every tool must answer with the expected version.
check() {
    "clang++-${llvm_version}" --version | head -1
    "clang-tidy-${llvm_version}" --version | grep -q "version ${llvm_version}\."
    clang-format --version | grep -q "version ${clang_format_version}"
    test -f "/usr/lib/llvm-${llvm_version}/include/clang-c/Index.h"
    cmake --version | head -1
    ninja --version
    g++ --version | head -1
    cppcheck --version
    echo "linux-deps: OK (LLVM ${llvm_version}, clang-format ${clang_format_version})"
}

case "${1:-}" in
    --check) check ;;
    "") install && check ;;
    *) echo "usage: scripts/linux-deps.sh [--check]" >&2 && exit 2 ;;
esac
