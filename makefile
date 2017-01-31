CC = g++
STDFLAG = -std=c++11
CFLAGS = -g -c -Wall -pedantic
LINKFLAGS = -pthread 
LIBFLAGS = -lg2

default: tcas
build-default: tcas

tcas : tcas.o TCAS_comms.o TCAS_defs.o Navigation.o TCAS_sim.o AC_sim.o Radar.o TCAS_CLI.o
	$(CC) $(LINKFLAGS) tcas.o TCAS_comms.o TCAS_defs.o Navigation.o TCAS_sim.o AC_sim.o Radar.o TCAS_CLI.o $(LIBFLAGS) -o tcas
    
tcas.o : TCAS_main.cpp TCAS_defs.o AC_sim.h Radar.h TCAS_comms.h TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) TCAS_main.cpp -o tcas.o
    
TCAS_comms.o : TCAS_comms.cpp TCAS_comms.h TCAS_defs.o TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) TCAS_comms.cpp
    
TCAS_defs.o : TCAS_defs.cpp TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) TCAS_defs.cpp
    
Navigation.o : Navigation.cpp Navigation.h TCAS_defs.o TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) Navigation.cpp
    
TCAS_sim.o : TCAS_sim.cpp TCAS_sim.h TCAS_defs.o TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) TCAS_sim.cpp
    
AC_sim.o : AC_sim.cpp AC_sim.h TCAS_defs.o TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) AC_sim.cpp
    
Radar.o : Radar.cpp Radar.h TCAS_defs.o TCAS_defs.h
	$(CC) $(STDFLAG) $(CFLAGS) Radar.cpp

TCAS_CLI.o : TCAS_CLI.h TCAS_CLI.cpp
	$(CC) $(STDFLAG) $(CFLAGS) TCAS_CLI.cpp
    
.PHONY : clean
clean : 
	-rm *.o tcas
