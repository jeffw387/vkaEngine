#!/bin/bash

mkdir -p profiles
rm profiles/*
GCC_PR=./profiles/gccprofile
CLANG_PR=./profiles/clangprofile
conan profile new --detect $GCC_PR
conan profile new --detect $CLANG_PR
conan profile update settings.compiler.libcxx=libstdc++ $GCC_PR;
conan profile update settings.compiler.libcxx=libc++ $CLANG_PR;
conan profile update settings.cppstd=17 $GCC_PR
conan profile update settings.cppstd=17 $CLANG_PR