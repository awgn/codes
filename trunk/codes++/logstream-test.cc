/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <logstream.hh>

int
main(int argc, char *argv[])
{
    more::logstreambuf * log = new more::logstreambuf(std::cout.rdbuf());

    log->priority(more::logstreambuf::ALERT);

    std::clog.rdbuf(log);

    // logprio_default() set the default priority for the current ostream
    //

    more::loglevel_default(std::clog, more::logstreambuf::ALERT);

    // more::logprio() set a temporary priority for the current ostream until a std::endl is streamed 
    //

    more::loglevel(std::clog, more::logstreambuf::EMERG)   << __LOG_FUNCTION__ << ": LOG_EMERG  : hello world!" << std::endl; 
    more::loglevel(std::clog, more::logstreambuf::ALERT)   << __LOG_FUNCTION__ << ": LOG_ALERT  : hello world!" << std::endl;
    more::loglevel(std::clog, more::logstreambuf::CRIT )   << __LOG_FUNCTION__ << ": LOG_CRIT   : hello world!" << std::endl;
    more::loglevel(std::clog, more::logstreambuf::ERR  )   << __LOG_FUNCTION__ << ": LOG_ERR    : hello world!" << std::endl;
    more::loglevel(std::clog, more::logstreambuf::WARNING) << __LOG_FUNCTION__ << ": LOG_WARNING: hello world!" << std::endl;
    more::loglevel(std::clog, more::logstreambuf::NOTICE)  << __LOG_FUNCTION__ << ": LOG_NOTICE : hello world!" << std::endl;
    more::loglevel(std::clog, more::logstreambuf::INFO)    << __LOG_FUNCTION__ << ": LOG_INFO   : hello world!" << std::endl;
    more::loglevel(std::clog, more::logstreambuf::DEBUG)   << __LOG_FUNCTION__ << ": LOG_DEBUG  : hello world!" << std::endl;

    std::clog << __LOG_FUNCTION__ << ": default level (ALERT)" << std::endl;

    more::loglevel_default(std::clog, more::logstreambuf::CRIT);

    std::clog << __LOG_FUNCTION__ << ": default level (CRIT) " << std::endl;    // with log->priority() set to ALERT this message won't be displayed

    return 0;
}
