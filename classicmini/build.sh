#!/bin/sh
#  Copyright (C) 2026 DefKorns (https://defkorns.github.io/LICENSE)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Cross-builds EmulationStation for the NES/SNES Classic inside the toolchain image; the repository is mounted, nothing
# but docker is needed on the host.
#
#   classicmini/build.sh
#   ES_TOOLCHAIN_IMAGE=classicmini-es-toolchain:local classicmini/build.sh
#
# The image is built from Dockerfile.es-armhf in DefKorns/classicmini-cross-toolchain and pinned here to a release;
# ES_TOOLCHAIN_IMAGE selects another tag (es-armhf is master's latest) or a local build.
# Result: classicmini/out/emulationstation (stripped ARM binary) and libfreetype.so.6
set -e
export MSYS_NO_PATHCONV=1

REPO=$(cd "$(dirname "$0")/.." && { pwd -W 2>/dev/null || pwd; })
image=${ES_TOOLCHAIN_IMAGE:-ghcr.io/defkorns/classicmini-es-toolchain:v0.1.0}

docker image inspect "$image" >/dev/null 2>&1 || docker pull -q "$image" >&2

# a root container would leave root-owned files behind on Linux/WSL
user_opts=
[ "$(uname -s)" = Linux ] && user_opts="--user $(id -u):$(id -g) -e HOME=/tmp"

mkdir -p "$REPO/classicmini/out"
# shellcheck disable=SC2086
docker run --rm $user_opts -v "$REPO:/src" -v "$REPO/classicmini/out:/out" "$image" sh /src/classicmini/scripts/build-es.sh
