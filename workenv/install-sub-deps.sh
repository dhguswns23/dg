#! /bin/bash

DEP_PATH="/deps"

# Download deps and untar xz
curl https://releases.llvm.org/6.0.0/llvm-6.0.0.src.tar.xz --output $DEP_PATH/llvm.src.tar.xz
curl https://releases.llvm.org/6.0.0/cfe-6.0.0.src.tar.xz --output $DEP_PATH/clang.src.tar.xz
curl https://releases.llvm.org/6.0.0/compiler-rt-6.0.0.src.tar.xz --output $DEP_PATH/compiler-rt.src.tar.xz
curl https://releases.llvm.org/6.0.0/clang-tools-extra-6.0.0.src.tar.xz --output $DEP_PATH/extra-tools.src.tar.xz

cd $DEP_PATH

tar xf llvm.src.tar.xz
mv llvm-6.0.0.src llvm

tar xf clang.src.tar.xz
mv cfe-6.0.0.src clang

tar xf compiler-rt.src.tar.xz
mv compiler-rt-6.0.0.src compiler-rt

tar xf extra-tools.src.tar.xz
mv clang-tools-extra-6.0.0.src clang-tools-extra

cp -r clang llvm/tools/clang
cp -r clang-tools-extra llvm/tools/clang-tools-extra
cp -r compiler-rt llvm/tools/compiler-rt

# Build
cd llvm
mkdir build
cd build
cmake ..
make -j8
