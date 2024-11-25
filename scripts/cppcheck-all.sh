#!/bin/bash

mkdir -p cppcheck-build
pushd cppcheck-build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

cppcheck \
    --enable=all \
    --std=c++20 \
    --project=compile_commands.json \
    --cppcheck-build-dir=. \
    --check-level=exhaustive \
    --inconclusive \
    --force \
    --template=gcc \
    --library=std.cfg \
    --suppressions-list=../scripts/suppressions.txt \
    -j 4

popd
