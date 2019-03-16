#!/bin/bash

cd /data1/conf/deploy/svncount

make clean

make

[ $? == "0" ] && make install
