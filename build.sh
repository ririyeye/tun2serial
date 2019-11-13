#!/bin/bash

mkdir arm
cd arm
cmake .. -Darch=arm 
make -j8


cd -
mkdir x86
cd x86
cmake .. -Darch=x86
make -j8

cp serial2tun /nfs
