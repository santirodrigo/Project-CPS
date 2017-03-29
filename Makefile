DIR=/usr/local
LIBS= \
   -lgecodeflatzinc  -lgecodedriver \
   -lgecodegist      -lgecodesearch \
   -lgecodeminimodel -lgecodeset    \
   -lgecodefloat     -lgecodeint    \
   -lgecodekernel    -lgecodesupport

p: p.cpp
	g++ -I$(DIR)/include -c p.cpp
	g++ -L$(DIR)/lib -o p p.o $(LIBS)
