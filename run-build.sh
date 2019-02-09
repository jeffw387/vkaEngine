#!/bin/bash

./make-profiles.sh
mkdir -p build
cd build
echo -e conan install .. --build=missing --profile=$CONANPROFILE
conan build ..