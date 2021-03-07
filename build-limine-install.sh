#!/usr/bin/env bash

git clone https://github.com/limine-bootloader/limine.git --branch=v1.0.8
cd limine && make && sudo make install
