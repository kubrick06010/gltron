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

echo "$DERIVED_DATA_PATH/Build/Products/$CONFIGURATION/GLtron.app"
