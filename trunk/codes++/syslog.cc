/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <syslog.hh>

namespace sys
{
    std::ostream log ( new sys::syslog(LOG_NDELAY) );                     
    std::ostream plog( new sys::syslog(LOG_NDELAY|LOG_PID) );        
    std::ostream err ( new sys::syslog(LOG_NDELAY|LOG_PERROR) );          
    std::ostream perr( new sys::syslog(LOG_NDELAY|LOG_PERROR|LOG_PID) ); 
}

