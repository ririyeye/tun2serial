prefix = 

CC = $(prefix)gcc
CXX = $(prefix)g++

LDFLAG = -lpthread
CXXFLAG = -std=c++11


all:tunserial

test:test.cpp serial_protol.o
	$(CXX) $^ -o $@ $(CXXFLAG) $(LDFLAG)

tunserial:main.cpp serial.o tundev.o serial_protol.o
	$(CXX) $^ -o $@ $(CXXFLAG) $(LDFLAG)

serial.o :serial.cpp
	$(CXX) -c $^ -o $@ $(CXXFLAG)

tundev.o :tundev.cpp
	$(CXX) -c $^ -o $@ $(CXXFLAG)

serial_protol.o :serial_protol.cpp
	$(CXX) -c $^ -o $@ $(CXXFLAG)


.PHONY :clean
clean:
	-rm tunserial
	-rm  *.o