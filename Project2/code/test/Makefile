# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: input exec halt matmult sort testfiles createlock fork createcv acquirelock releaselock destroylock wait signal broadcast destroycv test1 test2 test3 test4 test5 test6 test7 test8 test9 project2 project2simulation test

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

createlock.o: createlock.c
	$(CC) $(CFLAGS) -c createlock.c
createlock: createlock.o start.o
	$(LD) $(LDFLAGS) start.o createlock.o -o createlock.coff
	../bin/coff2noff createlock.coff createlock

acquirelock.o: acquirelock.c
	$(CC) $(CFLAGS) -c acquirelock.c
acquirelock: acquirelock.o start.o
	$(LD) $(LDFLAGS) start.o acquirelock.o -o acquirelock.coff
	../bin/coff2noff acquirelock.coff acquirelock

releaselock.o: releaselock.c
	$(CC) $(CFLAGS) -c releaselock.c
releaselock: releaselock.o start.o
	$(LD) $(LDFLAGS) start.o releaselock.o -o releaselock.coff
	../bin/coff2noff releaselock.coff releaselock

destroylock.o: destroylock.c
	$(CC) $(CFLAGS) -c destroylock.c
destroylock: destroylock.o start.o
	$(LD) $(LDFLAGS) start.o destroylock.o -o destroylock.coff
	../bin/coff2noff destroylock.coff destroylock
	
sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

wait.o: wait.c
	$(CC) $(CFLAGS) -c wait.c
wait: wait.o start.o
	$(LD) $(LDFLAGS) start.o wait.o -o wait.coff
	../bin/coff2noff wait.coff wait

	
matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

signal.o: signal.c
	$(CC) $(CFLAGS) -c signal.c
signal: signal.o start.o
	$(LD) $(LDFLAGS) start.o signal.o -o signal.coff
	../bin/coff2noff signal.coff signal

	
fork.o: fork.c
	$(CC) $(CFLAGS) -c fork.c
fork: fork.o start.o
	$(LD) $(LDFLAGS) start.o fork.o -o fork.coff
	../bin/coff2noff fork.coff fork
	
exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o
	$(LD) $(LDFLAGS) start.o exec.o -o exec.coff
	../bin/coff2noff exec.coff exec
	
input.o: input.c
	$(CC) $(CFLAGS) -c input.c
input: input.o start.o
	$(LD) $(LDFLAGS) start.o input.o -o input.coff
	../bin/coff2noff input.coff input
	
createcv.o: createcv.c
	$(CC) $(CFLAGS) -c createcv.c
createcv: createcv.o start.o
	$(LD) $(LDFLAGS) start.o createcv.o -o createcv.coff
	../bin/coff2noff createcv.coff createcv

broadcast.o: broadcast.c
	$(CC) $(CFLAGS) -c broadcast.c
broadcast: broadcast.o start.o
	$(LD) $(LDFLAGS) start.o broadcast.o -o broadcast.coff
	../bin/coff2noff broadcast.coff broadcast

destroycv.o: destroycv.c
	$(CC) $(CFLAGS) -c destroycv.c
destroycv: destroycv.o start.o
	$(LD) $(LDFLAGS) start.o destroycv.o -o destroycv.coff
	../bin/coff2noff destroycv.coff destroycv

test1.o: test1.c
	$(CC) $(CFLAGS) -c test1.c
test1: test1.o start.o
	$(LD) $(LDFLAGS) start.o test1.o -o test1.coff
	../bin/coff2noff test1.coff test1

test2.o: test2.c
	$(CC) $(CFLAGS) -c test2.c
test2: test2.o start.o
	$(LD) $(LDFLAGS) start.o test2.o -o test2.coff
	../bin/coff2noff test2.coff test2

test3.o: test3.c
	$(CC) $(CFLAGS) -c test3.c
test3: test3.o start.o
	$(LD) $(LDFLAGS) start.o test3.o -o test3.coff
	../bin/coff2noff test3.coff test3

test4.o: test4.c
	$(CC) $(CFLAGS) -c test4.c
test4: test4.o start.o
	$(LD) $(LDFLAGS) start.o test4.o -o test4.coff
	../bin/coff2noff test4.coff test4

test5.o: test5.c
	$(CC) $(CFLAGS) -c test5.c
test5: test5.o start.o
	$(LD) $(LDFLAGS) start.o test5.o -o test5.coff
	../bin/coff2noff test5.coff test5
	
test6.o: test6.c
	$(CC) $(CFLAGS) -c test6.c
test6: test6.o start.o
	$(LD) $(LDFLAGS) start.o test6.o -o test6.coff
	../bin/coff2noff test6.coff test6
	
test7.o: test7.c
	$(CC) $(CFLAGS) -c test7.c
test7: test7.o start.o
	$(LD) $(LDFLAGS) start.o test7.o -o test7.coff
	../bin/coff2noff test7.coff test7

test8.o: test8.c
	$(CC) $(CFLAGS) -c test8.c
test8: test8.o start.o
	$(LD) $(LDFLAGS) start.o test8.o -o test8.coff
	../bin/coff2noff test8.coff test8
	
test9.o: test9.c
	$(CC) $(CFLAGS) -c test9.c
test9: test9.o start.o
	$(LD) $(LDFLAGS) start.o test9.o -o test9.coff
	../bin/coff2noff test9.coff test9

project2.o: project2.c
	$(CC) $(CFLAGS) -c project2.c
project2: project2.o start.o
	$(LD) $(LDFLAGS) start.o project2.o -o project2.coff
	../bin/coff2noff project2.coff project2
	
project2simulation.o: project2simulation.c
	$(CC) $(CFLAGS) -c project2simulation.c
project2simulation: project2simulation.o start.o
	$(LD) $(LDFLAGS) start.o project2simulation.o -o project2simulation.coff
	../bin/coff2noff project2simulation.coff project2simulation
	
test.o: test.c
	$(CC) $(CFLAGS) -c test.c
test: test.o start.o
	$(LD) $(LDFLAGS) start.o test.o -o test.coff
	../bin/coff2noff test.coff test

clean:
	rm -f *.o *.coff
