#!/usr/bin/env bash
architecture=$(dpkg --print-architecture)
PACKAGES_TO_INSTALL="bear clang-format cpputest"
if [ "$architecture" == "amd64" ]; then
    wget https://github.com/bkryza/clang-uml/releases/download/0.6.2/clang-uml_0.6.2-1_amd64.deb
    PACKAGES_TO_INSTALL+=" ./clang-uml_0.6.2-1_amd64.deb"
fi
apt-get update && export DEBIAN_FRONTEND=noninteractive
apt-get -y install --no-install-recommends ${PACKAGES_TO_INSTALL}