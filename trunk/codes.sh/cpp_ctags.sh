#!/bin/bash 
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <bonelli@antifork.org> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
# ----------------------------------------------------------------------------

cpp_ctags() 
{
  /usr/bin/ctags --append=yes --sort=yes --languages=C,C++ --c++-kinds=+p --fields=+iaS --extra=+q  -I "__THROW,__NTH,__wur,__warnattr,__attribute__+, __nonnull,__attribute_malloc__, __attribute_pure__,__attribute_used__, __attribute_noinline__,__attribute_deprecated__,  __attribute_format_arg__, __attribute_format_strfmon__,__attribute_warn_unused_result__, __always_inline, __extern_inline,__extension__,__restrict" $*
}

if [ $# -eq 0 ]
then
        echo "usage: $0 [options] [file(s)]"
        exit 0
fi

cpp_ctags $*

