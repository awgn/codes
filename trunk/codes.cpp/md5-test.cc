/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <vector>
#include <fstream>
#include <md5.hh>

/* 
 * RFC 1321 test vectors
 */
static const char *md5_test_str[7] =
{
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012" \
    "345678901234567890"
};

static uint8_t md5_test_sum[7][16] =
{
    { 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
      0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E },
    { 0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
      0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61 },
    { 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
      0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 },
    { 0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
      0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0 },
    { 0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
      0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B },
    { 0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
      0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F },
    { 0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
      0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A }
};

/*
 * Checkup routine
 */
int md5_self_test()
{
    std::cout << std::hex;

    // cipher test:
    for(int i = 0; i < 7; i++ )
    {
        std::cout << "  MD5++ cipher test #" << i+1 << ": ";
        more::md5 ctx;
        ctx( (uint8_t *)md5_test_str[i],  strlen( md5_test_str[i] ) );
        ctx.finish();

        if( memcmp( ctx.digest_begin(), md5_test_sum[i], 16 ) != 0 )
        {
            std::cout << "failed" << std::endl;
            return 1;
        }

        std::cout << "passed" << std::endl;
    }

    // cipher test iterators:
    for(int i = 0; i < 7; i++ )
    {
        std::cout << "  MD5++ cipher test %" << i+1 << ": ";
        more::md5 ctx;
        ctx( md5_test_str[i],  md5_test_str[i] + strlen( md5_test_str[i] ) );
        ctx.finish();

        if( memcmp( ctx.digest_begin(), md5_test_sum[i], 16 ) != 0 )
        {
            std::cout << "iterator test failed" << std::endl;
            return 1;
        }

        std::cout << "iterator test passed" << std::endl;
    }

    // cipher test iterators (2):
    for(int i = 0; i < 7; i++ )
    {
        std::vector<uint8_t> vec(md5_test_str[i],md5_test_str[i]+strlen( md5_test_str[i]));

        std::cout << "  MD5++ cipher test ^" << i+1 << ": ";
        more::md5 ctx;
        
        ctx(vec);
        ctx.finish();

        if( memcmp( ctx.digest_begin(), md5_test_sum[i], 16 ) != 0 )
        {
            std::cout << "iterator test failed" << std::endl;
            return 1;
        }

        std::cout << "iterator test passed" << std::endl;
    }


    // STL alogorithm test:
    {
        std::cout << "  STL algorithm test: ";
        std::vector<uint8_t> vec;

        vec.push_back('a');
        vec.push_back('b');
        vec.push_back('c');

        more::md5 cipher;
        cipher = std::copy(vec.begin(), vec.end(), cipher );
        cipher.finish();

        std::cout << std::hex;
        std::copy( cipher.digest_begin(), cipher.digest_end(), std::ostream_iterator<uint32_t>(std::cout, " "));
        if ( std::equal(cipher.digest_begin(), cipher.digest_end(),  md5_test_sum[2]) )
            std::cout << ": passed";
        else
            std::cout << ": failed";

        std::cout << std::endl;
        std::cout << "          digest_str: " << cipher.digest_str() << std::endl;
    }

    // stream test:
    {
        std::cout << "     stream test (1): ";

        more::md5 cipher;
        cipher << "abc";
        cipher.finish();

        std::cout << std::hex;
        std::copy( cipher.digest_begin(), cipher.digest_end(), std::ostream_iterator<uint32_t>(std::cout, " "));
        if ( std::equal(cipher.digest_begin(), cipher.digest_end(),  md5_test_sum[2]) )
            std::cout << ": passed";
        else
            std::cout << ": failed";

        std::cout << std::endl;
    }
    // stream test(2):
    {
        std::cout << "     stream test (2): ";

        std::stringstream s;
        s << "abc";
        
        more::md5 cipher;
        cipher << s.rdbuf();    // stream the whole rdbuf() into the cipher
        cipher.finish();

        std::cout << std::hex;
        std::copy( cipher.digest_begin(), cipher.digest_end(), std::ostream_iterator<uint32_t>(std::cout, " "));
        if ( std::equal(cipher.digest_begin(), cipher.digest_end(),  md5_test_sum[2]) )
            std::cout << ": passed";
        else
            std::cout << ": failed";

        std::cout << std::endl;
    }


    // md5_adaptor test (csum functor):
    {
        std::cout << "  mdp5_adaptor  test: ";
        std::vector<uint8_t> vec;
        vec.push_back('a');
        vec.push_back('b');
        vec.push_back('c');

        more::md5_adaptor<std::vector<uint8_t> > csum_functor;
        std::vector<uint8_t> dig = csum_functor(vec.begin(), vec.end());

        std::cout << std::hex;
        std::copy( dig.begin(), dig.end(), std::ostream_iterator<uint32_t>(std::cout, " "));

        if ( std::equal(dig.begin(), dig.end(),  md5_test_sum[2]) )
            std::cout << ": passed";
        else
            std::cout << ": failed";
        std::cout << std::endl;

    }

    return 0;
}

int
main(int, char *[])
{
    md5_self_test();
    return 0;
}

