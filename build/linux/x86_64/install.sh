#!/bin/bash
lines=35
more <<"EOF"
platform will install add /opt/staros.xyz/vos location.
EOF
agreed=
while [ x$agreed = x ]; do
    echo
    echo "Do you agree to install the vos system? [yes or no] "
    read reply leftover
    case $reply in
    y* | Y*)
        agreed=1;;
    n* | N*)
    echo "If you don't agree you can't install this software";
    exit 1;;
    esac
done
if [ -d "/opt/staros.xyz/vos" ] ; then
    rm -rf /opt/staros.xyz/vos/
    echo "build path.."
else
    echo "build path..."
fi
echo "Unpacking..."
tail -n +$lines $0 >/tmp/tmp.tar.gz
tar xzf /tmp/tmp.tar.gz
mkdir -p /opt/staros.xyz/
mkdir -p /opt/staros.xyz/vos
/bin/cp -rf tmp/* /opt/staros.xyz/vos/
rm -rf /tmp/tmp.tar.gz
rm -rf tmp
echo "Successful"
exit 0
