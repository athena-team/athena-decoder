
GCC = g++
INC = -I../
CPPFLAGS =  -g -fPIC $(INC)
#OBJS = kaldi-error.o parse-options.o text-utils.o io-funcs.o kaldi-math.o kaldi-utils.o
OBJS = kaldi-error.o parse-options.o text-utils.o 

all:$(OBJS)

%.o:%.cc
	$(GCC) $(CPPFLAGS) -c $< -o $@
clean:
	rm -rf *.o
