#!/bin/bash

MATTER_ENV_PATH=$PATH

export PATH=/usr/local/miniconda/bin:/usr/local/miniconda/condabin:/opt/cargo-home/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

cd /uic

cmake -DCMAKE_INSTALL_PREFIX=$1/stage -GNinja -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm64_debian.cmake  -B build_unify_arm64/ -S components -DBUILD_TESTING=OFF

cmake --build build_unify_arm64

cmake --install build_unify_arm64 --prefix $1/stage

export PATH=$MATTER_ENV_PATH