#!/bin/bash
make clean
make $1 CC=x86_64-w64-mingw32-gcc WINDOWS=1