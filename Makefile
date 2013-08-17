CC = g++
CCFLAGS = -Wall -std=c++11 -g3 -pg
BPATH = ./bin/
OPATH = ./obj/
SPATH = ./src/
BNAME = otp

all: otp.o md5.o Link


Link:
	$(CC) $(CCFLAGS) -o $(BPATH)$(BNAME).bin $(OPATH)*.o 
	ln -f $(BPATH)$(BNAME).bin $(BNAME).bin

md5.o:
	$(CC) $(CCFLAGS) -c $(SPATH)md5.c -o $(OPATH)md5.o

otp.o:
	$(CC) $(CCFLAGS) -c $(SPATH)otp.c -o $(OPATH)otp.o

clean: 
	rm -f $(OPATH)*.o $(BPATH)$(BNAME).bin $(BNAME).bin
