#!/bin/sh
set -eu

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DERIVED_DATA_PATH="${DERIVED_DATA_PATH:-/tmp/gltron-derived-spec}"
CONFIGURATION="${CONFIGURATION:-Debug}"

cd "$ROOT_DIR"

DERIVED_DATA_PATH="$DERIVED_DATA_PATH" CONFIGURATION="$CONFIGURATION" ./script/build_macos.sh
APP_PATH="$DERIVED_DATA_PATH/Build/Products/$CONFIGURATION/GLtron.app"
RESOURCES="$APP_PATH/Contents/Resources"

test -x "$APP_PATH/Contents/MacOS/GLtron"
test -d "$RESOURCES/data"
test -d "$RESOURCES/sounds"
test -f "$RESOURCES/scripts/main.lua"
test -f "$RESOURCES/levels/simple.lua"
test -f "$RESOURCES/music/song_revenge_of_cats.it"
test -f "$RESOURCES/data/game_engine.wav"
test -f "$RESOURCES/data/game_crash.wav"
test -f "$RESOURCES/data/game_recognizer.wav"
test -f "$RESOURCES/sounds/menu_action.wav"
test -f "$RESOURCES/sounds/menu_highlight.wav"

for artpack in arcade_spots biohazard classic default metalTron; do
  test -f "$RESOURCES/art/$artpack/artpack.lua"
done

test ! -f "$RESOURCES/art/artpack.lua"

echo "macOS bundle spec passed: $APP_PATH"
