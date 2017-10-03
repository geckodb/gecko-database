#!/bin/bash

sudo apt-get update

echo "installing clang"

sudo apt-get install clang

echo "installing the openssl library"

sudo apt-get install openssl

sudo apt-get install libssl-dev

echo "installing the Apache Portable Runtime(APR) library"

sudo apt-get install apache2-dev libapr1-dev libaprutil1-dev

echo "installing the freebsd library"

sudo apt-get install libbsd-dev

echo "installing curses"

sudo apt-get install libncurses5-dev libncursesw5-dev

echo "installing doxygen"

sudo apt-get install doxygen

echo -e "\n checking the versions of the libraries"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
echo "check for the openssl"
echo "the version we are using is OpenSSL 1.0.2g  1 Mar 2016"
echo "the version you have installed is $(openssl version -v)"
echo -e  "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
echo "check for the apr"
echo "the version we are using is 1.5.2-3"
echo "the version you have installed is $(dpkg -l 'libapr1-dev')"
echo -e  "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
echo "check for the bsd"
echo "the version we are using is 0.8.2-1"
echo "the version you have installed is $(dpkg -l 'libbsd-dev')"
echo -e "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
echo "check for clang"
echo "the version we are using is 3.8.0"
echo "the version you have installed is $(clang --version)"
echo -e  "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
echo "check for the Curses"
echo "the version we are using is 6.0+20160213"
echo "the version you have installed is $(dpkg -l 'libncurses5')"
echo -e "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
echo "check for the Doxygen"
echo "the version we are using is 1.8.11"
echo "the version you have installed is $(doxygen --version)"
echo -e "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
