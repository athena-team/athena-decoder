CC = g++ -lstdc++ -std=c++11 -O2 -ldl
SRCS = encoder.cc decoder.cc inference.cc \
	   ../feat/CFeatureClass.o ../feat/CFrontend.o ../feat/FeatureExtraction.o \
	   ../tfutils/tfmodel.o
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I. \
			-I../tfutils \
			-I../feat 

TARGET = libinference-transformer.so
all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -shared -fPIC $^ -o $@

%.o:%.cc
	$(CC) $(INCLUDES) $(EXTRA_CXXFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.so

include ../path.mk
