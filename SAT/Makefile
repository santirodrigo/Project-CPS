
all: logic-synthesis-sat

logic-synthesis-sat: logic-synthesis-sat.o
	$(CCC) $(CCFLAGS) logic-synthesis-sat.o -o logic-sat.x $(CCLNFLAGS)
logic-synthesis-sat.o: logic-synthesis-sat.cpp
	$(CCC) -c $(CCFLAGS) logic-synthesis-sat.cpp -o logic-synthesis-sat.o

clean:
	rm --force logic-sat.x logic-synthesis-sat.o

################################

CCC = g++

CCOPT = -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions

CCLNFLAGS = -lm -pthread

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 
