#include <asio.hpp>
#include <iostream>
#include <string>

using asio::ip::tcp;

namespace server {
    std::string reply = "Hello, Socketeer world!\n";

    void initialise(uint16_t port) {
        // Let ASIO handle platform-specific setup.
        asio::io_context context;
        asio::error_code ec;

        // Create an endpoint and socket acceptor.
        tcp::endpoint endpoint(tcp::v4(), port);
        tcp::acceptor acceptor(context, endpoint);

        while(true) { // Handle connections.
            tcp::socket socket(context);
            acceptor.accept(socket);
            asio::write(socket, asio::buffer(reply), ec);
            std::cout << "Connection from " << socket.remote_endpoint().address().to_string() << "!\n";
        }
    }
}
