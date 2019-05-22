.PHONY: build help clean bin 

CC = gcc
CFLAGS = -std=gnu90

objects = bin/main bin/limb.o bin/utils.o bin/bulb

default: help

bin: $(objects)

build: 
	@make dummy_clean
	@mkdir -p /tmp/pipes
	@mkdir -p bin
	@make bin
	@chmod +x bin/*
	@ln -s bin/main run
	@echo "Build complete"

bin/main: src/main.c bin/utils.o bin/limb.o 
	$(CC) $(CFLAGS) -o bin/main src/main.c bin/utils.o bin/limb.o 
	
bin/bulb: src/bulb.c
	$(CC) $(CFLAGS) -o bin/bulb src/bulb.c bin/utils.o

bin/utils.o: src/utils.c
	$(CC) $(CFLAGS) -c -o bin/utils.o src/utils.c
	@echo "Compiling utils"

bin/limb.o: src/limb.c
	$(CC) $(CFLAGS) -c -o bin/limb.o src/limb.c
	@echo "Compiling limb"

clean:
	@rm -rf /tmp/pipes/
	@rm -rf bin/
	@rm -f run
	@echo "Clean complete"
	
dummy_clean: 
	@rm -rf /tmp/pipes/
	@rm -f run
	@echo "Clean complete"


help:
	@echo "Made by Alessia Marcolini, Matteo Slaviero, Nicolò Gottardello"
	@echo "To start, run 'make build' and then './run'"
