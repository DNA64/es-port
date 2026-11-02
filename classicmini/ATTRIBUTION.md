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
  `device/theme/games/assets/fonts/NotoSans-Regular.ttf` and `NotoSans-SemiBold.ttf`.
- Noto Sans JP, by Adobe and The Noto Project Authors (<https://github.com/notofonts/noto-cjk>):
  `device/resources/fallback_ja.ttf`, the fallback for Japanese glyphs. It is the variable font instantiated at
  weight 400 and reduced to kana, punctuation and the JIS X 0208 kanji.

## CA certificates

`device/resources/cacert.pem` is the Mozilla CA certificate bundle as extracted by the curl project
(<https://curl.se/docs/caextract.html>), licensed under the [Mozilla Public License 2.0](https://www.mozilla.org/MPL/2.0/).
It is unmodified; the console has no certificate store, so the scrapers use it to verify HTTPS.

"Super Nintendo Entertainment System" and the logo are trademarks of Nintendo. This project is not affiliated with
or endorsed by Nintendo.
