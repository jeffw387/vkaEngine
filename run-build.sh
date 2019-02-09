#!/bin/bash

mkdir -p build
cd build
conan install .. --build=missing --profile=$CONANPROFILE
conan build ..