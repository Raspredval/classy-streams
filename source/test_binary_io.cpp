#include <ConsoleStreams.hpp>
#include <BufferStreams.hpp>
#include <IOReadWrite.hpp>

int main() {
    auto
        six_seven   = 67;
    io::IOBufferStream
        buff;
    io::BinaryIO(buff)
        .put(six_seven)
        .go_start()
        .get(six_seven);
    io::cout.fmt("the value: {}\n", six_seven);
}