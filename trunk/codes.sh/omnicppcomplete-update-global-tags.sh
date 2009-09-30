#!/bin/bash 
# $Id$/
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <bonelli@antifork.org> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
# ----------------------------------------------------------------------------

global_tags=~/.vim/global-tags

magic_header=/usr/include/c++/4.3/i486-linux-gnu/bits/c++config.h
magic_path=/usr/include/c++/4.3/i486-linux-gnu/
base_path=/usr/include/c++/4.3

exuberant_ctags() 
{
        /usr/bin/ctags --append=yes --sort=yes --languages=C,C++ --c++-kinds=+p --fields=+iaS --extra=+q  -I "__THROW,__NTH,__wur,__warnattr,__attribute__+, __nonnull,__attribute_malloc__, __attribute_pure__,__attribute_used__, __attribute_noinline__,__attribute_deprecated__,  __attribute_format_arg__, __attribute_format_strfmon__,__attribute_warn_unused_result__, __always_inline, __extern_inline,__extension__,__restrict" -f $global_tags $1

}

if [ $# -eq 0 ]
then
        echo "usage: $0 /usr/include/g++/4.3/*/*.h ..."
        exit 0
fi

for f in $* 
do
        # output..
        echo "exuberant_ctags: parsing $f..."

        tmp_header=/tmp/`basename $f`

        # preproces macros...
        cat $f | g++ -E - -I$base_path -I$magic_path -include $magic_header >  $tmp_header 
      
        # ctags this header...
        exuberant_ctags $tmp_header 

        # rm temporary header...
        rm $tmp_header
done
