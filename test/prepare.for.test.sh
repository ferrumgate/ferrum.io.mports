#!/bin/bash
sudo ip link add name dummy0 up type dummy
FORWARD_IP=100.100.100.100
echo "adding ip address $FORWARD_IP/32 to dummy0 interface"
sudo ip addr add $FORWARD_IP/32 dev dummy0 || true

LISTEN_IP=192.168.105.105
echo "adding ip address $LISTEN_IP/32 to lo interface"
sudo ip addr add $LISTEN_IP/32 dev lo || true

docker stop nginx-mports || true
docker run -it --rm -d -p $FORWARD_IP:8080:80 --name nginx-mports nginx
