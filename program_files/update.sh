#!/bin/bash

pkill -f ./proj
git pull
make
./proj