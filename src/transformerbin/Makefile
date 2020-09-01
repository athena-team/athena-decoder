
INCLUDEDIR = -I../ -I../tools/gflags/build/include/ 
GCC = g++
CPPFLAGS = -std=c++11 -g -fPIC $(INCLUDEDIR)

LIBS = -lpthread -ldl -L../transformer -linference-transformer \
	   -L../lib -ltensorflow_cc -l tensorflow_framework \
	   -L../tools/gflags/build/lib/ -lgflags

TARGET = transformer-decode-faster
OBJ = transformer-decode-faster.o ../decoder/transformer-faster-decoder.o ../utils/graph-io.o ../utils/utils.o 

all: $(TARGET)
$(TARGET):$(OBJ)
	$(GCC) $(CPPFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf $(TARGET) *.o


