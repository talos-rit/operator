#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$script_dir/build"

if [[ ! -d "$build_dir" ]]; then
	echo "Build directory not found at $build_dir" >&2
	exit 1
fi

cd -- "$build_dir"
make "$@"

