#!/bin/sh


ip=192.168.0.11

while true
do
./serial2tun -s /dev/ttyACM0 -full -ip $ip
sleep 1
./serial2tun -s /dev/ttyACM1 -full -ip $ip
sleep 1
done

