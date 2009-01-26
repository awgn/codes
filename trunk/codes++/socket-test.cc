/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <socket.hh>

char buffer[80];

int main()
{
    // simple echo server
    //

    more::sockaddress<AF_INET> addr("127.0.0.1",31337);
    more::sockaddress<AF_INET> peer;
    more::socket<AF_INET> l(SOCK_STREAM);

    l.bind(addr);
    l.listen(1);

    for(;;) {
        std::cout << "waiting for a client... ";
        std::cout.flush();
        more::socket<AF_INET> r(SOCK_STREAM);
        l.accept(peer, r);
        std::cout << "[" << peer.host() << ":" << peer.port() << "]" << std::endl; 
        int n = r.recv(buffer, sizeof(buffer), 0);
        
        // r.send(buffer, n, 0); 
        // send double-echo by means of iovec...

        std::tr1::array<iovec,2> iov;

        iov[0].iov_base = buffer;
        iov[0].iov_len  = n;

        iov[1].iov_base = buffer;
        iov[1].iov_len  = n;

        r.send(iov,0);
    }
}
