#!/bin/bash
{ # Try:
	#Build the thing:
	scons DEBUG=Yes &&

	#To avoid a recursion of ffado-mixer files while installing ffado:
	#1: Remove current symlink
	rm /usr/local/lib/python2.7/site-packages/ffado &&
	#2: Install files
	sudo scons install &&
	#3: Remove installed files
	sudo rm -rf /usr/local/lib/python2.7/site-packages/ffado &&
	#4: Link FFADO mixer files to development
	ln -s /home/camilo/Escritorio/Repos/libffado/support/mixer-qt4/ffado /usr/local/lib/python2.7/site-packages/ffado &&
	sudo ldconfig
} || { # Catch:
	echo "Error!"
	return 1
}
return 0

