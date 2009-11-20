/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _ATTR_HH_
#define _ATTR_HH_ 

// rubyish attributes macros...
//

#define attr_reader(_class, _type, _member) \
    const _type & \
    _member() const\
    { return _M_ ## _member; }

#define attr_writer(_class, _type, _member) \
    _class & \
    _member(const _type &value)\
    { _M_ ## _member = value; return *this; }

#define attr_accessor(_class, _type, _member) \
    attr_reader(_class,_type,_member) \
    attr_writer(_class,_type,_member)

#endif /* _ATTR_HH_ */
