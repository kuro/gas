#!/bin/sh

cd ..
file="${1/$PWD\//}"
git whatchanged -n1 --pretty=oneline -- "${file}" | head -n1 | cut -d ' ' -f 1
