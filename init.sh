#!/bin/sh

# clears allolib and al_ext folders if they exist
rm -rf allolib
rm -rf al_ext

# get submodules
git submodule update --init --recursive

# generate .h files for all NAM models
find assets -name "*.nam" | while read -r file; do
  python3 RTNeural/modules/rt-nam/nam_to_header.py "$file" "${file%.nam}.h"
done