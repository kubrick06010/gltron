# Building GLtron

GLtron is a legacy SDL 1.2/OpenGL application. The repository keeps its
original build systems, with small wrapper scripts for the currently maintained
paths.

## Supported build paths

| Platform | Build path | Current scope |
| --- | --- | --- |
| Linux | Autotools | Tested on Debian 12/13-compatible systems, including music playback |
| Windows | Visual Studio 2022 / MSBuild | Builds a 32-bit (`Win32`) executable and distributable ZIP |
| macOS | Xcode | Apple Silicon (`arm64`) build with vendored dependencies |

Run every command below from the repository root.

## Linux

On Debian or Ubuntu, install the compiler and development libraries:

```sh
sudo apt update
sudo apt install \
  autoconf automake build-essential \
  libgl1-mesa-dev libglu1-mesa-dev \
  libmikmod-dev libpng-dev \
  libsdl1.2-dev libsdl-sound1.2-dev \
  zlib1g-dev
```

Generate the Autotools files and build GLtron:

```sh
./autogen.sh
./configure --enable-localdata
make -j"$(nproc)"
./gltron
```

`--enable-localdata` makes the executable use the resources in the checkout,
so installation is not required. Omit it and run `sudo make install` after
`make` if a system-wide installation is preferred.

Music playback needs SDL_sound and libmikmod. Do not pass `-s`, because that
option deliberately disables all sound. The bundled tracker module is
`music/song_revenge_of_cats.it`.

## Windows

Requirements:

- Windows 10 or 11.
- Visual Studio 2022 or Visual Studio Build Tools 2022.
- The **Desktop development with C++** workload, MSVC v143 toolset, and a
  Windows SDK.
- Windows PowerShell 5.1 or newer.

Build and package the application:

```powershell
.\script\package_windows.ps1
```

If local policy blocks scripts, use a process-only override:

```powershell
powershell -ExecutionPolicy Bypass -File .\script\package_windows.ps1
```

The script expands the tracked `win32\dependencies.zip`, builds the Release
configuration, and creates:

```text
dist\gltron-windows-0.72\
dist\gltron-windows-0.72.zip
```

Unpack the ZIP and run `bin\gltron.exe`. The current Windows target is 32-bit,
even on a 64-bit host. If Windows reports a missing Visual C++ runtime, install
the Microsoft Visual C++ 2015-2022 Redistributable for x86.

Optional parameters include:

```powershell
.\script\package_windows.ps1 -Configuration Release -Platform Win32 -Version 0.72
```

## macOS

Requirements:

- An Apple Silicon Mac.
- A full Xcode installation with its command-line tools selected.

Confirm that Xcode is available and build the app:

```sh
xcodebuild -version
./script/build_macos.sh
open /tmp/gltron-derived/Build/Products/Debug/GLtron.app
```

The repository contains the required arm64 SDL compatibility, libpng,
libopenmpt, mpg123, Ogg, and Vorbis libraries under
`third_party/macos-arm64`. The script embeds the runtime libraries and applies
an ad-hoc signature after assembling the app bundle.

For a Release build:

```sh
CONFIGURATION=Release \
DERIVED_DATA_PATH=/tmp/gltron-release \
./script/build_macos.sh

open /tmp/gltron-release/Build/Products/Release/GLtron.app
```

Run the bundle regression check with:

```sh
./tests/macos_bundle_spec.sh
```

## Known limits

- There is not yet a hosted continuous-integration matrix for all three
  platforms.
- The Windows result is `Win32`; a native x64 target remains future work.
- The macOS dependencies are arm64-only; Intel and universal builds are not
  currently supported.
- Generated Autotools files, local build directories, extracted Windows
  dependencies, and packages are intentionally ignored by Git.
