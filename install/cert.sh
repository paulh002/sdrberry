openssl req -x509 -newkey rsa:2048 -nodes -days 3650 \
  -keyout key.pem -out cert.crt \
  -subj "/CN=sdr.local" \
  -addext "subjectAltName=DNS:sdr.local,DNS:pi5b.local,IP:192.168.88.17"
  
  
sudo mkdir -p /etc/sdr
cat cert.crt key.pem | sudo tee /etc/sdr/cert.pem > /dev/null
sudo chmod 600 /etc/sdr/cert.pem
sudo chown pi /etc/sdr/cert.pem

openssl x509 -in /etc/sdr/cert.pem -noout -subject -dates -ext subjectAltName