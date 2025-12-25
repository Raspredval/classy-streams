#include <ConsoleStreams.hpp>
#include <BufferStreams.hpp>
#include <IOReadWrite.hpp>

int main() {
    auto
        six_seven   = 67;
    
    io::IOBufferStream
        buff;
    io::BinaryOutput(buff)
        .put(six_seven);

    six_seven       = 420;

    io::BinaryInput(buff)
        .go_start()
        .get(six_seven);
    io::cout.fmt("the value: {}\n", six_seven);
}