#ifndef DECODE_H
#define DECODE_H

#include "types.h"  // Contains user defined types

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Secret File Info */
    char secret_fname[100];;
    FILE *fptr_secret;
    long secret_file_extn_size;
    long secret_file_size;

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decodeInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decodeInfo);

/* Get File pointer for i/p file */
Status open_image_file(DecodeInfo *decodeInfo);

/* Skip bmp image header */
Status skip_bmp_header(FILE *fptr_src_image);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decodeInfo);

/* Decode secret file size */
Status decode_secret_file_extn_size(DecodeInfo *decodeInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decodeInfo);

/* Get File pointer for the secret file */
Status open_secret_file(DecodeInfo *decodeInfo); 

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decodeInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decodeInfo);

/* Decode a byte from LSB of image data array */
char decode_byte_from_lsb(char *image_buffer);

/* Decode a int from LSB of image data array */
long decode_int_from_lsb(char *image_buffer);

/* Close all files */
Status close_decode_files(DecodeInfo *decodeInfo);

#endif