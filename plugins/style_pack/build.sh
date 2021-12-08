#!/usr/bin/env bash
gcc -o style_pack.so style_pack.c $(yed --print-cflags) $(yed --print-ldflags) -w -Wno-error
