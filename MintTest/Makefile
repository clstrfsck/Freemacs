all:	../build/minttest

test:	../build/minttest
	../build/minttest

OBJ =	../build/objs/minttest.o ../build/objs/gtest_main.o ../build/objs/gtest-all.o

../build/minttest:	$(OBJ)
	$(CXX) $(CXXFLAGS) -DGTEST_HAS_PTHREAD=0 -o ../build/minttest $(OBJ) -L../build -lMint -lboost_regex

../build/objs/%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -DGTEST_HAS_PTHREAD=0 -o ../build/objs/$*.o -c $<
