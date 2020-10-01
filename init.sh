#!/bin/bash

set -eu

# debug log function
debug() {
    [ $VERB -eq 1 ] || return
    echo $1
}

# usage function
usage() {
    echo $0 [options]
    echo
    echo Options:
    echo -h show the usage for $0
    echo -c \<arg\> runs a custom command inside the namespace
    echo -n creates the network namespace
    echo -v activates the verbose mode
}

# Constants and global variables
# Unshare
VERB=0
UNSH_ARGS="rufpi"
UNSH_COMPLEX_ARGS="--mount-proc"
EXEC="/bin/sh"
# Filesystem
ROOT=`mktemp -d`
ROOT_FS="$PWD/rootfs/alpine-3.12.0-x86_64.tar.gz"

# Process arguments
OPTS=":nphvc:"

while getopts "$OPTS" opt; do
    case $opt in
        # Help
        h)
            usage
            exit 1
            ;;
        # Custom command to execute
        c)
            debug "Using custom command: $OPTARG"
            EXEC=$OPTARG
            ;;
        # Network namespace
        n)
            debug "Network namespace activated"
            UNSH_ARGS+="n"
            ;;
        # Verbose mode for debugging
        v)
            VERB=1
            debug "Running in verbose mode"
            ;;
        \?)
            echo "Invalid option -$OPTARG" >&2
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument" >&2
            exit 1
            ;;
    esac
done

# Prepare new rootfs
debug "Extracting $ROOT_FS to the directory $ROOT"
tar -xf $ROOT_FS -C $ROOT

# Run unshare command
debug "Running unshare with -$UNSH_ARGS $UNSH_COMPLEX_ARGS"
debug "and root at $ROOT"
unshare -$UNSH_ARGS -R $ROOT $UNSH_COMPLEX_ARGS "$EXEC"

# Clean the filesystem
debug "Removing $ROOT"
rm -rf $ROOT
