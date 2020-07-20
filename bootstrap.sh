#!/bin/sh
if [ ! -d autoconf ]; then mkdir autoconf; fi
autoreconf -vif
