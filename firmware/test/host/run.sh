#!/usr/bin/env bash
set -euo pipefail
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
test_binary="$(mktemp)"
trap 'rm -f "$test_binary"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I "$script_dir/../../include" \
    "$script_dir/scanner_test.cpp" -o "$test_binary"
"$test_binary"
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I "$script_dir/../../include" \
    "$script_dir/diagnostics_test.cpp" -o "$test_binary"
"$test_binary"
python3 "$script_dir/log_validator_test.py"
"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -I "$script_dir/../../include" \
    "$script_dir/logging_test.cpp" -o "$test_binary"
"$test_binary"
