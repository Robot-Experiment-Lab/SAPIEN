#!/bin/bash
if [ -z ${VERSION} ]
then
    echo VERSION variable is not specified
    VERSION=2.0.0.dev$(date +"%Y%m%d")
    echo VERSION defaults to ${VERSION}
    sleep 3
fi

echo ${VERSION} > python/VERSION
echo __version__=\"${VERSION}\" > python/py_package/version.py

./build.sh
