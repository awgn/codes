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

    log->loglevel(more::logstreambuf::ERR);

    std::clog.rdbuf(log);

    // default_priority() set the default priority for the stream
    //

    more::default_priority(std::clog, more::logstreambuf::ERR);

    // more::priority() set a temporary priority for the current ostream until a std::endl is streamed 
    //

    more::priority(std::clog, more::logstreambuf::EMERG)   << __LOG_FUNCTION__ << ": LOG_EMERG  : hello world!" << std::endl; 
    more::priority(std::clog, more::logstreambuf::ALERT)   << __LOG_FUNCTION__ << ": LOG_ALERT  : hello world!" << std::endl;
    more::priority(std::clog, more::logstreambuf::CRIT )   << __LOG_FUNCTION__ << ": LOG_CRIT   : hello world!" << std::endl;
    more::priority(std::clog, more::logstreambuf::ERR  )   << __LOG_FUNCTION__ << ": LOG_ERR    : hello world!" << std::endl;
    more::priority(std::clog, more::logstreambuf::WARNING) << __LOG_FUNCTION__ << ": LOG_WARNING: hello world!" << std::endl;
    more::priority(std::clog, more::logstreambuf::NOTICE)  << __LOG_FUNCTION__ << ": LOG_NOTICE : hello world!" << std::endl;
    more::priority(std::clog, more::logstreambuf::INFO)    << __LOG_FUNCTION__ << ": LOG_INFO   : hello world!" << std::endl;
    more::priority(std::clog, more::logstreambuf::DEBUG)   << __LOG_FUNCTION__ << ": LOG_DEBUG  : hello world!" << std::endl;

    std::clog << __LOG_FUNCTION__ << ": default priority (ERR)" << std::endl;

    more::default_priority(std::clog, more::logstreambuf::DEBUG);

    std::clog << __LOG_FUNCTION__ << ": default level (DEBUG) " << std::endl;    // with log->loglevel() set to ERR this message won't be displayed

    return 0;
}
