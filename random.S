@ Random number generator
@
@ This uses a 33-bit feedback shift register to generate a pseudo-randomly
@ ordered sequence of numbers which repeats in a cycle of length 2^33 - 1
@ NOTE: randomseed should not be set to 0, otherwise a zero will be generated
@ continuously (not particularly random!).
@
@ This is a good application of direct ARM assembler, because the 33-bit
@ shift register can be implemented using RRX (which uses reg + carry).
@ An ANSI C version would be less efficient as the compiler would not use RRX.


    @AREA  |C$$code|, CODE, READONLY
@|C$$code|:
          @.section .text,"x"
          @.code 32

	.global  randomnumber
randomnumber:
@ on exit:
@	a1 = low 32-bits of pseudo-random number
@	a2 = high bit (if you want to know it)
	ldr     ip, =seedpointer
	ldmia   ip, {a1, a2}
	tst     a2, a2, LSR#1           @ to bit into carry
	movs    a3, a1, RRX             @ 33-bit rotate right
	adc     a2, a2, a2              @ carry into LSB of a2
	eor     a3, a3, a1, LSL#12      @ (involved!)
	eor     a1, a3, a3, LSR#20      @ (similarly involved!)
	stmia   ip, {a1, a2}

	mov     pc, lr

@*****************************************************************************
@
@ seedrand set seed number
@
@*****************************************************************************
	.global  seedrand
seedrand:
    @
    @ Save the non-volatile registers which we will use.
    @
    stmdb   r13!, {r1}

	@
	@ Get a pointer to the state variables.
	@
	ldr     r1, =seed
	str     r0, [r1, #0]

	ldmia   r13!, {r1}

    @
    @ Return to the caller.
    @
    mov     pc, lr

		
seedpointer:
        .word     seed


        @AREA    |Random$$data|, DATA
        @.section .data,"x"

        .global  seed
seed:
        .word     0x55555555
        .word     0x55555555

        .end
