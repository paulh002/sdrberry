#!/usr/bin/env bash
set -euo pipefail

HOST="$(hostname)"                          # short name, e.g. pi5b
FQDN="$(hostname -f 2>/dev/null || echo "$HOST")"
IP="$(hostname -I | awk '{print $1}')"      # first non-loopback IPv4

# Build the SAN list: always include .local and the IP
SAN="DNS:${HOST}.local,DNS:${HOST},IP:${IP}"
[ "$FQDN" != "$HOST" ] && SAN="${SAN},DNS:${FQDN}"

openssl req -x509 -newkey rsa:2048 -nodes -days 3650 \
  -keyout key.pem -out cert.crt \
  -subj "/CN=${HOST}.local" \
  -addext "subjectAltName=${SAN}"
sudo mkdir -p /etc/sdr
cat cert.crt key.pem | sudo tee /etc/sdr/cert.pem > /dev/null
sudo chmod 600 /etc/sdr/cert.pem
sudo chown pi /etc/sdr/cert.pem