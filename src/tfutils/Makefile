CC = g++ -lstdc++ -std=c++11 -O2 -ldl
SRCS = encoder.cc decoder.cc inference.cc tfmodel.cc 
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

TF_DIR := path-of-tensorflow/
TF_INC_DIR := $(TF_DIR)/bazel-tensorflow
TF_LIB_DIR := $(TF_DIR)/bazel-bin/tensorflow

INCLUDES =  -I./tfutils \
			-I../feat \
			-I$(TF_DIR) \
			-I$(TF_DIR)/bazel-genfiles \
			-I$(TF_INC_DIR)/external/protobuf_archive/src \
            -I$(TF_INC_DIR)/external/nsync/public/ \
            -Ipath-of-protobuf/ \
            -Ipath-of-egien3/ \
            -Ipath-of-absl/


all:$(OBJS)

%.o:%.cc
	$(CC) $(INCLUDES) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o 
