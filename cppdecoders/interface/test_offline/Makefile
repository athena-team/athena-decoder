GCC = g++
INC = -Iinclude 
CPPFLAGS = -g -std=c++11 -fPIC $(INC)
LDFLAGS = -Wall -fPIC -Wshadow -Wno-sign-compare -w -Llib -loffline-decoder \
 -ldl -ltensorflow_cc -ltensorflow_framework 

OBJS = how_to_use_api.o
TARGET = how_to_use_api

all:$(TARGET)

$(TARGET):$(OBJS)
	$(GCC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

%.o:%.cc
		$(GCC) $(CPPFLAGS) -c $< -o $@
clean:
		rm -rf $(TARGET) *.o


