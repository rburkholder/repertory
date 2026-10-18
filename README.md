# repertory

Three libraries:

* MQTT - basic publish/subscribe to one one topic
* REST - send message, listen for commands
  * includes Telegram queries
* Anomalon - serve lua generated web pages

Installation:
```bash
## install required packages
sudo apt install libssl-dev
sudo apt install libpaho-mqtt-dev

## install required build environment, packages and libraries
git clone https://github.com/rburkholder/libs-build.git
pushd libs-build
./build.sh base
./build.sh zlib
./build.sh boost
popd

### build fmt
git clone --depth=1 https://github.com/fmtlib/fmt.git
pushd fmt
mkdir build
cd build
cmake -D FMT_TEST=FALSE ..
make
sudo make install
popd

## build this library
git clone https://github.com/rburkholder/repertory
pushd repertory
mkdir build
cd build
cmake \
-D CMAKE_BUILD_TYPE=Release \
-D OU_USE_STATIC_LIB=ON \
-D OU_USE_SHARED_LIB=OFF \
-D OU_USE_MQTT=ON \
-D OU_USE_REST=ON \
-D OU_USE_Anomalon=ON \
..
sudo cmake --build . --target=install
popd
```
