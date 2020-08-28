
INCLUDEDIR = -I../ -I../tools/cxxopts/include/ 
GCC = g++
CPPFLAGS = -std=c++11 -g -fPIC $(INCLUDEDIR)

LIBS = -lpthread -ldl 

TARGET1 = ctc-decode-faster
OBJ1 = ctc-decode-faster.o ../decoder/faster-decoder.o ../utils/graph-io.o ../utils/utils.o 

all: $(TARGET1)
$(TARGET1):$(OBJ1)
	$(GCC) $(CPPFLAGS) $(LIBS) $^ -o $@ 

clean:
	rm -rf $(TARGET1) *.o


