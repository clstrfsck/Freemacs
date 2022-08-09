# For ubuntu, libs required are libboost-regex-dev and libncurses5-dev

OPT = -O3 -g
STD = -std=c++17
MINCLUDES = -IMint
EINCLUDES = -IMint -IEmacs

CXX = g++

GTST	= -DGTEST_HAS_PTHREAD=0 -Igoogletest/googletest/include
GTGTST	= -Igoogletest/googletest

#PROF =	-fprofile-arcs -ftest-coverage

CXXFLAGS = $(OPT) $(STD) -Wall -Wextra -pedantic -D_DEBUG -D_VERBOSE_DEBUG -D_EXEC_DEBUG
# Other flags that might be useful: -DNCURSES -DXCURSES

# MacOS High Sierra with Homebrew boost and clang needs libboost_regex-mt
BOOST_REGEX=boost_regex
#BOOST_REGEX=boost_regex-mt

all:	dirs build/freemacs

MOBJ =	build/objs/bufprim.o \
	build/objs/emacsbuffer.o \
	build/objs/emacsbuffers.o \
	build/objs/frmprim.o \
	build/objs/libprim.o \
	build/objs/mint.o \
	build/objs/mintstring.o \
	build/objs/mthprim.o \
	build/objs/strprim.o \
	build/objs/sysprim.o \
	build/objs/varprim.o

EOBJ =	build/objs/emacs.o \
	build/objs/emwindow-curses.o \
	build/objs/winprim.o

TOBJ =	build/objs/minttest.o \
	build/objs/gapbuffertest.o \
	build/objs/gtest-all.o \
	build/objs/gtest_main.o

build/minttest:		$(TOBJ) build/libMint.a
	$(CXX) $(CXXFLAGS) $(PROF) -o build/minttest $(TOBJ) -Lbuild -lMint -lboost_regex

build/freemacs:		$(EOBJ) build/libMint.a
	$(CXX) $(CXXFLAGS) $(PROF) -o build/freemacs $(EOBJ) -Lbuild -lMint -l$(BOOST_REGEX) -lncurses

build/libMint.a:	$(MOBJ)
	ar rcs build/libMint.a $(MOBJ)

build/objs/%.o:		Emacs/%.cpp
	$(CXX) $(CXXFLAGS) $(EINCLUDES) $(PROF) -o build/objs/$*.o -c $<

build/objs/%.o:		Mint/%.cpp
	$(CXX) $(CXXFLAGS) $(MINCLUDES) $(PROF) -o build/objs/$*.o -c $<

build/objs/%.o:	MintTest/%.cpp
	$(CXX) $(CXXFLAGS) $(EINCLUDES) $(GTST) $(PROF) -o build/objs/$*.o -c $<

build/objs/%.o: googletest/googletest/src/%.cc
	$(CXX) $(CXXFLAGS) $(EINCLUDES) $(GTST) $(GTGTST) $(PROF) -o build/objs/$*.o -c $<

clean:
	rm -rf build
	rm -f *.gcov

test:	dirs build/minttest
	build/minttest
	find build -iname \*.gcno -exec gcov -a -b -c -f -u -p -l "{}" \;

deps:
	echo "No dependencies to install."

dirs:	build build/objs

build:
	-mkdir build

build/objs:
	-mkdir build/objs
