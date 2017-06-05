DIR=/usr/local
LIBS= \
   -lgecodeflatzinc  -lgecodedriver \
   -lgecodegist      -lgecodesearch \
   -lgecodeminimodel -lgecodeset    \
   -lgecodefloat     -lgecodeint    \
   -lgecodekernel    -lgecodesupport

all: logic-synthesis.cpp
	g++ -I$(DIR)/include -c logic-synthesis.cpp
	g++ -L$(DIR)/lib -o logic.x logic-synthesis.o $(LIBS)
