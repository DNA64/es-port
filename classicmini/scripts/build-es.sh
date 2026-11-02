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
set -e
sysroot_lib=/usr/lib/arm-linux-gnueabihf
sysroot_include=/usr/include/arm-linux-gnueabihf
build_dir=${build_dir:-/out/.build}
cross=arm-linux-gnueabihf

git config --global --add safe.directory '*' 2>/dev/null || true
git -C /src submodule update --init --depth 1

cat > /tmp/toolchain.cmake <<TC
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER $cross-gcc)
set(CMAKE_CXX_COMPILER $cross-g++)
set(CMAKE_FIND_ROOT_PATH /opt/freeimage /opt/gles-stubs /opt/curl-static /opt/rapidjson /opt/sdl2-headers /usr)
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
TC


mkdir -p "$build_dir" /out
cd "$build_dir"
cmake /src \
  -DCMAKE_TOOLCHAIN_FILE=/tmp/toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DGLES=ON \
  -DHINT_GLES_INCDIR=/opt/sdl2-headers \
  -DHINT_GLES_LIBDIR=/opt/gles-stubs/lib \
  -DFreeImage_INCLUDE_DIR=/opt/freeimage/include \
  -DFreeImage_LIBRARY_REL=/opt/freeimage/lib/libfreeimage.a \
  -DCURL_INCLUDE_DIR=/opt/curl-static/include \
  -DCURL_LIBRARY=/opt/curl-static/lib/libcurl.a \
  -DFREETYPE_LIBRARY=$sysroot_lib/libfreetype.so \
  -DFREETYPE_LIBRARY_RELEASE=$sysroot_lib/libfreetype.so \
  -DFREETYPE_INCLUDE_DIR_ft2build=$sysroot_include/freetype2 \
  -DFREETYPE_INCLUDE_DIR_freetype2=$sysroot_include/freetype2 \
  -DSDL2_LIBRARY=$sysroot_lib/libSDL2.so \
  -DSDL2_INCLUDE_DIR=/usr/include/SDL2 \
  -DALSA_LIBRARY=$sysroot_lib/libasound.so \
  -DALSA_INCLUDE_DIR=$sysroot_include \
  -DRAPIDJSON_INCLUDE_DIRS=/opt/rapidjson/include \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--allow-shlib-undefined" \
  -DCMAKE_CXX_STANDARD_LIBRARIES="-L/opt/mbedtls/lib -L$sysroot_lib -lmbedtls -lmbedx509 -lmbedcrypto -lz -lpthread" \
  >"$build_dir/cmake.log" 2>&1 || { tail -40 "$build_dir/cmake.log"; exit 1; }

cmake --build . -j"$(nproc)" >"$build_dir/make.log" 2>&1 || { grep -B3 -E "error|undefined reference" "$build_dir/make.log" | tail -40; exit 1; }

$cross-strip -o /out/emulationstation /src/emulationstation
cp $sysroot_lib/libfreetype.so.6 /out/libfreetype.so.6
ls -la /out
