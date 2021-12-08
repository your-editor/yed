#!/usr/bin/env bash
gcc -o glsl.so glsl.c $(yed --print-cflags) $(yed --print-ldflags)