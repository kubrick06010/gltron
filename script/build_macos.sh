#!/bin/sh
set -eu

CONFIGURATION="${CONFIGURATION:-Debug}"
DERIVED_DATA_PATH="${DERIVED_DATA_PATH:-/tmp/gltron-derived}"

xcodebuild \
  -project XCode2/GLtron.xcodeproj \
  -scheme GLtron.app \
  -configuration "$CONFIGURATION" \
  -derivedDataPath "$DERIVED_DATA_PATH" \
  CODE_SIGNING_ALLOWED=NO \
  build

APP_PATH="$DERIVED_DATA_PATH/Build/Products/$CONFIGURATION/GLtron.app"
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"
RUNTIME_DYLIBS="
third_party/macos-arm64/sdl12-compat/lib/libSDL-1.2.0.dylib
third_party/macos-arm64/libopenmpt/lib/libopenmpt.0.dylib
third_party/macos-arm64/mpg123/lib/libmpg123.0.dylib
third_party/macos-arm64/libogg/lib/libogg.0.dylib
third_party/macos-arm64/libvorbis/lib/libvorbis.0.dylib
third_party/macos-arm64/libvorbis/lib/libvorbisfile.3.dylib
"

mkdir -p "$FRAMEWORKS_PATH"
for dylib in $RUNTIME_DYLIBS; do
  cp "$dylib" "$FRAMEWORKS_PATH/"
done

echo "$APP_PATH"
