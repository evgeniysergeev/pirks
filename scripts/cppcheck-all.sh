#!/bin/bash

# Better to call this file with redirect stdout to file. This way you will see
# only errors and warnings on the screen
# ./scripts/cppcheck-all.sh > cppcheck.log

project_path=$(pwd -P)

echo $project_path

mkdir -p cppcheck-build
pushd cppcheck-build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# Note that it is possible to skip checking third-party directory
# by using this command line option: -i$project_path/third-party
# But for now checking everything
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
    -j 4 \
    --project=compile_commands.json

popd
