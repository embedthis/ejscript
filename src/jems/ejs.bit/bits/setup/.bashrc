CFG=linux-x86-debug

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

if [ `id -u` != 0 ] ; then
    export PATH="/home/mob/ejs/${CFG}/bin:$PATH:."
    work() {
        local d=~/$1
        cd $d
        export CDPATH=".:~:$d:$d/src:$d/src/jems:$CDPATH"
        export PATH="$d/${CFG}/bin:$PATH"
    }
    bare() {
        export PATH=".:/usr/local/bin:/usr/bin:/bin:/usr/X11R6/bin:%BTILDIR%/Host/bin:/WINDOWS/system32:/WINDOWS"
    }
    work appweb
    work ejs
    PS1='`pwd`> '
else
    PS1="# "
fi
set -o vi
