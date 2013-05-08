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

all: input exec halt matmult sort testfiles createlock createcv acquirelock releaselock customer salesman goodsloader manager cashier cashier_exec customer_exec salesman_exec goodsloader_exec manager_exec 

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

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

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

customer.o: customer.c
	$(CC) $(CFLAGS) -c customer.c
customer: customer.o start.o
	$(LD) $(LDFLAGS) start.o customer.o -o customer.coff
	../bin/coff2noff customer.coff customer
	
salesman.o: salesman.c
	$(CC) $(CFLAGS) -c salesman.c
salesman: salesman.o start.o
	$(LD) $(LDFLAGS) start.o salesman.o -o salesman.coff
	../bin/coff2noff salesman.coff salesman
	
goodsloader.o: goodsloader.c
	$(CC) $(CFLAGS) -c goodsloader.c
goodsloader: goodsloader.o start.o
	$(LD) $(LDFLAGS) start.o goodsloader.o -o goodsloader.coff
	../bin/coff2noff goodsloader.coff goodsloader
	
manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager

cashier.o: cashier.c
	$(CC) $(CFLAGS) -c cashier.c
cashier: cashier.o start.o
	$(LD) $(LDFLAGS) start.o cashier.o -o cashier.coff
	../bin/coff2noff cashier.coff cashier
	
cashier_exec.o: cashier_exec.c
	$(CC) $(CFLAGS) -c cashier_exec.c
cashier_exec: cashier_exec.o start.o
	$(LD) $(LDFLAGS) start.o cashier_exec.o -o cashier_exec.coff
	../bin/coff2noff cashier_exec.coff cashier_exec
	
customer_exec.o: customer_exec.c
	$(CC) $(CFLAGS) -c customer_exec.c
customer_exec: customer_exec.o start.o
	$(LD) $(LDFLAGS) start.o customer_exec.o -o customer_exec.coff
	../bin/coff2noff customer_exec.coff customer_exec
	
salesman_exec.o: salesman_exec.c
	$(CC) $(CFLAGS) -c salesman_exec.c
salesman_exec: salesman_exec.o start.o
	$(LD) $(LDFLAGS) start.o salesman_exec.o -o salesman_exec.coff
	../bin/coff2noff salesman_exec.coff salesman_exec
	
goodsloader_exec.o: goodsloader_exec.c
	$(CC) $(CFLAGS) -c goodsloader_exec.c
goodsloader_exec: goodsloader_exec.o start.o
	$(LD) $(LDFLAGS) start.o goodsloader_exec.o -o goodsloader_exec.coff
	../bin/coff2noff goodsloader_exec.coff goodsloader_exec
	
manager_exec.o: manager_exec.c
	$(CC) $(CFLAGS) -c manager_exec.c
manager_exec: manager_exec.o start.o
	$(LD) $(LDFLAGS) start.o manager_exec.o -o manager_exec.coff
	../bin/coff2noff manager_exec.coff manager_exec
	
clean:
	rm -f *.o *.coff
