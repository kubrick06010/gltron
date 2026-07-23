# gltron, a 3d lightcycle game using OpenGL 

Copyright (C) 1999 Andreas Umbach <marvin@dataway.ch>

Soundtrack:
gltron.it - 'Revenge of Cats' is copyright by Peter Hajba <skaven@remedy.fi>

The original project website was [gltron.org](http://www.gltron.org/). It is
kept here as a historical reference; maintained build information lives in
this repository.

## Building from source

See [BUILDING.md](BUILDING.md) for complete requirements, platform limits, and
troubleshooting notes.

Quick entry points:

```text
Linux:   ./autogen.sh && ./configure --enable-localdata && make
Windows: .\script\package_windows.ps1
macOS:   ./script/build_macos.sh
```

The maintained configurations are Linux with Autotools, Windows `Win32` with
Visual Studio 2022, and macOS `arm64` with Xcode.

## Project status

GLtron is a legacy project. Its original source and resource layout is kept
intact while build compatibility, packaging, and crash fixes are maintained.
The active work list is in [TODO](TODO); older design notes under `docs/` are
retained as historical references.

## Changes

See [ChangeLog](ChangeLog).

### Original Repositories

This is a git migration from the original source-code
from Andreas Umbach's [svn repository](https://svn.code.sf.net/p/gltron/code)
as well as some cherry picked fixes of his [git repository](https://git.code.sf.net/p/gltron/git).

Original branches and tags left intact.
However, email addresses of the other authors are unknown to me
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
