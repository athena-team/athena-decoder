CC = g++ -g -lstdc++ -std=c++11 -ldl

SRCS = ctc.cc inference.cc \
	   ../feat/CFeatureClass.o ../feat/CFrontend.o ../feat/FeatureExtraction.o \
	   ../tfutils/tfmodel.o
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I. \
			-I../tfutils \
			-I../feat 

DYNAMIC_LIB = libinference-ctc.so
STATIC_LIB = libinference-ctc.a
all:$(DYNAMIC_LIB) $(STATIC_LIB)

$(DYNAMIC_LIB):$(OBJS) 
	$(CC) -shared -fPIC $^ -o $@

$(STATIC_LIB):$(OBJS)
	$(BUILD_LIBRARY)

%.o:%.cc
	$(CC) $(INCLUDES) $(EXTRA_CXXFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.so *.a

include ../path.mk ../lib.mk


