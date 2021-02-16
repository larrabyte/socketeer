#include <client.hpp>
#include <server.hpp>
#include <iostream>
#include <cstring>

void woops(std::string executable) {
    std::cout << "Invalid argument form. Valid forms are:\n";
    std::cout << "    " << executable << " client <hostname> <port>\n";
    std::cout << "    " << executable << " server <port>\n";
}

int main(int argc, char **argv) {
    // Codepath for the client. Checks argc before calling client::initialise().
    if(argc == 4 && strcmp(argv[1], "client") == 0) {
        client::initialise(argv[2], argv[3]);
    }

    // Codepath for the server. Checks argc before calling server::initialise().
    else if(argc == 3 && strcmp(argv[1], "server") == 0) {
        uint16_t port = atoi(argv[2]);
        server::initialise(port);
    }

    else { // Woops.
        woops(argv[0]);
        return 69;
    }
}
