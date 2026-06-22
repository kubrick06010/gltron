# gltron, a 3d lightcycle game using OpenGL 

Copyright (C) 1999 Andreas Umbach <marvin@dataway.ch>

Soundtrack:
gltron.it - 'Revenge of Cats' is copyright by Peter Hajba <skaven@remedy.fi>

[Latest manual & information location](http://www.gltron.org/)

## Installation from source:

see the INSTALL file in this archive

## macOS Xcode build

The maintained macOS build entry point is:

```sh
./script/build_macos.sh
```

It builds the `GLtron.app` scheme from `XCode2/GLtron.xcodeproj` with code
signing disabled and writes DerivedData to `/tmp/gltron-derived` by default.
Override `CONFIGURATION` or `DERIVED_DATA_PATH` when needed:

```sh
CONFIGURATION=Release DERIVED_DATA_PATH=/tmp/gltron-release ./script/build_macos.sh
```

Current macOS builds expect Homebrew `sdl12-compat`, `libpng`, and `libopenmpt` to be
available under `/opt/homebrew/opt`. The Xcode target builds without
`SDL_sound`; the macOS audio path uses SDL for mixing and `libopenmpt` for the
bundled module music.

Run the macOS bundle spec with:

```sh
./tests/macos_bundle_spec.sh
```

The spec rebuilds the app and verifies that the generated bundle contains the
GLtron executable plus the expected `data`, `sounds`, `scripts`, `levels`,
`music`, and per-art-pack resource directories.

## Changes

See [ChangeLog](ChangeLog).

### Original Repositories

This is a git migration from the original source-code
from Andreas Umbach's [svn repository](https://svn.code.sf.net/p/gltron/code)
as well as some cherry picked fixes of his [git repository](https://git.code.sf.net/p/gltron/git).

Original branches and tags left intact.
However, emails addresses of the other authors are unknown to me
and must be eventually fixed with a rewrite.

## License:

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
