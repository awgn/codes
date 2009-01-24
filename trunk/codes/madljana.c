/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Nicola Bonelli
 * ----------------------------------------------------------------------------
 */


#include <stdio.h>

/* This is the asm inline version of the false disassembly routine 
 * written by Silvio Cesere. (http://vx.netlux.org/lib/vsc04.html)
 * The code has been cleaned up from most likely wanted bugs that 
 * prevented it to work properly. 
 */

#define SILVIO_CESARE_ANTI_DEBUG asm volatile ( \
"        jmp antidebug1 + 2	\n"	\
" antidebug1:			\n"	\
".short 0xc606			\n"	\
"       call reloc		\n"	\
"reloc:				\n"	\
"       popl %%esi		\n"	\
"       jmp antidebug2		\n"	\
"antidebug2:			\n"	\
"       addl $(data - reloc),%%esi \n"	\
"       movl %%esi,%%edi	\n"	\
"       jmp *%%edi		\n"	\
"data:				\n"	\
"       .long 0		\n" : : : "si", "di" )

/* 
 * The following macro is a minor changed version of DISASM_MISALIGN 
 * from Secure Programming Cookbook for C and C++ (John Viega & Matt Messier)
 */ 

#define MY_SECKSY_DISALIGN asm volatile ( \
        "  pushl %eax       \n"        \
        "  xor   %eax, %eax \n"        \
        "  jz    0f         \n"        \
        "  .byte 0x0F       \n"        \
        "0:                 \n"        \
        "  popl  %eax       \n")

/* 
 * The following macro bases upon the idea of DISASM_FALSERET (John Viega & Matt Messier)
 * Unfortunately DISASM_FALSERET doesn't work properly. Use MY_SECKSY_FALSERET in place of it. 
 */ 

#define MY_SECKSY_FALSERET asm volatile (       \
	"  pushl %eax		\n"		\
	"  pushl %ebp		\n"		\
	"  lea   0f, %eax	\n"		\
	"  pushl %eax		\n"		\
	"  pushl %ebp		\n"		\
	"  movl %esp, %ebp	\n" 		\
	"  leave		\n"		\
	"  ret			\n"		\
	"  .byte 0x0F		\n"		\
	" 0:			\n"		\
	"  popl %ebp		\n"		\
	"  popl %eax		\n" )


int 
main(int argc, char *argv[])
{
	SILVIO_CESARE_ANTI_DEBUG;
	printf("hello ");

	MY_SECKSY_DISALIGN;
	printf("world");

	MY_SECKSY_FALSERET;
	printf("!\n");

	return 0;
}	
