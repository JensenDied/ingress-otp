CC = g++
CCFLAGS = -Wall -std=c++11
BPATH = ./bin/
OPATH = ./obj/
SPATH = ./src/
BNAME = otp


all: otp.bin

debug: CCFLAGS += -g3 -pg
debug: all

fast: CCFLAGS += -O2
fast: all

obj/%.o: src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

bin/otp.bin: obj/otp.o obj/permutation_base.o obj/md5.o
	$(CC) $(CCFLAGS) -o $@ $^

otp.bin: bin/otp.bin
	ln -f $(BPATH)$(BNAME).bin $(BNAME).bin

clean:
	rm -f ./obj/*.o ./bin/*.bin ./otp.bin

