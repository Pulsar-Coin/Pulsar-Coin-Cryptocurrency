#!/bin/bash -ev

mkdir -p ~/.pulsar
echo "rpcuser=username" >>~/.pulsar/pulsar.conf
echo "rpcpassword=`head -c 32 /dev/urandom | base64`" >>~/.pulsar/pulsar.conf

