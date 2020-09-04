
GCC = g++
INC = -I../
CPPFLAGS = -std=c++11 -g -fPIC $(INC)

OBJS = faster-decoder.o transformer-faster-decoder.o transformer-beam-search-decoder.o transformer-argmax-decoder.o
all:$(OBJS)

%.o:%.cc
	$(GCC) $(CPPFLAGS) -c $< -o $@
clean:
	rm -rf *.o
