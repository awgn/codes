/*
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *      chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * iovec extensions by:
 *
 *	Bonelli Nicola
 *	http://awgn.antifork.org/
 *
 * Share and Enjoy!     :-)
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include <fnv.h>

static
const char cvsid[] = "$Id: fnv-ext.c,v 1.2 2004/09/17 17:05:26 awgn Exp $";

#define FNV_32_PRIME ((Fnv32_t)0x01000193)
#define FNV_64_PRIME ((Fnv64_t)0x100000001b3ULL)


Fnv32_t
fnv_32a_buf(void *buf, size_t len, Fnv32_t hval)
{
	unsigned char *bp = (unsigned char *) buf;	/* start of buffer */
	unsigned char *be = bp + len;	/* beyond end of buffer */

	/*
         * FNV-1a hash each octet in the buffer
         */
	while (bp < be) {

		/* xor the bottom with the current octet */
		hval ^= (Fnv32_t) * bp++;

		/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
		hval *= FNV_32_PRIME;
#else
		hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
	}

	/* return our new hash value */
	return hval;
}


Fnv32_t
fnv_32a_iovec(const struct iovec * vector, size_t count, Fnv32_t hval)
{
	unsigned char *bp;	/* start of block */
	unsigned char *be;	/* beyond end of buffer */
	int lcount;

	for (lcount = 0; lcount < count; lcount++) {
		bp = vector[lcount].iov_base;
		be = vector[lcount].iov_base + vector[lcount].iov_len;

		/*
	         * FNV-1a hash each octet in the buffer
	         */
		while (bp < be) {
			/* xor the bottom with the current octet */
			hval ^= (Fnv32_t) * bp++;

			/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
			hval *= FNV_32_PRIME;
#else
			hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
		}

	}

	/* return our new hash value */
	return hval;
}


Fnv64_t
fnv_64a_iovec(const struct iovec * vector, size_t count, Fnv64_t hval)
{
	unsigned char *bp;	/* start of buffer */
	unsigned char *be;	/* beyond end of buffer */
	int lcount;

	for (lcount = 0; lcount < count; lcount++) {
		bp = vector[lcount].iov_base;
		be = vector[lcount].iov_base + vector[lcount].iov_len;

		/*
	         * FNV-1a hash each octet of the buffer
	         */
		while (bp < be) {

			/* xor the bottom with the current octet */
			hval ^= (Fnv64_t) * bp++;

			/* multiply by the 64 bit FNV magic prime mod 2^64 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
			hval *= FNV_64_PRIME;
#else
			hval += (hval << 1) + (hval << 4) + (hval << 5) +
				(hval << 7) + (hval << 8) + (hval << 40);
#endif
		}
	}

	/* return our new hash value */
	return hval;
}


Fnv64_t
fnv_64a_buf(void *buf, size_t len, Fnv64_t hval)
{
	unsigned char *bp = (unsigned char *) buf;	/* start of buffer */
	unsigned char *be = bp + len;	/* beyond end of buffer */

	/*
         * FNV-1a hash each octet of the buffer
         */
	while (bp < be) {

		/* xor the bottom with the current octet */
		hval ^= (Fnv64_t) * bp++;

		/* multiply by the 64 bit FNV magic prime mod 2^64 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
		hval *= FNV_64_PRIME;
#else
		hval += (hval << 1) + (hval << 4) + (hval << 5) +
			(hval << 7) + (hval << 8) + (hval << 40);
#endif
	}

	/* return our new hash value */
	return hval;
}


Fnv32_t
fnv_32a_file(const char *fn, Fnv32_t init)
{
	unsigned char buffer[1024];
	Fnv32_t ret;
	FILE *f;
	int len;

	ret = init;

	if( (f= fopen(fn,"r")) == NULL)
		errx(1,"fopen() %s",strerror(errno));
	
	while ( (len=fread(buffer,1,sizeof(buffer),f)) > 0 )
		ret = fnv_32a_buf(buffer,len,ret);	 

	fclose(f);
	return ret;
}


Fnv64_t
fnv_64a_file(const char *fn, Fnv64_t init)
{
        unsigned char buffer[1024];
        Fnv64_t ret;
        FILE *f;
        int len;

        ret = init;

        if( (f= fopen(fn,"r")) == NULL)
                errx(1,"fopen() %s",strerror(errno));

        while ( (len=fread(buffer,1,sizeof(buffer),f)) > 0 )
                ret = fnv_64a_buf(buffer,len,ret);

        fclose(f);
        return ret;
}


#ifdef TEST_FNV
#define string_len(x)	(sizeof(x)-1)
#define msg(format,...)	fprintf(stdout,"%s=(" format ")\n" ,#__VA_ARGS__,## __VA_ARGS__)

int
main(int argc, char **argv)
{
	struct iovec io_test[2];

	char buff_a[] = "1234567890";
	char buff_b[] = "qwerty";
	char buff_c[] = "1234567890qwerty";

	putchar('\n');

	msg("%s", buff_a);
	msg("%s", buff_b);
	msg("%s", buff_c);

	io_test[0].iov_base = buff_a;
	io_test[1].iov_base = buff_b;
	io_test[0].iov_len = string_len(buff_a);
	io_test[1].iov_len = string_len(buff_b);

	msg("%s %d", (char *)io_test[0].iov_base, io_test[0].iov_len);
	msg("%s %d", (char *)io_test[1].iov_base, io_test[1].iov_len);

        putchar('\n');

	msg("0x%x", (unsigned int)fnv_32a_buf(buff_a, string_len(buff_a), FNV1_32_INIT));
	msg("0x%x", (unsigned int)fnv_32a_buf(buff_b, string_len(buff_b), FNV1_32_INIT));
	msg("0x%x", (unsigned int)fnv_32a_buf(buff_c, string_len(buff_c), FNV1_32_INIT));
	msg("0x%x", (unsigned int)fnv_32a_iovec(io_test, 2, FNV1_32_INIT));

	msg("0x%llx", fnv_64a_buf(buff_a, string_len(buff_a), FNV1_64_INIT));
	msg("0x%llx", fnv_64a_buf(buff_b, string_len(buff_b), FNV1_64_INIT));
	msg("0x%llx", fnv_64a_buf(buff_c, string_len(buff_c), FNV1_64_INIT));

	msg("0x%llx", fnv_64a_iovec(io_test, 2, FNV1_64_INIT));

        return 0;
}
#endif


