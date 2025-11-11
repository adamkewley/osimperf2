#!/usr/bin/env bash

apt_dependencies=(
    git
    freeglut3-dev
    libxi-dev
    libxmu-dev
    liblapack-dev
    wget
    build-essential
    cmake
    python3
)
sudo apt-get install -f ${apt_dependencies[@]}

