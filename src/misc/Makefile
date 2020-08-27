
GCC = g++
INC = -I../
CPPFLAGS =  -g -fPIC $(INC)
OBJS = graph-io.o utils.o

all:$(OBJS)

%.o:%.cc
	$(GCC) $(CPPFLAGS) -c $< -o $@
clean:
	rm -rf *.o
