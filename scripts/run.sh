#!/bin/bash

if [ "$1" = "release" ]
then
    ./build/release/pathfindng-visualiser
else
    ./build/debug/pathfindng-visualiser
fi