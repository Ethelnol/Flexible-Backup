ALL: FlexibleBackup
CPP=g++
OPTIMISED=O0
VERSION=c++20

backup.o : backup.cpp backup.h shared.o
	$(CPP) -$(OPTIMISED) -std=$(VERSION) -c backup.cpp

config.o : config.cpp config.h shared.o
	$(CPP) -$(OPTIMISED) -std=$(VERSION) -c config.cpp

scan.o : scan.cpp scan.h shared.o
	$(CPP) -$(OPTIMISED) -std=$(VERSION) -c scan.cpp

shared.o : shared.cpp shared.h
	$(CPP) -$(OPTIMISED) -std=$(VERSION) -c shared.cpp

FlexibleBackup : backup.o config.o scan.o shared.o
	$(CPP) -o FlexibleBackup backup.o config.o scan.o shared.o

clean:
	rm -f *.o
