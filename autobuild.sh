cd ./thridparty/json
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
