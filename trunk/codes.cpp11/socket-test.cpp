/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <socket.hpp>
#include <cstdlib>
#include <memory>

void client(std::string host, unsigned short port)
{
    // simple echo server
    std::cout << "connecting to " << host << ':' << port << std::endl;

    more::sockaddress<AF_INET> addr(host, port);

    more::socket<AF_INET> sock(SOCK_STREAM);

    sock.connect(addr);

    const char hello_world[] = "hello world!";

    auto len = htonl(sizeof(hello_world));  // this include '\0'

    sock.send_atomic(more::const_buffer(reinterpret_cast<const char *>(&len), sizeof(len)), 0);
    sock.send_atomic(more::const_buffer(hello_world, sizeof(hello_world)), 0);

    /// recv echo

    sock.recv_atomic(more::mutable_buffer(reinterpret_cast<char *>(&len), sizeof(len)), 0);

    auto size = ntohl(len);

    std::unique_ptr<char[]> buffer(new char[size]);

    sock.recv_atomic(more::mutable_buffer(buffer.get(), size), 0);

    std::cout << buffer.get() << std::endl;
}


void server(std::string host, unsigned short port)
{
    // simple echo server
    std::cout << "simple echo-server on " << host << ':' << port << std::endl;

    more::sockaddress<AF_INET> addr(host, port);
    more::sockaddress<AF_INET> peer;

    more::socket<AF_INET> local(SOCK_STREAM);

    local.bind(addr);
    local.listen(1);

    for(;;)
    {
        try
        {
            std::cout << "waiting for a client... ";
            std::cout.flush();

            more::socket<AF_INET> remote(SOCK_STREAM);

            local.accept(peer, remote);

            std::cout << "[" << peer.host() << ":" << peer.port() << "]" << std::endl;

            unsigned int s;

            remote.recv_atomic(more::mutable_buffer(reinterpret_cast<char *>(&s), sizeof(int)), 0);

            auto size = ntohl(s);

            std::unique_ptr<char[]> buffer(new char[size]);

            remote.recv_atomic(more::mutable_buffer(reinterpret_cast<char *>(buffer.get()), size), 0);

            /// send echo

            remote.send_atomic(more::const_buffer(reinterpret_cast<const char *>(&s), sizeof(int)), 0);
            remote.send_atomic(more::const_buffer(buffer.get(), size), 0);

        }
        catch(std::exception &e)
        {
            std::cerr << "error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, const char *argv[])
try
{
    if (argc < 4)
        throw std::runtime_error("socket-test [client|server] host port");

    if (strcmp(argv[1],"client") == 0)
        client(argv[2], static_cast<unsigned short>(atoi(argv[3])));
    else
    if (strcmp(argv[1], "server") == 0)
        server(argv[2], static_cast<unsigned short>(atoi(argv[3])));
    else
        throw std::runtime_error("error: unknown mode " + std::string(argv[1]));

}
catch(std::exception &e)
{
    std::cerr << e.what() << std::endl;
}
