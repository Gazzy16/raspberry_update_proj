#!/bin/bash

while [ 1 ]
do

pull=$(git pull)
echo "$pull"
if [ "$pull" == "Already up-to-date." ]; then
	sshpass -p '2435' ssh -t gazzy@192.168.0.102 sudo /home/gazzy/raspberry_update_proj/program_files/update.sh
fi
sleep 5
done
