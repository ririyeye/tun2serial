prefix = 

CC = $(prefix)gcc
CXX = $(prefix)g++

LDFLAG = -lserialport -lpthread
CXXFLAG = -std=c++11


all:tunserial



tunserial:main.cpp serial.o tundev.o
	$(CXX) $^ -o $@ $(CXXFLAG) $(LDFLAG)

serial.o :serial.cpp
	$(CXX) -c $^ -o $@ $(CXXFLAG)

tundev.o :tundev.cpp
	$(CXX) -c $^ -o $@ $(CXXFLAG)


.PHONY :clean
clean:
	-rm tunserial
	-rm  *.o