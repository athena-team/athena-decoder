CC = g++ -lstdc++ -std=c++11 -O2 -ldl
SRCS = tfmodel.cc 
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I./tfutils \
			-I../feat 
all:$(OBJS)

%.o:%.cc
	$(CC) $(INCLUDES) $(EXTRA_CXXFLAGS)  $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o 

include ../path.mk
