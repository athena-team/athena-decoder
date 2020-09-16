DEP=../dependencies

EVPP=$(DEP)/evpp
LIBEVENT=$(DEP)/libevent
GLOG=$(DEP)/glog
DECODER=../../cppdecoders

INC= -I. \
	-I$(EVPP)/ \
	-I$(LIBEVENT)/include \
	-I$(GLOG)/build \
	-I$(DECODER)/interface 

STATICLIB= -Wl,-Bstatic \
	-L$(EVPP)/build/lib -levpp_static \
	-L$(LIBEVENT)/build/lib -levent \
	-L$(GLOG)/build -lglog \
	-L$(DECODER)/interface -loffline-decoder \
	-Wl,-Bdynamic

DYNAMICLIB= -L$(DECODER)/lib -ltensorflow_cc -ltensorflow_framework -lpthread

GCC=g++
TARGET= offline-server

OBJ= main.o service-impl.o service-env.o

all:$(TARGET)

CXXFLAGS= -g -std=c++11 -Wall -pipe -fpermissive -msse -msse2 -fPIC -Wunreachable-code \
		  -Wno-unused-parameter -Wno-unused-function -Wunused-variable -Wunused-value \
		  -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare -Winvalid-pch -fms-extensions \
		  -Wno-sign-compare -Wno-unused-local-typedefs -Winit-self -Wfloat-equal -Wextra -fno-omit-frame-pointer

$(TARGET): $(OBJ)
	$(GCC) $(CXXFLAGS) -o $@  $^ $(STATICLIB) $(DYNAMICLIB) 

%.o : %.cc
	$(GCC) $(CXXFLAGS) -c $< -o $@ $(INC)

clean:
	rm -rf *.o
	rm -rf $(TARGET)
