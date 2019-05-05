#!/bin/bash
# enable inspircd with default config before testing it
sed --in-place "s/INSPIRCD_ENABLED=./INSPIRCD_ENABLED=1/" /etc/default/inspircd
service inspircd start
