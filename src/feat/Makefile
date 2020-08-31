CC = g++ -lstdc++ -std=c++11 -O2 -ldl
SRCS = CFeatureClass.cc CFrontend.cc FeatureExtraction.cc
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I. 

all:$(OBJS)


%.o:%.cc
	$(CC) $(INCLUDES) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o
