/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#define IN_ASM
#include "syscall.h"

        .text   
        .align  2

/* -------------------------------------------------------------
 * __start
 *	Initialize running a C program, by calling "main". 
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

	.globl __start
	.ent	__start
__start:
	jal	main
	move	$4,$0		
	jal	Exit	 /* if we return from main, exit(0) */
	.end __start

/* -------------------------------------------------------------
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r2, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r4, arg2 is 
 *	in r5, arg3 is in r6, arg4 is in r7)
 *
 * 	The return value is in r2. This follows the standard C calling
 * 	convention on the MIPS.
 * -------------------------------------------------------------
 */

	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,SC_Halt
	syscall
	j	$31
	.end Halt

	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,SC_Exit
	syscall
	j	$31
	.end Exit

	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,SC_Exec
	syscall
	j	$31
	.end Exec

	.globl Join
	.ent	Join
Join:
	addiu $2,$0,SC_Join
	syscall
	j	$31
	.end Join

	.globl Create
	.ent	Create
Create:
	addiu $2,$0,SC_Create
	syscall
	j	$31
	.end Create

	.globl Open
	.ent	Open
Open:
	addiu $2,$0,SC_Open
	syscall
	j	$31
	.end Open

	.globl Read
	.ent	Read
Read:
	addiu $2,$0,SC_Read
	syscall
	j	$31
	.end Read

	.globl Write
	.ent	Write
Write:
	addiu $2,$0,SC_Write
	syscall
	j	$31
	.end Write

	.globl Close
	.ent	Close
Close:
	addiu $2,$0,SC_Close
	syscall
	j	$31
	.end Close

	.globl Fork
	.ent	Fork
Fork:
	addiu $2,$0,SC_Fork
	syscall
	j	$31
	.end Fork

	.globl Yield
	.ent	Yield
Yield:
	addiu $2,$0,SC_Yield
	syscall
	j	$31
	.end Yield

	.globl Flag
	.ent	Flag
Flag:
	addiu $2,$0,SC_Flag
	syscall
	j	$31
	.end Flag

	.globl CreateLock
	.ent	CreateLock
CreateLock:
	addiu $2,$0,SC_CreateLock
	syscall
	j	$31
	.end CreateLock
	
	.globl AcquireLock
	.ent	AcquireLock
AcquireLock:
	addiu $2,$0,SC_AcquireLock
	syscall
	j	$31
	.end AcquireLock
	
	.globl ReleaseLock
	.ent	ReleaseLock
ReleaseLock:
	addiu $2,$0,SC_ReleaseLock
	syscall
	j	$31
	.end ReleaseLock
	
	.globl DestroyLock
	.ent	DestroyLock
DestroyLock:
	addiu $2,$0,SC_DestroyLock
	syscall
	j	$31
	.end DestroyLock

	.globl CreateCV
	.ent	CreateCV
CreateCV:
	addiu $2,$0,SC_CreateCV
	syscall
	j	$31
	.end CreateCV

	.globl SignalCV
	.ent	SignalCV
SignalCV:
	addiu $2,$0,SC_SignalCV
	syscall
	j	$31
	.end SignalCV
	
	.globl WaitCV
	.ent	WaitCV
WaitCV:
	addiu $2,$0,SC_WaitCV
	syscall
	j	$31
	.end WaitCV

	.globl BroadcastCV
	.ent	BroadcastCV
BroadcastCV:
	addiu $2,$0,SC_BroadcastCV
	syscall
	j	$31
	.end BroadcastCV
	
	.globl DestroyCV
	.ent	DestroyCV
DestroyCV:
	addiu $2,$0,SC_DestroyCV
	syscall
	j	$31
	.end DestroyCV
	
	.globl Print
	.ent	Print
Print:
	addiu $2,$0,SC_Print
	syscall
	j	$31
	.end Print
	
	.globl Print1
	.ent	Print1
Print1:
	addiu $2,$0,SC_Print1
	syscall
	j	$31
	.end Print1
	
	.globl Print2
	.ent	Print2
Print2:
	addiu $2,$0,SC_Print2
	syscall
	j	$31
	.end Print2
	
	.globl Print3
	.ent	Print3
Print3:
	addiu $2,$0,SC_Print3
	syscall
	j	$31
	.end Print3
	
	.globl PrintTest
	.ent	PrintTest
PrintTest:
	addiu $2,$0,SC_PrintTest
	syscall
	j	$31
	.end PrintTest
	
	.globl RandomSearch
	.ent RandomSearch
RandomSearch:
	addiu $2,$0,SC_Rand
	syscall
	j	$31
	.end RandomSearch
	
	.globl scan
	.ent scan
scan:
	addiu $2,$0,SC_Scan
	syscall
	j	$31
	.end scan
	
	.globl Flag
	.ent Flag
Flag:
	addiu $2,$0,SC_Flag
	syscall
	j	$31
	.end Flag
	
	.globl CreateMV
	.ent	CreateMV
CreateMV:
	addiu $2,$0,SC_CreateMV
	syscall
	j	$31
	.end CreateMV
	
	.globl GetMV
	.ent	GetMV
GetMV:
	addiu $2,$0,SC_GetMV
	syscall
	j	$31
	.end GetMV
	
	.globl SetMV
	.ent	SetMV
SetMV:
	addiu $2,$0,SC_SetMV
	syscall
	j	$31
	.end SetMV
	
	.globl DestroyMV
	.ent	DestroyMV
DestroyMV:
	addiu $2,$0,SC_DestroyMV
	syscall
	j	$31
	.end DestroyMV
	
/* dummy function to keep gcc happy */
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main

