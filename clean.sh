#!/usr/bin/env bash

SO=$(find . -name "*.so")
DSYM=$(find . -name "*.dSYM")
rm -rf ${SO} ${DSYM} _yed include lib
