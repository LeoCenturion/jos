/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

	TRAPHANDLER_NOEC(int0, T_DIVIDE);
	TRAPHANDLER_NOEC(int1, T_DEBUG);
	TRAPHANDLER_NOEC(int2, T_NMI);
	TRAPHANDLER_NOEC(int3, T_BRKPT);
	TRAPHANDLER_NOEC(int4, T_OFLOW);
	TRAPHANDLER_NOEC(int5, T_BOUND);
	TRAPHANDLER_NOEC(int6, T_ILLOP);
	TRAPHANDLER_NOEC(int7, T_DEVICE);
	TRAPHANDLER(int8, T_DBLFLT);
/*	TRAPHANDLER_NOEC(int9, T_COPROC);*/
	TRAPHANDLER(int10, T_TSS);
	TRAPHANDLER(int11, T_SEGNP);
	TRAPHANDLER(int12, T_STACK);
	TRAPHANDLER(int13, T_GPFLT);
	TRAPHANDLER(int14, T_PGFLT);
/*	TRAPHANDLER_NOEC(int15, T_RES);*/
	TRAPHANDLER_NOEC(int16, T_FPERR);
	TRAPHANDLER(int17, T_ALIGN);
	TRAPHANDLER_NOEC(int18, T_MCHK);
	TRAPHANDLER_NOEC(int19, T_SIMDERR);
	TRAPHANDLER_NOEC(int32, IRQ_OFFSET+IRQ_TIMER);
	TRAPHANDLER_NOEC(int48, T_SYSCALL);


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	push %ds
	push %es
	pusha
	movw $GD_KD, %ax
	movw %ax, %ds
	movw %ax, %es
	push %esp
	call trap
