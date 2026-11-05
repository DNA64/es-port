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
export MSYS_NO_PATHCONV=1

repo=$(cd "$(dirname "$0")/../.." && { pwd -W 2>/dev/null || pwd; })
image=${ES_TOOLCHAIN_IMAGE:-ghcr.io/defkorns/classicmini-es-toolchain:v0.1.0}

docker run --rm -v "$repo:/src:ro" "$image" sh -c '
  set -e
  g++ -std=c++14 -Wall -Wextra -Werror -I/src/es-app/src \
    /src/classicmini/tests/suspend_points_test.cpp /src/es-app/src/SuspendPoints.cpp -o /tmp/suspend_points_test
  /tmp/suspend_points_test
  g++ -std=c++14 -Wall -Wextra -Werror -I/src/es-app/src -I/src/es-core/src -I/src/external/pugixml/src -c     /src/classicmini/tests/metadata_overlay_test.cpp -o /tmp/metadata_overlay_test.o
  g++ -std=c++14 -w -I/src/es-app/src -I/src/es-core/src -I/src/external/pugixml/src     /tmp/metadata_overlay_test.o /src/es-app/src/MetaData.cpp /src/external/pugixml/src/pugixml.cpp     /src/es-core/src/utils/FileSystemUtil.cpp /src/es-core/src/utils/TimeUtil.cpp /src/es-core/src/utils/StringUtil.cpp     /src/es-core/src/Log.cpp -o /tmp/metadata_overlay_test
  /tmp/metadata_overlay_test'
