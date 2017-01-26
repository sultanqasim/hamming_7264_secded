/*
 * Copyright (c) 2017 Sultan Qasim Khan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fec_secded7264.h"

int main(int argc, char **argv)
{
    const char orig[256] = "this is some text that is test data\n";
    uint8_t enc[256] = {0};
    uint8_t lazy[256] = {0};
    uint8_t dec[256] = {0};
    unsigned int enc_len, dec_len, errors;

    // encode data
    enc_len = fec_secded7264_encode(strlen(orig), (uint8_t *)orig, enc);

    // mess it up
    enc[6] ^= 0x04;
    enc[14] ^= 0x10;
    enc[21] ^= 0x41;

    // decode it
    dec_len = fec_secded7264_decode_lazy(enc_len, enc, lazy);
    dec_len = fec_secded7264_decode(enc_len, enc, dec, &errors);

    printf("Orig len %u to enc len %u to dec len %u with %u errors.\n",
            (unsigned)strlen(orig), enc_len, dec_len, errors);
    printf("original: %s", orig);
    printf("corrupt:  %s", lazy);
    printf("decoded:  %s", dec);

    return 0;
}
