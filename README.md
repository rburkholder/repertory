# repertory

Two libraries:

* MQTT - basic publish/subscribe to one one topic
* Telegram - send message, listen for commands

Installation:

    git clone https://github.com/rburkholder/libs-build.git
    cd libs-build

    ./build.sh base
    ./build.sh zlib
    ./build.sh boost

    cd ..

    sudo apt install libssl-dev
    sudo apt install libpaho-mqtt-dev

    git clone https://github.com/rburkholder/repertory
    cd repertory

    mkdir build
    cd build

    cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D OU_USE_STATIC_LIB=ON \
    -D OU_USE_SHARED_LIB=OFF \
    -D OU_USE_MQTT=ON \
    -D OU_USE_Telegram=ON \
    ..
    sudo cmake --build . --target=install
