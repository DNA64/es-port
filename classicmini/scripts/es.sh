#!/bin/sh
set -e
CROSS=arm-linux-gnueabihf
PFX=/w/deps/sysroot
A=/usr/lib/arm-linux-gnueabihf
I=/usr/include/arm-linux-gnueabihf
# link-only copies of the console's Mali driver (local build input, not redistributed)
cp /w/fw/usr/lib/libMali.so $PFX/lib/libMali.so
cp /w/fw/usr/lib/libMali.so $PFX/lib/libGLESv2.so
cp /w/fw/usr/lib/libMali.so $PFX/lib/libEGL.so
cat > /w/deps/toolchain.cmake <<TC
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER $CROSS-gcc)
set(CMAKE_CXX_COMPILER $CROSS-g++)
set(CMAKE_FIND_ROOT_PATH $PFX /opt/sdl2-headers /usr)
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
TC
rm -rf /w/es-build
mkdir -p /w/es-build
cd /w/es-build
/w/deps/cmake/bin/cmake /w/es-src \
  -DCMAKE_TOOLCHAIN_FILE=/w/deps/toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DGLES=ON \
  -DHINT_GLES_INCDIR=/opt/sdl2-headers \
  -DCMAKE_PREFIX_PATH=$PFX \
  -DFreeImage_INCLUDE_DIR=$PFX/include \
  -DFreeImage_LIBRARY_REL=$PFX/lib/libfreeimage.a \
  -DCURL_INCLUDE_DIR=$PFX/include \
  -DCURL_LIBRARY=$PFX/lib/libcurl.a \
  -DFREETYPE_LIBRARY=$A/libfreetype.so -DFREETYPE_LIBRARY_RELEASE=$A/libfreetype.so \
  -DFREETYPE_INCLUDE_DIR_ft2build=$I/freetype2 \
  -DFREETYPE_INCLUDE_DIR_freetype2=$I/freetype2 \
  -DSDL2_LIBRARY=$A/libSDL2.so \
  -DSDL2_INCLUDE_DIR=/usr/include/SDL2 \
  -DALSA_LIBRARY=$A/libasound.so \
  -DALSA_INCLUDE_DIR=$I \
  -DRAPIDJSON_INCLUDE_DIRS=/w/deps/dl/rapidjson/include \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--allow-shlib-undefined" \
  >/w/es-cmake.log 2>&1 || { tail -40 /w/es-cmake.log; exit 1; }
tail -15 /w/es-cmake.log
