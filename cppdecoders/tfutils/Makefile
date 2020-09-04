CC = g++ -lstdc++ -std=c++11 -O2 -ldl
SRCS = tfmodel.cc 
OBJS = $(SRCS:.cc=.o)

LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w 

INCLUDES =  -I./tfutils \
			-I../feat \
			-I$(TF_DIR) \
			-I$(TF_DIR)/bazel-genfiles \
			-I$(TF_INC_DIR)/external/protobuf_archive/src \
            -I$(EIGEN) \
            -I$(ABSL)

all:$(OBJS)

%.o:%.cc
	$(CC) $(INCLUDES) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf *.o 

include ../path.mk
