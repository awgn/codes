#!/bin/bash 
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <bonelli@antifork.org> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
# ----------------------------------------------------------------------------


arch=i486-linux-gnu
#arch=x86_64-linux-gnu

arch_header=/usr/include/c++/4.4/$arch/bits/c++config.h
arch_path=/usr/include/c++/4.4/$arch/
base_path=/usr/include/c++/4.4

if [ $# -eq 0 ]
then
        echo "usage: $0 tag-name"
        exit 0
fi

tag_name=$1

shift

for f in /usr/include/c++/4.4/bits/*.h /usr/include/c++/4.4/tr1/*.h 
do
        # output..
        # echo "exuberant_ctags: parsing $f..."

        tmp_header=/tmp/`basename $f`

        # preproces macros...
        cat $f | g++ -E - -I$base_path -I$arch_path -D__cplusplus=1 -include $arch_header >  $tmp_header

        # ctags this header...
        cpp_ctags.sh -f $tag_name $tmp_header

        # rm temporary header...
        rm $tmp_header
done
