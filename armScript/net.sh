#!/bin/sh
{
while true
do

/root/serial2tun -s /dev/ttyGS0 -full -ip 192.168.0.10 -gateway 192.168.0.11
sleep 1
done

}&