#!/bin/sh
set -eu
# Isolated runtime: no Docker bridge/firewall changes or boot enablement.
target=/opt/talos-docker/29.8.2
test ! -e "$target/dockerd"
sudo -n mkdir -p "$target" /etc/talos-containers
sudo -n tar -xzf /tmp/talos-docker-arm64.tgz --strip-components=1 -C "$target"
sudo -n install -m 0755 /tmp/talos-docker-compose-arm64 "$target/docker-compose"
sudo -n tee /etc/talos-containers/daemon.json >/dev/null <<'EOF'
{
  "data-root": "/var/lib/talos-docker",
  "exec-root": "/run/talos-docker",
  "pidfile": "/run/talos-docker.pid",
  "hosts": ["unix:///run/talos-docker.sock"],
  "iptables": false,
  "ip6tables": false,
  "ip-forward": false,
  "ip-masq": false,
  "bridge": "none",
  "features": {"containerd-snapshotter": false},
  "storage-driver": "overlay2",
  "log-driver": "json-file",
  "log-opts": {"max-size": "5m", "max-file": "2"}
}
EOF
sudo -n tee /etc/systemd/system/talos-docker.service >/dev/null <<'EOF'
[Unit]
Description=Isolated TALOS container runtime
After=network.target
[Service]
Environment=PATH=/opt/talos-docker/29.8.2:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
Environment=HTTP_PROXY=http://127.0.0.1:18081
Environment=HTTPS_PROXY=http://127.0.0.1:18081
Environment=NO_PROXY=localhost,127.0.0.1,::1
ExecStart=/opt/talos-docker/29.8.2/dockerd --config-file=/etc/talos-containers/daemon.json
Restart=no
LimitNOFILE=1048576
Delegate=yes
[Install]
WantedBy=multi-user.target
EOF
sudo -n systemctl daemon-reload
sudo -n systemctl start talos-docker.service
for attempt in 1 2 3 4 5 6 7 8 9 10; do
    test ! -S /run/talos-docker.sock || break
    sleep 1
done
sudo -n "$target/docker" -H unix:///run/talos-docker.sock info --format '{{.ServerVersion}} {{.Architecture}}'
