#!/bin/bash
#Author:dengshuo
#Time:2020-05-24 10:05:28
#Name:load.sh
#Version:V1.0
#Description:This is a production script.

insmod ./bin/driver.ko
./bin/log &
./bin/daq_function &
./bin/configuration_management &

