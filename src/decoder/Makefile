
GCC = g++
INC = -I../
CPPFLAGS = -std=c++11 -g -fPIC $(INC)

OBJS = faster-decoder.o 
all:$(OBJS)

%.o:%.cc
	$(GCC) $(CPPFLAGS) -c $< -o $@
clean:
	rm -rf *.o
