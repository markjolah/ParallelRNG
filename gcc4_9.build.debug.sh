#!/bin/bash
#
# build.gcc4_9.sh
#
#

ARCH=gcc4_9
FULL_ARCH=x86_64-${ARCH}-linux-gnu
TOOLCHAIN_FILE=./cmake/UncommonCMakeModules/Toolchains/Toolchain-${FULL_ARCH}.cmake
INSTALL_PATH=_${ARCH}.install
BUILD_PATH=_${ARCH}.build/Debug
ARGS=""
ARGS="${ARGS} -DBUILD_TESTING=On"
ARGS="${ARGS} -DOPT_EXPORT_BUILD_TREE=On"
rm -rf $INSTALL_PATH $BUILD_PATH

set -ex

cmake -H. -B$BUILD_PATH -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -DCMAKE_INSTALL_PREFIX="$INSTALL_PATH" -DCMAKE_BUILD_TYPE=Debug ${ARGS}
VERBOSE=1 cmake --build $BUILD_PATH --target install -- -j${NUM_PROCS}
