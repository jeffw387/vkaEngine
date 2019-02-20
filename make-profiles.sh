#!/bin/bash

GCC_PR=gccprofile
CLANG_PR=clangprofile
conan profile new --detect $GCC_PR
conan profile new --detect $CLANG_PR
conan profile update settings.compiler.libcxx=libstdc++11 $GCC_PR;
conan profile update settings.compiler.libcxx=libc++ $CLANG_PR;
conan profile update settings.cppstd=17 $GCC_PR
conan profile update settings.cppstd=17 $CLANG_PR