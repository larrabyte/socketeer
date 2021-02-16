#include <asio.hpp>
#include <iostream>

using asio::ip::tcp;

namespace client {
    constexpr size_t bufsize = 4096;

    void initialise(std::string hostname, std::string port) {
        // Let ASIO handle platform-specific setup.
        asio::mutable_buffer buffer = asio::buffer(malloc(bufsize), bufsize);
        asio::io_context context;
        asio::error_code ec;

        // Get a list of endpoints corresponding to the hostname/port combo.
        tcp::resolver resolver(context);
        auto endpoints = resolver.resolve(hostname, port);

        // Attempt a connection using the endpoints gathered from before.
        tcp::socket socket(context);
        asio::connect(socket, endpoints);
        while(ec != asio::error::eof) {
            socket.read_some(buffer, ec);
        }

        // Print the data out and exit.
        std::cout << "Connection closed. Data read:\n" << static_cast<char*>(buffer.data());
    }
}
