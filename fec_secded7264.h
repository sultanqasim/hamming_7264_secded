#ifndef FEC7264_H
#define FEC7264_H

unsigned int fec_secded7264_encode(unsigned int _dec_msg_len,
                                   const unsigned char *_msg_dec,
                                   unsigned char *_msg_enc);

unsigned int fec_secded7264_decode(unsigned int dec_msg_len,
                                   const unsigned char *msg_enc,
                                   unsigned char *msg_dec);

#endif
