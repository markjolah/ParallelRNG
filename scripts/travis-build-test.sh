#!/bin/bash
# scripts/travis-build-test.sh <cmake args...>
#
# Continuous integration build and test script.
#
# Environment variables to be set:
#  BUILD_TYPE
#  OPT_ARMADILLO_INT64
#
PACKAGE_NAME=ParallelRngManager
if [ -z "$BUILD_TYPE" ]; then echo "No BUILD_TYPE"; exit 1; fi
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SRC_PATH=${SCRIPT_DIR}/..
INSTALL_PATH=${SCRIPT_DIR}/../_travis.install
BUILD_PATH=${SCRIPT_DIR}/../_travis.build/${BUILD_TYPE}
NUM_PROCS=`grep -c ^processor /proc/cpuinfo`

ARGS="-DCMAKE_INSTALL_PREFIX=$INSTALL_PATH"
ARGS="${ARGS} -DBUILD_STATIC_LIBS=OFF"
ARGS="${ARGS} -DBUILD_SHARED_LIBS=ON"
ARGS="${ARGS} -DBUILD_TESTING=On"
ARGS="${ARGS} -DOPT_INSTALL_TESTING=On"
ARGS="${ARGS} -DOPT_EXPORT_BUILD_TREE=On"
if [ -z "$OPT_ARMADILLO_INT64" ]; then
    ARGS="${ARGS} -DOPT_ARMADILLO_INT64=On"
else
    ARGS="${ARGS} -DOPT_ARMADILLO_INT64=$OPT_ARMADILLO_INT64"
fi

set -ex
rm -rf $INSTALL_PATH $BUILD_PATH
cmake -H$SRC_PATH -B$BUILD_PATH -DCMAKE_BUILD_TYPE=$BUILD_TYPE $ARGS $@
cmake --build $BUILD_PATH --target install -- -j$NUM_PROCS


if [ "$BUILD_TYPE" == "Debug" ]; then
    SUFFIX=".debug"
fi
#CTest in build directory
cmake --build $BUILD_PATH --target test -- -j$NUM_PROCS
#Run test in install directory
${INSTALL_PATH}/lib/${PACKAGE_NAME}/test/test${PACKAGE_NAME}${SUFFIX}
