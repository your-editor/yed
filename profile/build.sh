#!/usr/bin/env bash
gcc -o profile.so profile.c $(yed --print-cflags) $(yed --print-ldflags)