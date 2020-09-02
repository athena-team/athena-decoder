
INCLUDEDIR = -I../ -I../tools/gflags/build/include/ 
GCC = g++
CPPFLAGS = -std=c++11 -g -fPIC $(INCLUDEDIR)

LIBS = -lpthread -ldl -L../transformer -linference-transformer \
	   -L../lib -ltensorflow_cc -l tensorflow_framework \
	   -L../tools/gflags/build/lib/ -lgflags

TARGET1 = transformer-decode-faster
OBJ1 = transformer-decode-faster.o ../decoder/transformer-faster-decoder.o ../utils/graph-io.o ../utils/utils.o 
TARGET2 = transformer-beam-search
OBJ2 = transformer-beam-search.o ../decoder/transformer-beam-search-decoder.o ../utils/graph-io.o ../utils/utils.o 
TARGET3 = transformer-argmax
OBJ3 = transformer-argmax.o ../decoder/transformer-argmax-decoder.o ../utils/graph-io.o ../utils/utils.o 

all: $(TARGET1) $(TARGET2) $(TARGET3)
$(TARGET1):$(OBJ1)
	$(GCC) $(CPPFLAGS) $^ -o $@ $(LIBS)
$(TARGET2):$(OBJ2)
	$(GCC) $(CPPFLAGS) $^ -o $@ $(LIBS)
$(TARGET3):$(OBJ3)
	$(GCC) $(CPPFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf $(TARGET1) $(TARGET2) $(TARGET3) *.o


