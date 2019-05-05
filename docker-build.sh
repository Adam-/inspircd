#!/bin/bash

set -e

docker build -t debbuild .

docker run \
        -v `pwd`:/home/adam/inspircd \
	-v `pwd`/debbuild:/home/adam \
        -u `id -u`:`id -g` \
        -it debbuild \
	debuild -b -uc -us

