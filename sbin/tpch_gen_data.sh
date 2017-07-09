#!/bin/bash
echo "TPC-H dataset generation script. Copyright (c) 2017 Marcus Pinnecke"
echo ""
echo "This script will create the following directories in the parent directory: bench/olap/tpch/database"
echo ""
read -p "Do you want to clone tpch-dgen from GitHub (https://github.com/electrum/tpch-dbgen.git) and continue [Yn] " -n 1 -r
echo    
if [[ $REPLY =~ ^[Yy]$ ]]
then

	read -p "Set scale factor and hit ENTER [1, 10, 100, 300, 1000, 3000, 10000, 30000, 100000] " 
    
    mkdir ../bench
    mkdir ../bench/olap
    mkdir ../bench/olap/tpch
    mkdir ../bench/olap/tpch/database

    cmake ../.
    make tpch-convert ../.

    
	git clone https://github.com/electrum/tpch-dbgen.git
	cd tpch-dbgen/
	make

	./dbgen -s $REPLY -v -f -T p 
	./dbgen -s $REPLY -v -f -T c 
	./dbgen -s $REPLY -v -f -T s 
	./dbgen -s $REPLY -v -f -T o
	./dbgen -s $REPLY -v -f -T n
	./dbgen -s $REPLY -v -f -T r
	./dbgen -s $REPLY -v -f -T l

	cd ..

	../bin/tpch-convert

	#rm -rf tpch-dbgen/

fi