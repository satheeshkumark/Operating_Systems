









































































































 

















































#ident	"@(#)trap.h	1.34	04/05/22 SMI"






























































#ident	"@(#)asm_linkage.h	1.39	04/02/02 SMI"









#ident	"@(#)stack.h	1.22	04/09/28 SMI"




































































































































































































































.seg    "text"










.globl        ThreadRoot
ThreadRoot:

	nop  ; nop         

	clr	%fp        

			   


	mov	%o0, %l0  
	mov	%o1, %l1
	mov	%o2, %l2
			   




	call	%o3,0
	nop
	call	%l0, 1	
	mov	%l1, %o0   
	call	%l2, 0
	nop
			   

	ta	0x01



.globl        SWITCH
SWITCH:

	save	%sp, -(((((16*4) + (6*4) + 4))+(8-1)) & ~(8-1)), %sp
	st	%fp, [%i0]
	st	%i0, [%i0+4]
	st	%i1, [%i0+8]
	st	%i2, [%i0+12]
	st	%i3, [%i0+16]
	st	%i4, [%i0+20]
	st	%i5, [%i0+24]
	st	%i7, [%i0+32]
	ta	0x03
	nop
	mov	%i1, %l0
	ld	[%l0+4], %i0
	ld	[%l0+8], %i1
	ld	[%l0+12], %i2
	ld	[%l0+16], %i3
	ld	[%l0+20], %i4
	ld	[%l0+24], %i5
	ld	[%l0+32], %i7
	ld	[%l0], %i6
	ret
	restore






