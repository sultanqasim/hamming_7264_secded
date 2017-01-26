#ifndef LIQUID_INTERNAL_H
#define LIQUID_INTERNAL_H

// available FEC schemes
#define LIQUID_FEC_NUM_SCHEMES  28
typedef enum {
    LIQUID_FEC_UNKNOWN=0,       // unknown/unsupported scheme
    LIQUID_FEC_NONE,            // no error-correction
    LIQUID_FEC_REP3,            // simple repeat code, r1/3
    LIQUID_FEC_REP5,            // simple repeat code, r1/5
    LIQUID_FEC_HAMMING74,       // Hamming (7,4) block code, r1/2 (really 4/7)
    LIQUID_FEC_HAMMING84,       // Hamming (7,4) with extra parity bit, r1/2
    LIQUID_FEC_HAMMING128,      // Hamming (12,8) block code, r2/3
    
    LIQUID_FEC_GOLAY2412,       // Golay (24,12) block code, r1/2
    LIQUID_FEC_SECDED2216,      // SEC-DED (22,16) block code, r8/11
    LIQUID_FEC_SECDED3932,      // SEC-DED (39,32) block code
    LIQUID_FEC_SECDED7264,      // SEC-DED (72,64) block code, r8/9

    // codecs not defined internally (see http://www.ka9q.net/code/fec/)
    LIQUID_FEC_CONV_V27,        // r1/2, K=7, dfree=10
    LIQUID_FEC_CONV_V29,        // r1/2, K=9, dfree=12
    LIQUID_FEC_CONV_V39,        // r1/3, K=9, dfree=18
    LIQUID_FEC_CONV_V615,       // r1/6, K=15, dfree<=57 (Heller 1968)

    // punctured (perforated) codes
    LIQUID_FEC_CONV_V27P23,     // r2/3, K=7, dfree=6
    LIQUID_FEC_CONV_V27P34,     // r3/4, K=7, dfree=5
    LIQUID_FEC_CONV_V27P45,     // r4/5, K=7, dfree=4
    LIQUID_FEC_CONV_V27P56,     // r5/6, K=7, dfree=4
    LIQUID_FEC_CONV_V27P67,     // r6/7, K=7, dfree=3
    LIQUID_FEC_CONV_V27P78,     // r7/8, K=7, dfree=3

    LIQUID_FEC_CONV_V29P23,     // r2/3, K=9, dfree=7
    LIQUID_FEC_CONV_V29P34,     // r3/4, K=9, dfree=6
    LIQUID_FEC_CONV_V29P45,     // r4/5, K=9, dfree=5
    LIQUID_FEC_CONV_V29P56,     // r5/6, K=9, dfree=5
    LIQUID_FEC_CONV_V29P67,     // r6/7, K=9, dfree=4
    LIQUID_FEC_CONV_V29P78,     // r7/8, K=9, dfree=4

    // Reed-Solomon codes
    LIQUID_FEC_RS_M8            // m=8, n=255, k=223
} fec_scheme;

// fec object (pointer to fec structure)
typedef struct fec_s * fec;

// fec : basic object
struct fec_s {
    // common
    fec_scheme scheme;
    //unsigned int dec_msg_len;
    //unsigned int enc_msg_len;
    float rate;

    // lengths: convolutional, Reed-Solomon
    unsigned int num_dec_bytes;
    unsigned int num_enc_bytes;

    // convolutional : internal memory structure
    unsigned char * enc_bits;
    void * vp;      // decoder object
    int * poly;     // polynomial
    unsigned int R; // primitive rate, inverted (e.g. R=3 for 1/3)
    unsigned int K; // constraint length
    unsigned int P; // puncturing rate (e.g. p=3 for 3/4)
    int * puncturing_matrix;

    // viterbi decoder function pointers
    void*(*create_viterbi)(int);
    //void (*set_viterbi_polynomial)(int*);
    int  (*init_viterbi)(void*,int);
    int  (*update_viterbi_blk)(void*,unsigned char*,int);
    int  (*chainback_viterbi)(void*,unsigned char*,unsigned int,unsigned int);
    void (*delete_viterbi)(void*);

    // Reed-Solomon
    int symsize;    // symbol size (bits per symbol)
    int genpoly;    // generator polynomial
    int fcs;        //
    int prim;       //
    int nroots;     // number of roots in the polynomial
    //int ntrials;    //
    unsigned int rspad; // number of implicit padded symbols
    int nn;         // 2^symsize - 1
    int kk;         // nn - nroots
    void * rs;      // Reed-Solomon internal object

    // Reed-Solomon decoder
    unsigned int num_blocks;    // number of blocks: ceil(dec_msg_len / nn)
    unsigned int dec_block_len; // number of decoded bytes per block: 
    unsigned int enc_block_len; // number of encoded bytes per block: 
    unsigned int res_block_len; // residual bytes in last block
    unsigned int pad;           // padding for each block
    unsigned char * tblock;     // decoder input sequence [size: 1 x n]
    int * errlocs;              // error locations [size: 1 x n]
    int * derrlocs;             // decoded error locations [size: 1 x n]
    int erasures;               // number of erasures

    // encode function pointer
    void (*encode_func)(fec _q,
                        unsigned int _dec_msg_len,
                        unsigned char * _msg_dec,
                        unsigned char * _msg_enc);

    // decode function pointer
    void (*decode_func)(fec _q,
                        unsigned int _dec_msg_len,
                        unsigned char * _msg_enc,
                        unsigned char * _msg_dec);

    // decode function pointer (soft decision)
    void (*decode_soft_func)(fec _q,
                             unsigned int _dec_msg_len,
                             unsigned char * _msg_enc,
                             unsigned char * _msg_dec);
};

// get the theoretical rate of a particular forward error-
// correction scheme (object-independent method)
static float fec_get_rate(fec_scheme _scheme)
{
    switch (_scheme) {
    case LIQUID_FEC_UNKNOWN:        return 0;
    case LIQUID_FEC_NONE:           return 1.;
    case LIQUID_FEC_REP3:           return 1./3.;
    case LIQUID_FEC_REP5:           return 1./5.;
    case LIQUID_FEC_HAMMING74:      return 4./7.;
    case LIQUID_FEC_HAMMING84:      return 4./8.;
    case LIQUID_FEC_HAMMING128:     return 8./12.;
    case LIQUID_FEC_GOLAY2412:      return 1./2.;
    case LIQUID_FEC_SECDED2216:     return 2./3.;   // ultimately 16/22 ~ 0.72727
    case LIQUID_FEC_SECDED3932:     return 4./5.;   // ultimately 32/39 ~ 0.82051
    case LIQUID_FEC_SECDED7264:     return 8./9.;

    // convolutional codes
#if LIBFEC_ENABLED
    case LIQUID_FEC_CONV_V27:       return 1./2.;
    case LIQUID_FEC_CONV_V29:       return 1./2.;
    case LIQUID_FEC_CONV_V39:       return 1./3.;
    case LIQUID_FEC_CONV_V615:      return 1./6.;
    case LIQUID_FEC_CONV_V27P23:    return 2./3.;
    case LIQUID_FEC_CONV_V27P34:    return 3./4.;
    case LIQUID_FEC_CONV_V27P45:    return 4./5.;
    case LIQUID_FEC_CONV_V27P56:    return 5./6.;
    case LIQUID_FEC_CONV_V27P67:    return 6./7.;
    case LIQUID_FEC_CONV_V27P78:    return 7./8.;
    case LIQUID_FEC_CONV_V29P23:    return 2./3.;
    case LIQUID_FEC_CONV_V29P34:    return 3./4.;
    case LIQUID_FEC_CONV_V29P45:    return 4./5.;
    case LIQUID_FEC_CONV_V29P56:    return 5./6.;
    case LIQUID_FEC_CONV_V29P67:    return 6./7.;
    case LIQUID_FEC_CONV_V29P78:    return 7./8.;

    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:          return 223./255.;
#else
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:

    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        fprintf(stderr,"error: fec_get_rate(), convolutional codes unavailable (install libfec)\n");
        exit(-1);

    case LIQUID_FEC_RS_M8:
        fprintf(stderr,"error: fec_get_rate(), Reed-Solomon codes unavailable (install libfec)\n");
        exit(-1);
#endif

    default:
        printf("error: fec_get_rate(), unknown/unsupported scheme: %d\n", _scheme);
        exit(-1);
    }
    return 0;
}

// compute encoded message length for block codes
//  _dec_msg_len    :   decoded message length (bytes)
//  _m              :   input block size (bits)
//  _k              :   output block size (bits)
unsigned int fec_block_get_enc_msg_len(unsigned int _dec_msg_len,
                                       unsigned int _m,
                                       unsigned int _k)
{
    // validate input
    if (_m == 0) {
        fprintf(stderr,"fec_block_get_enc_msg_len(), input block size cannot be zero\n");
        exit(1);
    } else if (_k < _m) {
        fprintf(stderr,"fec_block_get_enc_msg_len(), output block size cannot be smaller than input\n");
        exit(1);
    }

    // compute total number of bits in decoded message
    unsigned int num_bits_in = _dec_msg_len*8;

    // compute total number of blocks: ceil(num_bits_in/_m)
    unsigned int num_blocks = num_bits_in / _m + (num_bits_in%_m ? 1 : 0);

    // compute total number of bits out
    unsigned int num_bits_out = num_blocks * _k;

    // compute total number of bytes out: ceil(num_bits_out/8)
    unsigned int num_bytes_out = num_bits_out/8 + (num_bits_out%8 ? 1 : 0);
#if 0
    printf("fec_block_get_enc_msg_len(%u,%u,%u)\n", _dec_msg_len, _m, _k);
    printf("    dec msg len :   %u bytes\n", _dec_msg_len);
    printf("    m           :   %u bits\n", _m);
    printf("    k           :   %u bits\n", _k);
    printf("    num bits in :   %u bits\n", num_bits_in);
    printf("    num blocks  :   %u\n", num_blocks);
    printf("    num bits out:   %u bits\n", num_bits_out);
    printf("    enc msg len :   %u bytes\n", num_bytes_out);
#endif
    return num_bytes_out;
}

// return the encoded message length using a particular error-
// correction scheme (object-independent method)
//  _scheme     :   forward error-correction scheme
//  _msg_len    :   raw, uncoded message length
unsigned int fec_get_enc_msg_length(fec_scheme _scheme,
                                    unsigned int _msg_len)
{
    switch (_scheme) {
    case LIQUID_FEC_UNKNOWN:        return 0;
    case LIQUID_FEC_NONE:           return _msg_len;
    case LIQUID_FEC_REP3:           return 3*_msg_len;
    case LIQUID_FEC_REP5:           return 5*_msg_len;
    case LIQUID_FEC_HAMMING74:      return fec_block_get_enc_msg_len(_msg_len,4,7);
    case LIQUID_FEC_HAMMING84:      return fec_block_get_enc_msg_len(_msg_len,4,8);
    case LIQUID_FEC_HAMMING128:     return fec_block_get_enc_msg_len(_msg_len,8,12);
    case LIQUID_FEC_GOLAY2412:      return fec_block_get_enc_msg_len(_msg_len,12,24);
    case LIQUID_FEC_SECDED2216:     return _msg_len + _msg_len/2 + ((_msg_len%2) ? 1 : 0);
    case LIQUID_FEC_SECDED3932:     return _msg_len + _msg_len/4 + ((_msg_len%4) ? 1 : 0);
    case LIQUID_FEC_SECDED7264:     return _msg_len + _msg_len/8 + ((_msg_len%8) ? 1 : 0);

#if LIBFEC_ENABLED
    // convolutional codes
    case LIQUID_FEC_CONV_V27:       return 2*_msg_len + 2;  // (K-1)/r=12, round up to 2 bytes
    case LIQUID_FEC_CONV_V29:       return 2*_msg_len + 2;  // (K-1)/r=16, 2 bytes
    case LIQUID_FEC_CONV_V39:       return 3*_msg_len + 3;  // (K-1)/r=24, 3 bytes
    case LIQUID_FEC_CONV_V615:      return 6*_msg_len + 11; // (K-1)/r=84, round up to 11 bytes
    case LIQUID_FEC_CONV_V27P23:    return fec_conv_get_enc_msg_len(_msg_len,7,2);
    case LIQUID_FEC_CONV_V27P34:    return fec_conv_get_enc_msg_len(_msg_len,7,3);
    case LIQUID_FEC_CONV_V27P45:    return fec_conv_get_enc_msg_len(_msg_len,7,4);
    case LIQUID_FEC_CONV_V27P56:    return fec_conv_get_enc_msg_len(_msg_len,7,5);
    case LIQUID_FEC_CONV_V27P67:    return fec_conv_get_enc_msg_len(_msg_len,7,6);
    case LIQUID_FEC_CONV_V27P78:    return fec_conv_get_enc_msg_len(_msg_len,7,7);

    case LIQUID_FEC_CONV_V29P23:    return fec_conv_get_enc_msg_len(_msg_len,9,2);
    case LIQUID_FEC_CONV_V29P34:    return fec_conv_get_enc_msg_len(_msg_len,9,3);
    case LIQUID_FEC_CONV_V29P45:    return fec_conv_get_enc_msg_len(_msg_len,9,4);
    case LIQUID_FEC_CONV_V29P56:    return fec_conv_get_enc_msg_len(_msg_len,9,5);
    case LIQUID_FEC_CONV_V29P67:    return fec_conv_get_enc_msg_len(_msg_len,9,6);
    case LIQUID_FEC_CONV_V29P78:    return fec_conv_get_enc_msg_len(_msg_len,9,7);

    // Reed-Solomon codes
    case LIQUID_FEC_RS_M8:          return fec_rs_get_enc_msg_len(_msg_len,32,255,223);
#else
    case LIQUID_FEC_CONV_V27:
    case LIQUID_FEC_CONV_V29:
    case LIQUID_FEC_CONV_V39:
    case LIQUID_FEC_CONV_V615:

    case LIQUID_FEC_CONV_V27P23:
    case LIQUID_FEC_CONV_V27P34:
    case LIQUID_FEC_CONV_V27P45:
    case LIQUID_FEC_CONV_V27P56:
    case LIQUID_FEC_CONV_V27P67:
    case LIQUID_FEC_CONV_V27P78:

    case LIQUID_FEC_CONV_V29P23:
    case LIQUID_FEC_CONV_V29P34:
    case LIQUID_FEC_CONV_V29P45:
    case LIQUID_FEC_CONV_V29P56:
    case LIQUID_FEC_CONV_V29P67:
    case LIQUID_FEC_CONV_V29P78:
        fprintf(stderr, "error: fec_get_enc_msg_length(), convolutional codes unavailable (install libfec)\n");
        exit(-1);

    case LIQUID_FEC_RS_M8:
        fprintf(stderr, "error: fec_get_enc_msg_length(), Reed-Solomon codes unavailable (install libfec)\n");
        exit(-1);
#endif
    default:
        printf("error: fec_get_enc_msg_length(), unknown/unsupported scheme: %d\n", _scheme);
        exit(-1);
    }

    return 0;
}

// number of ones in a byte
//  0   0000 0000   :   0
//  1   0000 0001   :   1
//  2   0000 0010   :   1
//  3   0000 0011   :   2
//  4   0000 0100   :   1
//  ...
//  126 0111 1110   :   6
//  127 0111 1111   :   7
//  128 1000 0000   :   1
//  129 1000 0001   :   2
//  ...
//  253 1111 1101   :   7
//  254 1111 1110   :   7
//  255 1111 1111   :   8
extern const unsigned char liquid_c_ones[256];

// Count the number of ones in an integer, inline insertion
#define liquid_count_ones_uint16(x) (           \
    liquid_c_ones[  (x)      & 0xff ] +         \
    liquid_c_ones[ ((x)>>8)  & 0xff ])

#define liquid_count_ones_uint24(x) (           \
    liquid_c_ones[  (x)      & 0xff ] +         \
    liquid_c_ones[ ((x)>> 8) & 0xff ] +         \
    liquid_c_ones[ ((x)>>16) & 0xff ])

#define liquid_count_ones_uint32(x) (           \
    liquid_c_ones[  (x)      & 0xff ] +         \
    liquid_c_ones[ ((x)>> 8) & 0xff ] +         \
    liquid_c_ones[ ((x)>>16) & 0xff ] +         \
liquid_c_ones[ ((x)>>24) & 0xff ])

// SEC-DED (72,64)

// compute parity byte on 64-byte input
unsigned char fec_secded7264_compute_parity(unsigned char * _v);

// compute syndrome on 72-bit input
unsigned char fec_secded7264_compute_syndrome(unsigned char * _v);

// encode symbol
//  _sym_dec    :   input symbol [size: 8 x 1]
//  _sym_enc    :   input symbol [size: 9 x 1]
void fec_secded7264_encode_symbol(unsigned char * _sym_dec,
                                  unsigned char * _sym_enc);

// estimate error vector, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   encoded symbol [size: 9 x 1]
//  _e_hat      :   estimated error vector [size: 9 x 1]
int fec_secded7264_estimate_ehat(unsigned char * _sym_enc,
                                 unsigned char * _e_hat);

// decode symbol, returning 0/1/2 for zero/one/multiple errors detected
//  _sym_enc    :   input symbol [size: 8 x 1]
//  _sym_dec    :   input symbol [size: 9 x 1]
int fec_secded7264_decode_symbol(unsigned char * _sym_enc,
								 unsigned char * _sym_dec);


extern unsigned char secded7264_P[64];
extern unsigned char secded7264_syndrome_w1[72];

fec fec_secded7264_create(void *_opts);
void fec_secded7264_destroy(fec _q);
void fec_secded7264_print(fec _q);
void fec_secded7264_encode(fec _q,
                           unsigned int _dec_msg_len,
                           unsigned char * _msg_dec,
                           unsigned char * _msg_enc);
void fec_secded7264_decode(fec _q,
                           unsigned int _dec_msg_len,
                           unsigned char * _msg_enc,
                           unsigned char * _msg_dec);

#endif
