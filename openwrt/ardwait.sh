#!/bin/bash
echo -n "iziz" > /dev/ttyUSB0
while [ 1 ]; do
	if  ! [ -f /var/ardresprunning ]; then	 
		cat /var/ardresp > /var/resp
		break;
	fi
done
rm /var/ardresp
echo "$(cat /var/resp)"
