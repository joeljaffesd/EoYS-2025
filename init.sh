#!/bin/sh

rm -rf allolib
rm -rf al_ext

git submodule update --init --recursive

# generate .h files for all NAM models
find resources -name "*.nam" | while read -r file; do
  python RTNeural/modules/rt-nam/nam_to_header.py "$file" "${file%.nam}.h"
done