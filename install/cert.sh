openssl req -x509 -newkey rsa:2048 -nodes -days 3650 \
  -keyout key.pem -out cert.crt \
  -subj "/CN=pi1.local" \
  -addext "subjectAltName=DNS:pi1.local,DNS:pi1.local,IP:192.168.88.31"
  
  
sudo mkdir -p /etc/sdr
cat cert.crt key.pem | sudo tee /etc/sdr/cert.pem > /dev/null
sudo chmod 600 /etc/sdr/cert.pem
sudo chown pi /etc/sdr/cert.pem

openssl x509 -in /etc/sdr/cert.pem -noout -subject -dates -ext subjectAltName