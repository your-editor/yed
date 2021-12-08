#!/usr/bin/env bash
gcc -o vt_light.so vt_light.c $(yed --print-cflags) $(yed --print-ldflags)