
all: logic-synthesis-cplex

logic-synthesis-cplex: logic-synthesis-cplex.o
	$(CCC) $(CCFLAGS) logic-synthesis-cplex.o -o logic-cplex.x $(CCLNFLAGS)
logic-synthesis-cplex.o: logic-synthesis-cplex.cpp
	$(CCC) -c $(CCFLAGS) logic-synthesis-cplex.cpp -o logic-synthesis-cplex.o

clean:
	rm --force logic-cplex.x logic-synthesis-cplex.o

################################

# Uncomment for CPLEX_Enterprise_Server126
 SYSTEM     = x86-64_linux
 LIBFORMAT  = static_pic
 CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Enterprise_Server126/CPLEX_Studio/cplex
 CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Enterprise_Server126/CPLEX_Studio/concert

# Uncomment for CPLEX_Studio124
#SYSTEM     = x86-64_sles10_4.1
#LIBFORMAT  = static_pic
#CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio124/cplex
#CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio124/concert

################

CCC = g++

CCOPT = -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD

CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 
