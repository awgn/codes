/***
   Original idea :

       Federico 'simon' Simoncelli <f.simon@email.it>
       http://www.jkcal.org/simon/files/coding/dyn_caller.c

   Improved the asm code, removed the _memmove call.

   Bonelli Nicola <bonelli@antifork.org>
***/

#include <stdio.h>
#include <stdarg.h>

int 
dyn_caller(char *str, void *fun, int stack_size,...)
{
	int ret;
	va_list ap;

	va_start(ap, stack_size);

	__asm__(
		"pushl %%ecx\n"
		"pushl %%esi\n"
		"pushl %%edi\n"
		"subl %1,%%esp\n"	/* Create space for new stack */
		"movl %%esp, %%edi\n"	/* destination */
		"movl %0, %%esi\n"	/* source */
		"movl %1, %%ecx\n"	/* howmany */
		"cld\n"
		"rep\n"
		"movsl\n"
		"call *%2\n"
		"movl %%eax, %3\n"
		"addl %1,%%esp\n"	/* Restore old position of %esp */
		"popl %%edi\n"
		"popl %%esi\n"
		"popl %%ecx\n"
: :		"m"(ap), "m"(stack_size), "m"(fun), "m"(ret)
		);

	/* do whatever you want here.. */


	return ret;
}

int
main(int argc,char **argv)
{

	dyn_caller("test", printf, 2, "hello wor1d\n", 1);
	return 0;
}
