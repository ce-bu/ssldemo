Example client/server SSL programming


## Building
```
# install few packages
sudo apt install -y libgoogle-glog-dev libgflags-dev 

# generate cmake file
./scripts/bld prep

# build
./scripts/bld

# setup certificates
./scripts/sslconf.sh

```

## Running

```
# server
./build/src/server

# client
./build/src/client

```

## Compile OpenSSL
```
git clone https://github.com/openssl/openssl.git

./config --prefix=/opt/openssl \
    --openssldir=/opt/openssl \
    no-shared enable-ssl2 \
    enable-ssl3 \
    enable-weak-ssl-ciphers \
    enable-ssl-trace
```
