all:	../build/libboost_regex.a

OBJ =	../build/objs/regex.o \
	../build/objs/instances.o \
	../build/objs/static_mutex.o \
	../build/objs/regex_raw_buffer.o \
	../build/objs/regex_traits_defaults.o \
	../build/objs/cpp_regex_traits.o

../build/libboost_regex.a:	$(OBJ)
	ar rcs ../build/libboost_regex.a $(OBJ)

../build/objs/%.o:	libs/regex/src/%.cpp
	$(CXX) $(CXXFLAGS) -o ../build/objs/$*.o -c $<
