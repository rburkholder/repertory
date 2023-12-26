# repertory

Two libraries:

* MQTT - basic publish/subscribe to one one topic
* Telegram - send message, listen for commands

Installation:

    sudo apt install libpaho-mqtt-dev

    mkdir build
    cd build

    cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D OU_USE_STATIC_LIB=ON \
    -D OU_USE_SHARED_LIB=OFF \
    -D OU_USE_MQTT=ON \
    -D OU_USE_Telegram=OFF \
    ..
    sudo cmake --build . --target=install
