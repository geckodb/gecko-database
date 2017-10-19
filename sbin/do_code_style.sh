#!/bin/bash

echo "This is the code formatting script of GridStore. Written by Marcus Pinnecke 2017 (pinnecke@ovgu.de)\n
When proceeding, all source and header files in this directory (incl. sub directories) will be re-formatted according the style standards. \n"

read -p "Do you want to proceed [Y/n]?" ans

if [ $ans == "Y" -o $ans == "y" ]
	then

	astyle --style=linux --indent=spaces --attach-extern-c --indent-switches --indent-cases --indent-after-parens --indent-preproc-block --indent-preproc-define --indent-col1-comments --break-blocks=all --pad-oper --pad-comma --delete-empty-lines --align-pointer=name --break-one-line-headers --add-braces --convert-tabs --max-code-length=120 --break-after-logical --recursive "../*.h"

	astyle --style=linux --indent=spaces --attach-extern-c --indent-switches --indent-cases --indent-after-parens --indent-preproc-block --indent-preproc-define --indent-col1-comments --break-blocks=all --pad-oper --pad-comma --delete-empty-lines --align-pointer=name --break-one-line-headers --add-braces --convert-tabs --max-code-length=120 --break-after-logical --recursive "../*.c"
fi