#include <ConsoleStreams.hpp>
#include <NetworkStreams.hpp>

int main() {
    try {
        io::Local::ONetworkClient
            local_client;
        
        auto
            optConnection   = local_client.Connect(io::Local::Addr{"./local_socket"});
        if (!optConnection)
            throw std::runtime_error("failed to connect to a server");

        auto&
            ostream = *optConnection;
        while (ostream) {
            std::string
                strMessage;
            io::cout.put("enter message: ");
            io::cin.get_line(strMessage);
            io::SerialTextOutput(ostream)
                .put(strMessage)
                .put_endl().flush();
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