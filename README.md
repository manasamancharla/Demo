mkdir build
cd build

cmake ..

windows:
cmake --build .

unix:
make

g++ main.cpp ./networking/ProxyServer.cpp -o main -I ./asio-1.30.2/include/ -D_WIN32_WINNT=0x0A00 -lwsock32 -lws2_32
