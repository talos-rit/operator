#!/bin/sh
set -eu
cd "$(dirname "$0")"
if [ "$#" = 1 ] && [ "$1" = stop ]; then
    set -- stop bluey-vision bingo-vision bluey-camera bingo-camera bluey-operator bingo-operator
fi
case " $* " in
    *" up "*|*" start "*|*" restart "*)
        case " $* " in *compose.software.yaml*) ;; *) sh check-host.sh ;; esac
        case " $* " in
            *"--profile bingo "*|*"--profile=bingo "*|*" bingo-operator "*)
                test "${TALOS_CONFIRM_NATIVE_HOME:-0}" = 1 || {
                    echo 'Native startup HOMEs Bingo. Use activate-bingo.sh --confirm-home.' >&2; exit 1;
                } ;;
        esac
        ;;
esac
if [ -S /run/talos-docker.sock ]; then
    export DOCKER_HOST=unix:///run/talos-docker.sock
    exec sudo -n env DOCKER_HOST="$DOCKER_HOST" /opt/talos-docker/29.8.2/docker-compose "$@"
fi
exec sudo docker compose "$@"
