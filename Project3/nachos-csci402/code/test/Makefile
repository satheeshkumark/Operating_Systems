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

all: input exec halt matmult sort testfiles createlock fork createcv acquirelock releaselock destroylock wait signal broadcast destroycv project2 project2simulation test matmult_fork testcasesForProject3_part1_2 testcasesForProject3_part3

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
	
matmult_fork.o: matmult_fork.c
	$(CC) $(CFLAGS) -c matmult_fork.c
matmult_fork: matmult_fork.o start.o
	$(LD) $(LDFLAGS) start.o matmult_fork.o -o matmult_fork.coff
	../bin/coff2noff matmult_fork.coff matmult_fork

testcasesForProject3_part1_2.o: testcasesForProject3_part1_2.c
	$(CC) $(CFLAGS) -c testcasesForProject3_part1_2.c
testcasesForProject3_part1_2: testcasesForProject3_part1_2.o start.o
	$(LD) $(LDFLAGS) start.o testcasesForProject3_part1_2.o -o testcasesForProject3_part1_2.coff
	../bin/coff2noff testcasesForProject3_part1_2.coff testcasesForProject3_part1_2
	
testcasesForProject3_part3.o: testcasesForProject3_part3.c
	$(CC) $(CFLAGS) -c testcasesForProject3_part3.c
testcasesForProject3_part3: testcasesForProject3_part3.o start.o
	$(LD) $(LDFLAGS) start.o testcasesForProject3_part3.o -o testcasesForProject3_part3.coff
	../bin/coff2noff testcasesForProject3_part3.coff testcasesForProject3_part3
	
clean:
	rm -f *.o *.coff
