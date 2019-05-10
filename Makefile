
# BNN

LIB_bnn = ./lib_bnn
LIB_hls = ./hls

CXX= g++

CFLAGS+=  -O2 -I . 
CFLAGS+= -Wno-unused-result -Wno-write-strings -Wno-deprecated-register  -std=c++11
#CFLAGSA = $(CFLAGS) -std=c++11

#XI_CFLAGS = $(CFLAGS) -DXILINX -DOFFLOAD --std=gnu++11 -march=armv7-a -I $(LIB_bnn)  -I $(LIB_hls) -I ${CURDIR} -mfloat-abi=hard 


XI_CFLAGS = $(CFLAGS) -DXILINX -DOFFLOAD  -march=armv7-a -I $(LIB_bnn)  -I $(LIB_hls) -I ${CURDIR} -mfloat-abi=hard 

XI_CFLAGS+= -DNEON -mfpu=neon -funsafe-math-optimizations -ftree-vectorize -mvectorize-with-neon-quad -ftree-vectorizer-verbose=2 #If NEON SIMD instructions are to be used


XI_LDFLAGS = -L $(LIB_bnn)

LDFLAGS = -pthread -lpthread -z muldefs

XI_LDFLAGS+= -lrt -lkernelbnn 

XI_PROGs= BNN

SOURCE= main.cpp   kernelbnn.h

FLAGS = -std=c++11
OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

.PHONY: all clean odroid

all: odroid

odroid: $(XI_PROGs)


foldedmv-offload.o: foldedmv-offload.cpp foldedmv-offload.h
	$(CXX) -c foldedmv-offload.cpp $(XI_CFLAGS)

rawhls-offload.o: rawhls-offload.cpp 
	$(CXX) -c rawhls-offload.cpp $(XI_CFLAGS)
	
opencam.o: opencam.cpp
	$(CXX) $(FLAGS) -c opencam.cpp $(LIBS)


BNN: $(SOURCE) foldedmv-offload.o rawhls-offload.o opencam.o
	$(CXX) $(FLAGS) -o $@ $< foldedmv-offload.o rawhls-offload.o opencam.o $(XI_CFLAGS) $(XI_LDFLAGS)  $(LDFLAGS) $(LIBS)
	

clean:
	rm -f  $(XI_PROGs) foldedmv-offload.o rawhls-offload.o

