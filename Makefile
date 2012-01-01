OPT = -O3 -g
INCLUDES = -I../Mint -I../boost_1_44
# Unordered map works with G++, but not G++ 4.0.8 (const find seems to be broken).
#UMAP = -std=c++0x -DUNORDERED_MAP
#UMAP = -DUNORDERED_MAP -DNEED_TR1

CXXFLAGS = $(OPT) -Wall $(INCLUDES) $(UMAP)
#CXXFLAGS = $(OPT) -Wall $(INCLUDES) $(UMAP) -DNCURSES
#CXXFLAGS = $(OPT) -Wall -DUSE_ARGS_SLIST -DNCURSES $(INCLUDES)
#CXXFLAGS = $(OPT) -Wall -DUSE_ARGS_SLIST -DUSE_BUFFER_ROPE -DUSE_MINTSTRING_ROPE -DNCURSES $(INCLUDES)
#CXXFLAGS = $(OPT) -Wall -DUSE_ARGS_SLIST -DUSE_BUFFER_ROPE -DUSE_MINTSTRING_ROPE -DXCURSES $(INCLUDES)

all:	dirs
	$(MAKE) -C boost_1_44 CXXFLAGS="$(CXXFLAGS)"
	$(MAKE) -C Mint CXXFLAGS="$(CXXFLAGS)"
	$(MAKE) -C Emacs CXXFLAGS="$(CXXFLAGS)"

clean:
	rm -rf build


dirs:	build build/objs

build:
	-mkdir build

build/objs:
	-mkdir build/objs