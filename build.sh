#!/bin/bash -e

BUILD_DIR=$PWD/build
JOBS=$(nproc)

caller_dir=$PWD
# Move to the current script directory
cd "${0%/*}"

while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    -c|--clean)
    rm -rf $BUILD_DIR
    shift # past argument
    ;;
    -j|--jobs)
    JOBS="$2"
    shift # past argument
    shift # past value
    ;;
    *)
    echo "Unrecognized argument: $key"
    exit 1
esac
done

export GTEST_COLOR=1
export CTEST_OUTPUT_ON_FAILURE=1

mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake ..
make -j $JOBS

cd $BUILD_DIR/unit-tests
ctest -V --repeat-until-fail 1

# Go back to the initial directory when you are done
cd $caller_dir