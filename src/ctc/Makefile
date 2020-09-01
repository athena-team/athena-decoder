CC = g++ -lstdc++ -std=c++11 -O2 -ldl

SRCS = ctc.cc inference.cc \
	   ../feat/CFeatureClass.o ../feat/CFrontend.o ../feat/FeatureExtraction.o \
	   ../tfutils/tfmodel.o
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I. \
			-I../tfutils \
			-I../feat \
			-I$(TF_DIR) \
			-I$(TF_DIR)/bazel-genfiles \
			-I$(TF_INC_DIR)/external/protobuf_archive/src \
            -I$(EIGEN) \
            -I$(ABSL)

TARGET = libinference-ctc.so
all:$(TARGET) 

$(TARGET):$(OBJS) 
	$(CC) -shared -fPIC $^ -o $@

%.o:%.cc
	$(CC) $(INCLUDES) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.so

include ../path.mk


