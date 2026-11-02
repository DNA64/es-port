# ES-RetroPie cross-build for the SNES/NES Classic

Build check only: it compiles for the console's CPU and libraries. It was **not run on a console**.

- Base: `RetroPie/EmulationStation`, branch `stable` (`a31c36a`, v2.12.0 merge); remote `upstream`. `external/pugixml` is a
  submodule, fetch it (`git submodule update --init --depth 1`).
- The changes are committed in this branch (`classicmini`, based on `stable`): libvlc removed (video component replaced by a stub that shows the snapshot), `std::put_time`
  replaced (libstdc++ 4.9 has none), `SDL_JoystickGetVendor/Product` guarded (SDL 2.0.4 has none).
- `scripts/`: run in the `classicmini-cross-toolchain:jessie-armhf` image with the work folder mounted at `/w`
  (`/w/es-src` (this repository), `/w/deps/dl` with the FreeImage 3.18.0 zip, curl 7.88.1, a `rapidjson` clone and CMake 3.27 under
  `/w/deps/cmake`; `/w/fw/usr/lib/libMali.so` is the console's driver, used only to link). Order: `deps.sh` (libcurl),
  `fi.sh` (FreeImage), `es.sh` (configure), then `cmake --build`. The image's own CMake (3.0.2) is too old for ES and
  pugixml, so a newer one is used. On Git Bash set `MSYS_NO_PATHCONV=1` before `docker run`.
