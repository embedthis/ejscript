#!/bin/sh

clean () {
    echo "Cleaning..."
    echo OUT $OUT
    echo TOP $TOP
    rm -fr $OUT/lib/*
    rm -fr $OUT/bin/*
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
