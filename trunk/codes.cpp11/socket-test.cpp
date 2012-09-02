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

char buf[80];

int main()
{
    // simple echo server
    //

    more::sockaddress<AF_INET> addr("127.0.0.1",31337);
    more::sockaddress<AF_INET> peer;
    more::socket<AF_INET> tmp(SOCK_STREAM);

    more::socket<AF_INET> local(std::move(tmp));

    assert(tmp.fd() == -1);

    local.bind(addr);
    local.listen(1);

    std::cout << "Simple double-echo server on 127.0.0.1:31337..." << std::endl << std::endl;

    for(;;) {
        std::cout << "waiting for a client... ";
        std::cout.flush();

        more::socket<AF_INET> remote(SOCK_STREAM);

        local.accept(peer, remote);
        
        std::cout << "[" << peer.host() << ":" << peer.port() << "]" << std::endl; 
        auto n = remote.recv(more::mutable_buffer(buf), 0);
        
        // r.send(buf, 0); 
        // send double-echo by means of iovec...

        std::array<iovec,2> iov;

        iov[0].iov_base = buf;
        iov[0].iov_len  = static_cast<size_t>(n);

        iov[1].iov_base = buf;
        iov[1].iov_len  = static_cast<size_t>(n);

        remote.send(iov,0);
    }
}
