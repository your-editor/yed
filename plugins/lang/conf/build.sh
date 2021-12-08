#!/usr/bin/env bash
gcc -o conf.so conf.c $(yed --print-cflags) $(yed --print-ldflags)