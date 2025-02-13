#!/usr/bin/env bash
apt-get -y update
apt-get -y install libxml2-dev libsctp-dev libpcap-dev
cd build; make
