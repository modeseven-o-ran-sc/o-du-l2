#!/usr/bin/env bash
sudo apt-get -y update
sudo apt-get -y install libxml2-dev libsctp-dev libpcap-dev libnsl
cd build; make
