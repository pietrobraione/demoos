	.section ".text.boot"

	.globl _start

_start:
	mrs 	x5, mpidr_el1
	and	x5, x5, #3
	cbz	x5, init_primary_code

wait_for_event:
	wfe
	b	wait_for_event

init_primary_code:
	ldr 	x5, =_start
	mov	sp, x5
	ldr	x5, =__bss_start
	ldr	w6, =__bss_size

clear_bss_loop:
	cbz	w6, kernel_launch
	str	xzr, [x5], #8
	sub 	w6, w6, #1
	cbnz	w6, clear_bss_loop

kernel_launch:
	bl	kernel_main
	b	wait_for_event


	.globl get_el

get_el:
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret
