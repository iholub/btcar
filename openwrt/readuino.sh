#!/bin/bash

while [ 1 ]; do
	if  ! [ -f /var/ardresp ]; then	 
		echo $$ > /var/ardresprunning
		head -n 1 /dev/ttyUSB0 >/var/ardresp
		rm /var/ardresprunning
	fi
done
