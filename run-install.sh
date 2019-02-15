#!/bin/bash

set -x
cd build
conan install .. --build=missing --profile=$CONANPROFILE