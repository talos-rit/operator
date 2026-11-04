#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
operator=$(CDPATH= cd -- "$here/../.." && pwd)
python3 -c "import sys; assert sys.version_info[:2] == (3, 11), 'Use Debian Bookworm Python 3.11 for the runtime ABI'"
test "$(uname -m)" = aarch64 || { echo 'Build this image on a 64-bit ARM Pi.' >&2; exit 1; }
for command in git cmake python3 pkg-config; do command -v "$command" >/dev/null; done
pkg-config --exists cpputest || { echo 'Install libcpputest-dev first (see README).' >&2; exit 1; }
commander=${1:-"$here/.cache/commander"}
ref=$(cat "$here/commander.ref")
if [ ! -d "$commander/.git" ]; then
    mkdir -p "$(dirname "$commander")"
    git clone --branch feat/dual-arm-containers --depth 1 https://github.com/talos-rit/commander.git "$commander"
fi
test "$(git -C "$commander" rev-parse HEAD)" = "$ref" || {
    echo "Commander must be at $ref. Update that checkout explicitly, or use a fresh managed cache." >&2; exit 1;
}
cmake -S "$operator" -B "$here/.cache/build" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$here/.cache/build" --parallel 2
ctest --test-dir "$here/.cache/build" --output-on-failure
if [ ! -x "$here/.cache/venv/bin/python" ]; then python3 -m venv "$here/.cache/venv"; fi
"$here/.cache/venv/bin/pip" install --only-binary=:all: -r "$here/requirements.lock"
context="$here/context"
mkdir -p "$context/runtime" "$context/tracker" "$context/configs"
cp "$here/Dockerfile" "$context/Dockerfile"
cp "$here/.cache/build/bin/erv" "$context/runtime/erv"
cp -a "$here/.cache/venv/." "$context/runtime/venv/"
cp "$commander"/tools/pi_tracker/*.py "$context/tracker/"
cp "$commander/assets/yolo/yolo11n-pose-192.onnx" "$context/runtime/"
cp "$here"/configs/*.conf "$context/configs/"
if [ -S /run/talos-docker.sock ]; then
    sudo -n env DOCKER_BUILDKIT=0 /opt/talos-docker/29.8.2/docker -H unix:///run/talos-docker.sock \
        build --network host --build-arg http_proxy="${http_proxy:-}" --build-arg https_proxy="${https_proxy:-}" \
        -t talos-arms:demo-v1 "$context"
else
    sudo docker build --network host -t talos-arms:demo-v1 "$context"
fi
echo 'Image built and C++ tests passed. No robot services were started.'
