#!/bin/bash

set -e

chown root.root *
spectool -g -R inspircd.spec
rpmbuild -ba inspircd.spec
