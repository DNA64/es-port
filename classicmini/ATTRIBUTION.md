# Attribution

## Super Nintendo logo

`device/theme/games/assets/logo_snes.png` is adapted from "Logo SNES.png" by Juanito23n64
(<https://commons.wikimedia.org/wiki/File:Logo_SNES.png>), licensed under the
[Creative Commons Attribution-Share Alike 3.0 Unported](https://creativecommons.org/licenses/by-sa/3.0/) licence.

Changes: the wordmark is recoloured to `#c8c8d9` with dark lettering in the banner, and the coloured buttons are
widened horizontally. The adapted file is available under the same licence.

## Fonts

Both are licensed under the [SIL Open Font License 1.1](https://openfontlicense.org); each licence text sits next
to its font.

- Noto Sans, by The Noto Project Authors (<https://github.com/notofonts/latin-greek-cyrillic>):
  `device/theme/games/assets/fonts/NotoSans-Regular.ttf` and `NotoSans-SemiBold.ttf`, and
  `device/resources/fallback_latin.ttf`, the Regular weight reduced to Latin Extended-A/B and Latin Extended
  Additional, used as the fallback for accented letters such as macrons.
- Noto Sans JP, by Adobe and The Noto Project Authors (<https://github.com/notofonts/noto-cjk>):
  `device/resources/fallback_ja.ttf`, the fallback for Japanese glyphs. It is the variable font instantiated at
  weight 400 and reduced to kana, punctuation and the JIS X 0208 kanji.

## CA certificates

`device/resources/cacert.pem` is the Mozilla CA certificate bundle as extracted by the curl project
(<https://curl.se/docs/caextract.html>), licensed under the [Mozilla Public License 2.0](https://www.mozilla.org/MPL/2.0/).
It is unmodified; the console has no certificate store, so the scrapers use it to verify HTTPS.

## MP3 decoding

`es-core/src/minimp3/` is minimp3 by lieff (<https://github.com/lieff/minimp3>), released under CC0 1.0; the
licence text sits next to the headers.

## Menu audio and covers

The menu sound effects and the default background music are imported on the console from the firmware's own
files at first run and are not shipped in this repository. Cover images are user data kept on the console in
`/media/data/emulationstation/covers`; this repository ships only the alias table that maps console game codes to
cover names, and a script to shrink covers.

## DefKorns logo

`resources/defkorns_logo.png` is the DefKorns logo, used on the About screen.

"Super Nintendo Entertainment System" and the logo are trademarks of Nintendo. This project is not affiliated with
or endorsed by Nintendo.
