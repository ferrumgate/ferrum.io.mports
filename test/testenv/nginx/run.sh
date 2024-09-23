#!/bin/bash
mkdir -p /tmp/nginx

if [ ! -f "$(pwd)"/nginx_data/1G.file ]; then
  dd if=/dev/zero of="$(pwd)"/nginx_data/1G.file bs=1M count=1024
fi

docker stop nginx-ssl1 || true
docker rm nginx-ssl1 | true
sudo ip addr add 100.100.100.100/32 dev lo || true
docker run --name nginx-ssl1 -d --rm -ti \
  -p 100.100.100.100:8080:8080 \
  -e SSLKEYLOGFILE=/tmp/ssl.log \
  -v /tmp/nginx:/tmp \
  -v "$PWD"/nginxconfig:/etc/nginx \
  -v "$PWD"/nginx_data:/var/www/example.com \
  nginx:1.22-alpine

curl 100.100.100.100:8080
