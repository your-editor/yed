#!/usr/bin/env bash
gcc -o hook.so hook.c $(yed --print-cflags) $(yed --print-ldflags)