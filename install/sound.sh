cat ~/.asoundrc
cat << EOF | tee ~/.asoundrc

pcm.!default {
        type hw
        card 1
}

ctl.!default {
        type hw
        card 1
}
EOF
sudo sed -i "s/^defaults.ctl.card.*/defaults.ctl.card 1/" /usr/share/alsa/alsa.conf
sudo sed -i "s/^defaults.pcm.card.*/defaults.pcm.card 1/" /usr/share/alsa/alsa.conf
