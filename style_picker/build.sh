#!/usr/bin/env bash
gcc -o style_picker.so style_picker.c $(yed --print-cflags) $(yed --print-ldflags)