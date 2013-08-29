CC = g++
CCFLAGS = -Wall -std=c++11
BPATH = ./bin/
OPATH = ./obj/
SPATH = ./src/
BNAME = otp


all: path_setup otp.bin

debugclean:
	rm -f ./obj/otp.o ./obj/md5.o ./bin/*.bin ./otp.bin

debug: CCFLAGS += -g -pg
debug: debugclean all

fast: CCFLAGS += -Ofast -march=native
fast: all

obj/%.o: src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

path_setup:
	test -d $(OPATH) || mkdir $(OPATH)
	test -d $(BPATH) || mkdir $(BPATH)

bin/otp.bin: obj/otp.o obj/permutation_base.o obj/md5.o
	$(CC) $(CCFLAGS) -o $@ $^

otp.bin: bin/otp.bin
	ln -f $(BPATH)$(BNAME).bin $(BNAME).bin

clean:
	rm -f ./obj/*.o ./bin/*.bin ./otp.bin

