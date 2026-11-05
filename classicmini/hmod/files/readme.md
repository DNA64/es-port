# EmulationStation

EmulationStation 2.12 (RetroPie fork) as the front end of the NES/SNES Classic, driven by the games hakchi syncs.

Needs the RetroArch (S)NESCE hmod for the libretro cores.

**Needs an SD card or USB stick.** While it installs, the hmod mounts the card itself and puts its default theme, [es-theme-simple](https://github.com/RetroPie/es-theme-simple), in `/media/data/emulationstation/themes/simple`; nothing of the theme goes to the NAND. Without a card or stick the install stops and EmulationStation is not installed; the hakchi log says so, but the PC may still report the install as done and mark the module as installed, so remove it in hakchi before trying again. Extra themes go in the same folder. Updating keeps the theme files you changed.

Hold Select while the console boots to get the stock UI. After two fast crashes in a row it falls back to the stock UI by itself.

Uninstalling removes EmulationStation, its settings and library cache from the NAND but keeps the favourites list. It also mounts the card and removes the theme files you did not change; the ones you changed or added stay.
