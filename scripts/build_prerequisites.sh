#!/bin/bash

MATTER_ENV_PATH=$PATH

export PATH=/usr/local/miniconda/bin:/usr/local/miniconda/condabin:/opt/cargo-home/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

cd /uic

if [ "$2" == "amd64" ]; then
    cmake -DCMAKE_INSTALL_PREFIX=$1/stage -GNinja -B build_unify_$2/ -S components
else
    cmake -DCMAKE_INSTALL_PREFIX=$1/stage -GNinja -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/${2}_debian.cmake -B build_unify_$2/ -S components -DBUILD_TESTING=OFF
fi

cmake --build build_unify_${2}

cmake --install build_unify_${2} --prefix $1/stage

export PATH=$MATTER_ENV_PATH