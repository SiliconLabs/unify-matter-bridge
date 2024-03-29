#!/bin/bash

MATTER_ENV_PATH=$PATH

export PATH=/usr/local/miniconda/bin:/usr/local/miniconda/condabin:/opt/cargo-home/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

cd "$3"

if [ "$2" == "amd64" ]; then
toolchain=" "
elif [ "$2" == "arm" ]; then
toolchain="-DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/armhf_debian.cmake "
elif [ "$2" == "arm64" ]; then
toolchain="-DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/arm64_debian.cmake "
else
exit 1  # Return failure for non-supported target architecture.
fi

cmake -DCMAKE_INSTALL_PREFIX=$1/stage -GNinja $toolchain -B build_unify_$2/ -S components -DBUILD_TESTING=OFF

cmake --build build_unify_${2}

cmake --install build_unify_${2} --prefix $1/stage

export PATH=$MATTER_ENV_PATH