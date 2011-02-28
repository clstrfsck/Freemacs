all:	../build/libMint.a

OBJ =	../build/objs/bufprim.o \
	../build/objs/emacsbuffer.o \
	../build/objs/emacsbuffers.o \
	../build/objs/frmprim.o \
	../build/objs/libprim.o \
	../build/objs/mint.o \
	../build/objs/mintstring.o \
	../build/objs/mthprim.o \
	../build/objs/strprim.o \
	../build/objs/sysprim.o \
	../build/objs/varprim.o

../build/libMint.a:	$(OBJ)
	ar rcs ../build/libMint.a $(OBJ)

../build/objs/%.o:	%.cpp
	$(CXX) $(CXXFLAGS) -o ../build/objs/$*.o -c $<
