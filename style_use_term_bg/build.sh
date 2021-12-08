#!/usr/bin/env bash
gcc -o style_use_term_bg.so style_use_term_bg.c $(yed --print-cflags) $(yed --print-ldflags)