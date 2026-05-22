#!/bin/sh

git add Makefile.inc Makefile_debug.inc Makefile.inc_static *.sln
git add gitadd.sh

if [ x"$1" = "x" ]; then
    echo 'git commit -m "Comment"'
    #echo 'git push origin master'
    echo 'git push origin main'
else
    #git commit -m "$1" && git push origin master
    git commit -m "$1" && git push origin main
fi

