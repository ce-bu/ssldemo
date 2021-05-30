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
