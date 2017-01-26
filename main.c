#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fec_secded7264.h"

int main(int argc, char **argv)
{
    const char orig[256] = "this is some text that is test data\n";
    uint8_t enc[256] = {0};
    uint8_t dec[256] = {0};
    unsigned int enc_len, dec_len, errors;

    // encode data
    enc_len = fec_secded7264_encode(strlen(orig), (uint8_t *)orig, enc);

    // mess it up
    enc[6] ^= 0x04;
    enc[14] ^= 0x20;
    enc[21] ^= 0x41;

    // decode it
    dec_len = fec_secded7264_decode(enc_len, enc, dec, &errors);

    printf("Orig len %u to enc len %u to dec len %u with %u errors.\n",
            (unsigned)strlen(orig), enc_len, dec_len, errors);
    printf("%s", dec);

    return 0;
}
