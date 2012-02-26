all:	../build/minttest

test:	../build/minttest
	../build/minttest

OBJ =	../build/objs/minttest.o ../build/objs/gtest_main.o ../build/objs/gtest-all.o

../build/minttest:	$(OBJ)
	$(CXX) $(CXXFLAGS) -o ../build/minttest $(OBJ) -L../build -lMint -lboost_regex

../build/objs/%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -o ../build/objs/$*.o -c $<
