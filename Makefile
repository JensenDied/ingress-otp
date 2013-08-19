CC = g++
CCFLAGS = -Wall -std=c++11
BPATH = ./bin/
OPATH = ./obj/
SPATH = ./src/
BNAME = otp

all: otp.o md5.o Link

debug: CCFLAGS += -g3 -pg
debug: all

fast: CCFLAGS += -O2
fast: all



Link:
	$(CC) $(CCFLAGS) -o $(BPATH)$(BNAME).bin $(OPATH)*.o
	ln -f $(BPATH)$(BNAME).bin $(BNAME).bin

md5.o:
	$(CC) $(CCFLAGS) -c $(SPATH)md5.c -o $(OPATH)md5.o

otp.o:
	$(CC) $(CCFLAGS) -c $(SPATH)otp.c -o $(OPATH)otp.o

clean: 
	rm -f $(OPATH)*.o $(BPATH)$(BNAME).bin $(BNAME).bin

