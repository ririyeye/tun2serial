#!/bin/sh
{
while true
do

/root/serial2tun -s /dev/ttyGS0 -full 
sleep 1
done

}&

sleep 20
mount -t nfs -o nolock 192.168.50.160:/nucroot /home
