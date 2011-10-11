#!/bin/sh

clean () {
    echo "Cleaning..."
    echo OUT $OUT
    echo TOP $TOP
    rm -f $OUT/lib/*
    rm -f $OUT/bin/*
}

# MAIN
echo "clean.sh -- Running with ACTION=${ACTION}"

case $ACTION in
    "")
        # build action
        ;;

    "clean")
        clean
        ;;
esac

exit 0
