#ifndef FEC7264_H
#define FEC7264_H

/* fec_secded7264_encode
 *
 * Inputs:
 * dec_msg_len  size of original data
 * msg_dec      original message data
 *
 * Outputs:
 * msg_enc      encoded message data gets put here
 *
 * Returns:
 * Size of encoded message in bytes
 */
unsigned int fec_secded7264_encode(unsigned int dec_msg_len,
                                   const unsigned char *msg_dec,
                                   unsigned char *msg_enc);

/* fec_secded7264_decode
 *
 * Inputs:
 * dec_msg_len  size of encoded data
 * msg_enc      encoded message data
 *
 * Outputs:
 * msg_dec      decoded original mesasage data gets put here
 * num_errors   number of unrecoverable errors encountered in decoding
 *              (can be NULL)
 *
 * Returns:
 * Size of decoded message in bytes
 */
unsigned int fec_secded7264_decode(unsigned int enc_msg_len,
                                   const unsigned char *msg_enc,
                                   unsigned char *msg_dec,
                                   unsigned int *num_errors);

#endif
