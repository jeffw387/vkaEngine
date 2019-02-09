#!/bin/bash

cd build
conan install .. --build=missing --profile=$CONANPROFILE
conan build ..