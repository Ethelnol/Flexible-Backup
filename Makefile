.DELETE_ON_ERROR:
CXX = g++
CPPFLAGS ?= -O2
OBJS = backup.o config.o out.o scan.o shared.o
EXECUTABLE_NAME = FlexibleBackup
BIN_PREFIX ?= "$(HOME)/.local/bin"
CONFIG_PREFIX ?= "$(HOME)/.config"

all: $(OBJS)
	$(CXX) $(CPPFLAGS) $^ -o $(EXECUTABLE_NAME)
.PHONY: all

%.o : %.c
	$(CXX) $(CPPFLAGS) -c $^ -o $@
.PHONY: %.o

install:
	@cp $(EXECUTABLE_NAME) $(BIN_PREFIX)/
	@cp flexible-backup.conf.example $(CONFIG_PREFIX)/flexible-backup.conf
.PHONY: install

uninstall:
	@rm -f $(BIN_PREFIX)/$(EXECUTABLE_NAME)
.PHONY: uninstall

clean:
	rm -f *.o
.PHONY: clean
