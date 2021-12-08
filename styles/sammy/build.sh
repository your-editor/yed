#!/usr/bin/env bash
gcc -o sammy.so sammy.c $(yed --print-cflags) $(yed --print-ldflags)