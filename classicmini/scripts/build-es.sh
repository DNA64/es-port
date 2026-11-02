#!/bin/sh
# Runs inside the classicmini-es-toolchain image: /src is the repository, /out receives the result.
set -e
A=/usr/lib/arm-linux-gnueabihf
I=/usr/include/arm-linux-gnueabihf
BUILD=${BUILD:-/out/.build}
CROSS=arm-linux-gnueabihf

# external/pugixml is a submodule
git config --global --add safe.directory '*' 2>/dev/null || true
git -C /src submodule update --init --depth 1

cat > /tmp/toolchain.cmake <<TC
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER $CROSS-gcc)
set(CMAKE_CXX_COMPILER $CROSS-g++)
set(CMAKE_FIND_ROOT_PATH /opt/freeimage /opt/gles-stubs /opt/curl-static /opt/rapidjson /opt/sdl2-headers /usr)
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
TC


mkdir -p "$BUILD" /out
cd "$BUILD"
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
  -DFREETYPE_LIBRARY=$A/libfreetype.so \
  -DFREETYPE_LIBRARY_RELEASE=$A/libfreetype.so \
  -DFREETYPE_INCLUDE_DIR_ft2build=$I/freetype2 \
  -DFREETYPE_INCLUDE_DIR_freetype2=$I/freetype2 \
  -DSDL2_LIBRARY=$A/libSDL2.so \
  -DSDL2_INCLUDE_DIR=/usr/include/SDL2 \
  -DALSA_LIBRARY=$A/libasound.so \
  -DALSA_INCLUDE_DIR=$I \
  -DRAPIDJSON_INCLUDE_DIRS=/opt/rapidjson/include \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--allow-shlib-undefined" \
  -DCMAKE_CXX_STANDARD_LIBRARIES="-L/opt/mbedtls/lib -L$A -lmbedtls -lmbedx509 -lmbedcrypto -lz -lpthread" \
  >"$BUILD/cmake.log" 2>&1 || { tail -40 "$BUILD/cmake.log"; exit 1; }

cmake --build . -j"$(nproc)" >"$BUILD/make.log" 2>&1 || { grep -B3 -E "error|undefined reference" "$BUILD/make.log" | tail -40; exit 1; }

# ES's CMake writes the binary next to the sources
$CROSS-strip -o /out/emulationstation /src/emulationstation
cp $A/libfreetype.so.6 /out/libfreetype.so.6
ls -la /out
