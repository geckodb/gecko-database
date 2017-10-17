#!/bin/bash

for file in $(find . -name '*.c'); do 
	mv $file $(dirname $file)/gs_$(basename $file); 
done

for file in $(find . -name '*.h'); do 
	mv $file $(dirname $file)/gs_$(basename $file); 
done
