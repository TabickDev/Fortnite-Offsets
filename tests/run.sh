#!/bin/sh
set -eu

cd "$(dirname "$0")"
build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM

# Allow compiler/sanitizer overrides while keeping assertions enabled.
${CXX:-c++} ${CXXFLAGS:-} -std=c++17 -Wall -Wextra -Werror -pedantic -UNDEBUG \
    fname_test.cpp -o "$build_dir/fname_test"
"$build_dir/fname_test"
