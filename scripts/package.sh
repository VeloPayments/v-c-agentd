#!/bin/sh

# The package script assumes it is run from the build directory.

#Sanity check: is the version set?
if [ -z "$1" ]; then
    echo "First argument must be set as version."
    exit 1
fi

#Sanity check: does the conf file exist?
if [ ! -f $2 ]; then
    echo "Second argument must be the conf file."
    exit 1
fi

#Sanity check: does agentd exist?
if [ ! -x $3 ]; then
    echo "agentd NOT found!"
    exit 1
fi

PACKAGE_DIR=agentd-${1}
PACKAGE_NAME=agentd-${1}.tar.xz
CONF_FILE=$2

#get the TAR binary
TAR=`which gtar 2>/dev/null`
if [ "$TAR" == "" ]; then
TAR=`which tar 2>/dev/null`
fi

#verify that this is GNU Tar
if ($TAR --version | grep -q GNU); then
    : #do nothing
else
echo "GNU tar required."
exit 1
fi

#delete the old package directory
rm -rf $PACKAGE_DIR

#create the package directory
mkdir -p $PACKAGE_DIR/bin $PACKAGE_DIR/lib $PACKAGE_DIR/etc $PACKAGE_DIR/data

#install agentd to package dir
install $3 $PACKAGE_DIR/bin

#add any dynamic libs required
ldd $3 | grep -E "[.]so" | grep -v ld.so | grep -v vdso.so \
    | grep -v ld-linux-x86-64.so.2 \
    | sed 's/(.*)//; s/\(.*\)=>/\1/' \
    | awk '{ print $NF }' \
    | xargs -I instfile install instfile $PACKAGE_DIR/lib

#pick up ld.so
if [ -f /usr/libexec/ld.so ]; then
    mkdir -p $PACKAGE_DIR/usr/libexec
    install /usr/libexec/ld.so $PACKAGE_DIR/usr/libexec
fi

#pick up ld for x86_64.
if [ -f /lib64/ld-linux-x86-64.so.2 ]; then
    mkdir -p $PACKAGE_DIR/lib64
    install /lib64/ld-linux-x86-64.so.2 $PACKAGE_DIR/lib64
fi

#strip binaries
find $PACKAGE_DIR -type f -name "*.so.*" -exec strip {} \;
strip $PACKAGE_DIR/bin/agentd

#build the pid dir.
mkdir -p $PACKAGE_DIR/var/pid

#install the default config file
install -m 400 $CONF_FILE $PACKAGE_DIR/etc/agentd.conf

#build the package
$TAR --owner=veloagent --group=veloagent -cf - $PACKAGE_DIR | xz -c > $PACKAGE_NAME

#clean up
rm -rf $PACKAGE_DIR
