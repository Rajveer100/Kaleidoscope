#!/bin/bash

if [[ "$1" == "--clean" ]]; then
  rm -rf build/
  rm compile_commands.json

  cmake -S ./ -B build/ -G Ninja
  ln -s build/compile_commands.json ./
fi

ninja -C build/

./build/bin/main-driver
