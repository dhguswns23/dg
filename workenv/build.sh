#! /bin/bash

BASE_DG_DIR=/dg

cd $BASE_DG_DIR
mkdir build
cd build
cmake -B build --DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake ..

make -j4