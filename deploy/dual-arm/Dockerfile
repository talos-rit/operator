FROM debian:bookworm-slim
ARG http_proxy
ARG https_proxy
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 libstdc++6 libgcc-s1 libgomp1 libglib2.0-0 ffmpeg ca-certificates \
    && rm -rf /var/lib/apt/lists/*
# Snapshot the known-working ARM64 Python environment, with the same Debian/Python ABI.
COPY runtime/venv /opt/venv
COPY runtime/erv /opt/talos/erv
COPY tracker /opt/talos/tracker
COPY configs /opt/talos/configs
COPY runtime/yolo11n-pose-192.onnx /opt/talos/model.onnx
WORKDIR /opt/talos/tracker
ENV PATH=/opt/venv/bin:$PATH PYTHONUNBUFFERED=1
STOPSIGNAL SIGINT
