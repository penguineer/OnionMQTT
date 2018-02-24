# http://stackoverflow.com/questions/2145590/what-is-the-purpose-of-phony-in-a-makefile

DEBUG   = -O3                                                                   
CC      = gcc                                                                   
INCLUDE = -I/usr/local/include                                                  
CFLAGS  = $(DEBUG) -Wall $(INCLUDE) -Winline -pipe                              
                                                                                
LDFLAGS = -L/usr/local/lib                                                      
LDLIBS    = -lpthread -lm -lmosquitto -loniondebug -lonioni2c -lonionoledexp


.phony: clean

all: onionmqtt

clean:
	rm onionmqtt *.o

onionmqtt: onionmqtt.o
	@$(CC) -o $@ onionmqtt.o $(LDFLAGS) $(LDLIBS) 

onionmqtt.o: onionmqtt.c
	@$(CC) -c onionmqtt.c -o $@

