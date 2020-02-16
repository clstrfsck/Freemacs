# For ubuntu, libs required are libboost-regex-dev and libncurses5-dev

OPT = -O3 -g
STD = -std=c++17
INCLUDES = -I../Mint

CXX = g++

CXXFLAGS = $(OPT) $(STD) -Wall -pedantic $(INCLUDES) $(UMAP)
# slist and rope not available on MacOS, but pretty common on Linux
#CXXFLAGS = $(OPT) $(STD) -Wall -pedantic -DUSE_ARGS_SLIST -DUSE_BUFFER_ROPE -DUSE_MINTSTRING_ROPE $(INCLUDES) $(UMAP)
# Other flags that might be useful: -DNCURSES -DXCURSES

all:	dirs
	$(MAKE) -C Mint CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"
	$(MAKE) -C Emacs CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

clean:
	rm -rf build

test:	all
	$(MAKE) -C MintTest CXXFLAGS="$(CXXFLAGS)" test

deps:
	echo "No dependencies to install."

dirs:	build build/objs

build:
	-mkdir build

build/objs:
	-mkdir build/objs
