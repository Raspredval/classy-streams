#include <ConsoleStreams.hpp>
#include "include/NetworkStreams.hpp"

int main() {
    try {
        io::IPv4::ONetworkClient
            ip_client;
        
        auto
            optConnection   = ip_client.Connect({"127.0.0.1", "1337"});
        if (!optConnection)
            throw std::runtime_error("failed to connect to a server");

        while (optConnection->Good()) {
            std::string
                strMessage;
            io::cout.put("enter message: ");
            io::cin.get_line(strMessage);
            io::SerialTextOutput(*optConnection)
                .put(strMessage);
            if (strMessage == "/exit")
                return EXIT_SUCCESS;
        }

        throw std::runtime_error("failed to send a message\n");
    }
    catch (std::exception& err) {
        io::cerr.fmt("error: {}\n", err.what());
        return EXIT_FAILURE;
    }
}