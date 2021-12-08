#!/usr/bin/env bash
gcc -o nord.so nord.c $(yed --print-cflags) $(yed --print-ldflags)