EMAKE_SHA1       ?= 1b23379eb5a9f82d3e2d227d0f217864e40f23e0
PACKAGE_BASENAME := yeast

include emake.mk

checkout:
	git submodule init
	git submodule update --depth 1

yeast: checkout
	mkdir build
	cd build && cmake .. && make
