mkdir build
cd build

cmake ..

windows:
cmake --build .

unix:
make

g++ main.cpp ./networking/ProxyServer.cpp -o main -I ./asio-1.30.2/include/ -I ./networking/ -D_WIN32_WINNT=0x0A00 -lwsock32 -lws2_32

g++ main.cpp ./networking/ProxyServer.cpp -o main -I ./asio-1.30.2/include/ -I ./networking/
