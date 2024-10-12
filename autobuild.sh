sudo apt install mysql-server libmysqlclient-dev -y
cd ./thridparty/json
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
