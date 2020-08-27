
INCLUDEDIR = -I../ 
GCC = g++
CPPFLAGS = -std=c++11 -g -fPIC $(INCLUDEDIR)

LIBS = -lpthread -ldl 

TARGET1 = ctc-decode-faster
OBJ1 = ctc-decode-faster.o ../decoder/faster-decoder.o ../kaldi/kaldi-error.o ../misc/graph-io.o ../misc/utils.o ../kaldi/parse-options.o ../kaldi/text-utils.o

all: $(TARGET1)
$(TARGET1):$(OBJ1)
	$(GCC) $(CPPFLAGS) $(LIBS) $^ -o $@ 

clean:
	rm -rf $(TARGET1) *.o


