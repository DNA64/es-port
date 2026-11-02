#!/bin/sh
set -e
CROSS=arm-linux-gnueabihf
PFX=/w/deps/sysroot
cd /w/deps/build/FreeImage
find . -name "*.o" -delete
export CFLAGS="-O2 -fPIC -fexceptions -fvisibility=hidden -DPNG_ARM_NEON_OPT=0"
export CXXFLAGS="-O2 -fPIC -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy"
make -f Makefile.gnu -j4 CC=$CROSS-gcc CXX=$CROSS-g++ AR=$CROSS-ar LD=$CROSS-ld libfreeimage.a >/w/deps/fi-make.log 2>&1 || { tail -25 /w/deps/fi-make.log; exit 1; }
mkdir -p $PFX/lib $PFX/include
cp libfreeimage.a $PFX/lib/
cp Source/FreeImage.h $PFX/include/
ls -la $PFX/lib
