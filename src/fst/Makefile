
GCC = g++
INC = -I../
CPPFLAGS =  -g -fPIC $(INC)
OBJS = properties.o

all:$(OBJS)

%.o:%.cc
	$(GCC) $(CPPFLAGS) -c $< -o $@
clean:
	rm -rf *.o
