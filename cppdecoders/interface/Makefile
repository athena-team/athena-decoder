GCC = g++
INC = -I. -I../
CPPFLAGS = -std=c++11 -g -fPIC $(INC)

STREAMING_OBJ = streaming-decoder-impl.o 
STREAMING_TARGET = libstreaming-decoder.a

OFFLINE_OBJ = offline-decoder-impl.o
OFFLINE_TARGET = liboffline-decoder.a



all:$(STREAMING_TARGET) $(OFFLINE_TARGET)

$(STREAMING_TARGET):$(STREAMING_OBJ) ../decoder/faster-decoder.o \
	../utils/graph-io.o ../utils/utils.o ../ctc/inference.o ../ctc/ctc.o ../feat/CFeatureClass.o ../feat/CFrontend.o \
	../feat/FeatureExtraction.o ../tfutils/tfmodel.o
	$(BUILD_LIBRARY)

$(OFFLINE_TARGET):$(OFFLINE_OBJ) ../decoder/transformer-beam-search-decoder.o ../decoder/transformer-argmax-decoder.o ../decoder/transformer-faster-decoder.o \
	../utils/graph-io.o ../utils/utils.o ../transformer/inference.o ../transformer/decoder.o ../transformer/encoder.o ../feat/CFeatureClass.o ../feat/CFrontend.o \
	../feat/FeatureExtraction.o ../tfutils/tfmodel.o
	$(BUILD_LIBRARY)

%.o:%.cc
	$(GCC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.a

include ../lib.mk
