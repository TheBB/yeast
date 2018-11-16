PACKAGE_BASENAME := yeast

include emake.mk

checkout:
	git submodule init
	git submodule update

yeast: checkout
	mkdir build
	cd build && cmake .. && make
