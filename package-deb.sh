#!/bin/bash

VERSION=0.0.1

cp build_release/cygnus packageroot/usr/local/bin/cygnus
dpkg-deb -b packageroot "cygnus_${VERSION}_all.deb"
