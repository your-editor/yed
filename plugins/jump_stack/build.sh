#!/usr/bin/env bash
gcc -o jump_stack.so jump_stack.c $(yed --print-cflags) $(yed --print-ldflags)