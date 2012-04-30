
# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

if [ "$XLOGIN_SHELL" = "" ] ; then
    export XLOGIN_SHELL=1
    # User specific aliases and functions

    ORIG_PATH=$PATH
    CFG=linux-x86-debug
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
    set -o vi
else
    [ `id -u` = 0 ] && PS1="# "
fi
