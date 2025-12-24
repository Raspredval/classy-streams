#include <ConsoleStreams.hpp>
#include <NetworkStreams.hpp>

int main() {
    try {
        io::IPv4::INetworkServer
            tcp_server(io::IPv4::Address{1337});

        io::cout.put("accepting connections\n");
        
        auto
            optConnection   = tcp_server.Accept();
        if (!optConnection)
            throw std::runtime_error("failed to accept a connection from a client");

        io::cout.fmt("accepted a connection from {}\n",
            optConnection->second.ToString());

        auto&
            istream = optConnection->first;
        while (istream) {
            std::string
                strMessage;
            io::SerialTextInput(istream)
                .get_line(strMessage);
            io::cout.fmt("accepted message: \"{}\"\n", strMessage);
            if (strMessage.empty() || strMessage == "/exit")
                return EXIT_SUCCESS;
        }

        throw std::runtime_error("failed to accept a message\n");
    }
    catch (std::exception& err) {
        io::cerr.fmt("error: {}\n", err.what());
        return EXIT_FAILURE;
    }
}