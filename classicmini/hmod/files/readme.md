# EmulationStation

EmulationStation 2.12 (RetroPie fork) as the front end of the NES/SNES Classic, driven by the games hakchi syncs.

Needs the RetroArch (S)NESCE hmod for the libretro cores.

**Needs an SD card or USB stick.** The default theme, [es-theme-simple](https://github.com/RetroPie/es-theme-simple), is installed to `/media/data/emulationstation/themes/simple` and nothing of it goes to the NAND. Without one the install stops and EmulationStation is not installed; the hakchi log says so, but the PC may still report the install as done. Insert the card or stick and install again. Extra themes go in the same folder. Updating keeps the theme files you changed.

Hold Select while the console boots to get the stock UI. After two fast crashes in a row it falls back to the stock UI by itself.

Uninstalling removes EmulationStation, its settings and library cache from the NAND but keeps the favourites list. The theme on the SD card or USB stick is removed too, except the files you changed or added, which stay.
