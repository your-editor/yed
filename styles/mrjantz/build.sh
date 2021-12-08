#!/usr/bin/env bash
gcc -o mrjantz.so mrjantz.c $(yed --print-cflags) $(yed --print-ldflags)