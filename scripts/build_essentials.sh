#!/bin/bash

#Initialize the submodules
git submodule update --init

#Checks out linux submodules
./linux/third_party/connectedhomeip/scripts/checkout_submodules.py --platform linux