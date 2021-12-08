#!/bin/bash
gcc -o ypm.so -g ypm.c $(yed --print-cflags) $(yed --print-ldflags)
