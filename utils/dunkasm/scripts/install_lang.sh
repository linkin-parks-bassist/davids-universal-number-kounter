#!/bin/bash -x

for path in /usr/share/gtksourceview-*/language-specs
do
	if [ -d $path ]; then
		cp dunkasm.lang $path
	fi
done
