#!/usr/bin/env bash
gcc -o scroll_buffer.so scroll_buffer.c $(yed --print-cflags) $(yed --print-ldflags)