#!/bin/bash

set -e

docker build -t rpmbuild .

docker run \
	-v `pwd`/SOURCES:/root/rpmbuild/SOURCES \
	-v `pwd`/RPMS:/root/rpmbuild/RPMS \
        -it rpmbuild \
	./rpmbuild.sh

