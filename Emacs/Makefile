all:	../build/freemacs

OBJ =	../build/objs/emacs.o \
	../build/objs/emwindow-curses.o \
	../build/objs/winprim.o


../build/freemacs:	$(OBJ)
	$(CXX) $(CXXFLAGS) -o ../build/freemacs $(OBJ) -L../build -lMint -lboost_regex -lncurses

../build/objs/%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -o ../build/objs/$*.o -c $<
