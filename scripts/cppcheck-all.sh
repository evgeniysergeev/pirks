#!/bin/bash

project_path=$(pwd -P)

echo $project_path

mkdir -p cppcheck-build
pushd cppcheck-build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

cppcheck \
    --enable=all \
    --std=c++20 \
    --cppcheck-build-dir=. \
    --check-level=exhaustive \
    --inconclusive \
    --force \
    --template=gcc \
    --library=std.cfg \
    --inline-suppr \
    --suppressions-list=../scripts/suppressions.txt \
    -i_deps \
    -i$project_path/third-party \
    -j 4 \
    --project=compile_commands.json

popd
