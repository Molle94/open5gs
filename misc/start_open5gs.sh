#!/bin/bash

export DB_URI="mongodb://localhost/open5gs"

mongod --smallfiles --dbpath /var/lib/mongodb --logpath /open5gs/install/var/log/open5gs/mongodb.log --logRotate reopen --logappend &

sleep 10 && cd /open5gs/webui && npm run dev &

cd /

if ! grep "ogstun" /proc/net/dev > /dev/null; then
    ip tuntap add name ogstun mode tun
    ip addr add 10.45.0.1/16 dev ogstun
    ip link set ogstun up
    iptables -t nat -A POSTROUTING -s 10.45.0.1/16 ! -o ogstun -j MASQUERADE
fi

./open5gs/install/bin/open5gs-nrfd & 
sleep 5
./open5gs/install/bin/open5gs-smfd &
./open5gs/install/bin/open5gs-amfd & 
./open5gs/install/bin/open5gs-ausfd &
./open5gs/install/bin/open5gs-udmd &
./open5gs/install/bin/open5gs-udrd &
./open5gs/install/bin/open5gs-pcfd &
./open5gs/install/bin/open5gs-bsfd &
./open5gs/install/bin/open5gs-nssfd &
./open5gs/install/bin/open5gs-upfd
