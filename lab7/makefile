CC = gcc
CFLAGS = -Wall -pedantic
APP = labCLI
APP_SOURCES = main.c
APP_OBJECTS = labCLI.o

all: $(APP) clean run

$(APP): 
	$(CC) $(APP_SOURCES) -o $@

.PHONY: clean
clean:
	del $(APP_OBJECTS) 
run:
	$(APP)