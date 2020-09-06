
INCLUDEDIR = -I../ -I../tools/gflags/build/include/ 
GCC = g++
CPPFLAGS = -std=c++11 -g -fPIC $(INCLUDEDIR)

LIBS = -lpthread -ldl -L../ctc -Wl,-Bstatic -linference-ctc -Wl,-Bdynamic \
	   -L../lib -ltensorflow_cc -l tensorflow_framework \
	   -L../tools/gflags/build/lib/ -lgflags

TARGET = ctc-decode-faster
OBJ = ctc-decode-faster.o ../decoder/faster-decoder.o ../utils/graph-io.o ../utils/utils.o 

all: $(TARGET)
$(TARGET):$(OBJ)
	$(GCC) $(CPPFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -rf $(TARGET) *.o


