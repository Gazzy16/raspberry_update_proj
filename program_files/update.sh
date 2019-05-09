#!/bin/bash

cd /home/gazzy/raspberry_update_proj/program_files
pkill -f ./proj
git pull
make
./proj &