ALL: FlexibleBackup
CPP=g++
OPT ?= O0
VERSION=17

backup.o: backup.cpp backup.h shared.h
	$(CPP) -$(OPT) -std=c++$(VERSION) -c backup.cpp

config.o: config.cpp config.h shared.h
	$(CPP) -$(OPT) -std=c++$(VERSION) -c config.cpp

out.o: out.cpp out.h shared.h
	$(CPP) -$(OPT) -std=c++$(VERSION) -c out.cpp

scan.o: scan.cpp scan.h backup.h config.h out.h shared.h
	$(CPP) -$(OPT) -std=c++$(VERSION) -c scan.cpp

shared.o: shared.cpp shared.h
	$(CPP) -$(OPT) -std=c++$(VERSION) -c shared.cpp

FlexibleBackup: backup.o config.o out.o scan.o shared.o
	$(CPP) -o FlexibleBackup backup.o config.o out.o scan.o shared.o

clean:
	rm -f *.o
