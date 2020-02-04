# MacOS High Sierra with Homebrew boost and clang needs libboost_regex-mt
BOOST_REGEX=boost_regex
#BOOST_REGEX=boost_regex-mt

all:	../build/freemacs

OBJ =	../build/objs/emacs.o \
	../build/objs/emwindow-curses.o \
	../build/objs/winprim.o


../build/freemacs:	$(OBJ)
	$(CXX) $(CXXFLAGS) -o ../build/freemacs $(OBJ) -L../build -lMint -l$(BOOST_REGEX) -lncurses

../build/objs/%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -o ../build/objs/$*.o -c $<
