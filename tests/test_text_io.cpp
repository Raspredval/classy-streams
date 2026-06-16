#include <ConsoleStreams.hpp>
#include <BufferStreams.hpp>
#include <IOReadWrite.hpp>

int main() {
    io::IOBufferStream
        buff;
    io::TextIO
        buffio  = buff;
    buffio
        .put(69).putnl()
        .put(-6.7).putnl()
        .put(1337.e-2).putnl()
        .go_start();
    io::cout
        .importi(buff).putnl()
        .importf(buff).putnl()
        .importf(buff).putnl()
        .flush();
}