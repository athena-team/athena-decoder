CC = g++ -g -lstdc++ -std=c++11 -ldl

SRCS = service-env.cc service-impl.cc main.cc

OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I. \
			-I../dependencies/ThreadPool/ \
			-I../../cppdecoders/interface/ \
			-I../dependencies/websocketpp/ \
			-I../dependencies/boost_1_72_0/ \
			-I../dependencies/glog/build/

LIB = -L../../cppdecoders/interface -lstreaming-decoder -lpthread \
	  -L../dependencies/glog/build/ -lglog -L../../cppdecoders/lib \
	  -ltensorflow_cc -ltensorflow_framework

TARGET = online-server
all: $(TARGET)

$(TARGET):$(OBJS) 
	$(CC) $(INCLUDES) $^ -o $@ $(LIB)

%.o:%.cc
	$(CC) $(INCLUDES) $(LDFLAGS) -c $< -o $@ $(LIB)

clean:
	rm -rf *.o $(TARGET)


