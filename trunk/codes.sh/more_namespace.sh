#!/bin/bash 
# $Id$/
# 
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <bonelli@antifork.org> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
# ----------------------------------------------------------------------------

MORE_BASEDIR=/opt/nicola-bonelli-repo/codes++

usage()
{
        echo "$0 more::namespace {status|update}"
}

version()
{
        if [ -f $1 ];
        then
                ret=$(cat $1 | grep "\$Id.*\$" | awk '{print $4}')
        else
                ret=-1
        fi
        return $ret 
}

diff()
{
        bdir=$1
        fname=$2

        version $fname 
        ver=$?

        version $bdir/$fname
        base_ver=$?

        if [ $ver -eq $base_ver ];
        then
                /usr/bin/diff $bdir/$fname $fname >& /dev/null
                return $?        
        fi

        url=`svn info $bdir/$fname | grep URL | awk '{print $2}'`
        cd /tmp
        svn export -r $ver $url >& /dev/null
        cd - >& /dev/null

        /usr/bin/diff /tmp/$fname $fname >& /dev/null
        return $?
}

update() 
{
        bdir=$1
        echo "Updating from $bdir:"
        for i in *.hh
        do                
                version $i
                ver=$?
                version $bdir/$i
                base_ver=$?

                diff $bdir $i
                case "$?" in
                        0)
                        if [ $base_ver -gt $ver ];
                        then 
                                echo "U $i"
                                cp -f $bdir/$i $i > /dev/null
                        fi
                        ;;
                        1)
                        echo "M $i (update not possible)"
                        ;;
                esac
        done
        echo "done."
}

status_()
{
        bdir=$1
        for i in *.hh
        do
                version $i
                ver=$?
                version $bdir/$i
                base_ver=$?
                
                diff $bdir $i
                case "$?" in
                        0)

                        if [ $ver -eq $base_ver ];
                        then
                                echo ". $i" 
                        else
                                echo "U $i"
                        fi
                        ;;
                        1)
                        echo "M $i"
                        ;;
                        *)
                        echo "? $i"
                        ;;
                esac
        done
}


if [ -z "$1" ];
then
        usage
        exit 1
fi


case "$1" in
        status)
        status_ $MORE_BASEDIR
        ;;
        update)
        update $MORE_BASEDIR
        ;;
        *)
        usage
        exit 1
        ;;
esac