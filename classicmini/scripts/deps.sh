#!/bin/sh
set -e
export PATH=$PATH
CROSS=arm-linux-gnueabihf
PFX=/w/deps/sysroot
mkdir -p $PFX /w/deps/build
cd /w/deps/build
which unzip python3 || true
if [ ! -f $PFX/lib/libcurl.a ]; then
  rm -rf curl-7.88.1; tar xzf /w/deps/dl/curl-7.88.1.tar.gz; cd curl-7.88.1
  ./configure --host=$CROSS --prefix=$PFX --disable-shared --enable-static --without-ssl --without-libpsl --without-brotli --without-zstd --without-nghttp2 --disable-ldap --disable-ldaps --disable-rtsp --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-mqtt --disable-manual --disable-docs >/w/deps/curl-conf.log 2>&1
  make -j4 >/w/deps/curl-make.log 2>&1 && make install >/dev/null 2>&1
  cd ..
fi
ls $PFX/lib
