# ES-RetroPie for the NES/SNES Classic

Branch `classicmini`, based on `stable` (`a31c36a`, v2.12.0 merge); `upstream` is `RetroPie/EmulationStation`.
EmulationStation 2.12 as the front end of the NES/SNES Classic, driven by the games hakchi syncs, packaged as the
`emulationstation_classicmini` hmod.

## Install

1. Install the **RetroArch (S)NESCE** hmod (the libretro cores and the `retroarch` launcher) and sync your games with
   hakchi. The RetroArch build from KMFDManic may work as well; it has not been tested.
2. Install `emulationstation_classicmini.hmod` with hakchi and sync. The first boot builds the library, which takes a
   while on large collections.
3. Optional: extra themes go on the SD card or USB stick, in `/media/data/emulationstation/themes`. The hmod only
   ships the built-in default theme.

Hold **Select** while the console boots to get the stock UI. If EmulationStation crashes twice in a row right after
starting, the next boot goes to the stock UI by itself. Uninstalling the hmod removes EmulationStation, its settings
and library cache from the NAND and keeps `favorites.txt`; nothing on the SD card or USB stick is touched.

## Build the hmod

```sh
classicmini/build.sh
cd classicmini/hmod
sh stage.sh
sh ../../vendor/hmod-build/docker-make.sh
```

`stage.sh` collects the binary, scripts and resources into `mod/`; the hmod lands in `out/`.

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
