# macOS arm64 vendored dependencies

This directory contains the macOS arm64 libraries needed by the Xcode build so
GLtron can build without a Homebrew installation.

Sources:

- `sdl12-compat` 1.2.68 from Homebrew, copied from
  `/opt/homebrew/opt/sdl12-compat`
- `libpng` 1.6.53 from Homebrew, copied from `/opt/homebrew/opt/libpng`
- `libopenmpt` 0.8.7 from Homebrew, copied from
  `/opt/homebrew/opt/libopenmpt`
- `mpg123` 1.33.6 from Homebrew, copied from `/opt/homebrew/opt/mpg123`
- `libogg` 1.3.6 from Homebrew, copied from `/opt/homebrew/opt/libogg`
- `libvorbis` 1.3.7 from Homebrew, copied from
  `/opt/homebrew/opt/libvorbis`

The copied artifacts are:

- SDL 1.2 compatibility headers, `libSDL.dylib`, `libSDL-1.2.0.dylib`, and
  `libSDLmain.a`
- libpng headers and static archives
- libopenmpt headers and dylib
- mpg123, libogg, and libvorbis runtime dylibs used by libopenmpt

Vendored runtime dylibs have their install names rewritten to load from
`@executable_path/../Frameworks`. The build script copies those dylibs into the
app bundle after `xcodebuild` completes.

License files and upstream readmes are stored under each dependency's
`licenses` directory.
