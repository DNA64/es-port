# ES-RetroPie for the NES/SNES Classic

Branch `classicmini`, based on `stable` (`a31c36a`, v2.12.0 merge); `upstream` is `RetroPie/EmulationStation`.
It compiles for the console's CPU and libraries. It has **not been run on a console**.

## Changes against `stable`

- libvlc removed: the video component is a stub that shows the snapshot image.
- `std::put_time` replaced by `strftime` (libstdc++ 4.9 has none).
- `SDL_JoystickGetVendor/Product` guarded (SDL 2.0.4 on the console has none).

## Build

```sh
classicmini/build.sh
```

Only docker is needed on the host. The build runs in `classicmini-es-toolchain`, built from `Dockerfile.es-armhf` in
[classicmini-cross-toolchain](https://github.com/DefKorns/classicmini-cross-toolchain) on top of the shared toolchain
image (GCC 4.9.4 / glibc 2.21 for Cortex-A7, the console's SDL2 2.0.4, FreeType, ALSA, static libcurl on mbedTLS). The
ES image adds CMake 3.27, static FreeImage, RapidJSON and link stubs for `libGLESv2`; nothing is taken from a console.
`ES_TOOLCHAIN_IMAGE` selects another tag (default `classicmini-es-toolchain:local`).

`scripts/build-es.sh` runs inside the container with the repository mounted: it fetches the `pugixml` submodule,
configures with `-DGLES=ON`, builds and strips. The build tree stays in `classicmini/out/.build` so rebuilds are
incremental. ES's CMake writes the `emulationstation` binary into the repository root; it is ignored by git.

Result in `classicmini/out/`: `emulationstation` (stripped ARM hard-float ELF, about 7.9 MB) and `libfreetype.so.6`, which
ships with the program. At run time it needs the console's `libSDL2.so`, `libasound.so.2` and `libGLESv2.so`.
