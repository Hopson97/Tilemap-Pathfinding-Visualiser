#!/bin/bash

target_release() {
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../..
    make
    echo "Built target in build/release/"
    cd ../..
}

target_debug() {
    cd debug 
    cmake -DCMAKE_BUILD_TYPE=Debug ../..
    make
    echo "Built target in build/debug/"
    cd ../..
}

# Create folder for distribution
if [ "$1" = "release" ]
then
    if [ -d "$pathfindng-visualiser" ]
    then
        rm -rf -d pathfindng-visualiser
    fi

    mkdir -p pathfindng-visualiser
fi

# Creates the folder for the buildaries
mkdir -p pathfindng-visualiser 
mkdir -p pathfindng-visualiser/assets
mkdir -p build
mkdir -p build/release
mkdir -p build/debug
cd build

# Builds target
if [ "$1" = "release" ]
then
    target_release
    cp build/release/pathfindng-visualiser pathfindng-visualiser/pathfindng-visualiser
else
    target_debug
fi

cp -R assets pathfindng-visualiser/
