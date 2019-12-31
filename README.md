# sense-hat-socket
Access the Raspberry Pi sense-hat IMU data from any language with a UNIX Socket

# Installation
```bash
sudo apt update
sudo apt install -y sense-hat
git clone https://github.com/alexdrean/sense-hat-socket
cd sense-hat-socket
make
sudo make install
```

# Usage
```bash
netcat -U /run/sense-hat/imu.sock
```

Output
```
: roll:-1.755584, pitch:0.184328, yaw:-35.736270
: roll:-1.754425, pitch:0.185284, yaw:-35.738821
: roll:-1.752745, pitch:0.186840, yaw:-35.741755
...
```
