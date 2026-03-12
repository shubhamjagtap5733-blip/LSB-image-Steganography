#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */
typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname; // To store the src image name
    FILE *fptr_src_image;  // To store the address of the src image
    char magic_string[3]; //To store the magic string
    /* Secret File Info */
    char secret_fname[20];       // To store the output file name
    FILE *fptr_secret;        // To store the output file address
    uint extn_size;
    char extn_secret_file[5]; // To store the output file extension
    uint size_secret_file;    // To store the size of the secret data


        

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *dncInfo); 

/* Get File pointer for i/p file */
Status open_src_file(DecodeInfo *dncInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *dncInfo);

/* Decoded Magic String */
Status decode_magic_string(DecodeInfo *dncInfo);

/* Decode a byte from LSB of image data array */
Status decode_byte_from_lsb(unsigned char *data, char *image_buffer);

// Decode a size from lsb
Status decode_size_to_lsb(unsigned int *size, char *image_buffer);

/* Decode output file extention size */
Status decode_secret_file_extn_size(DecodeInfo *dncInfo);

/* Encode secret file extention */
Status decode_secret_file_extn(DecodeInfo *dncInfo);

/* Open output file*/
Status open_secret_file(DecodeInfo *dncInfo);

/* Encode secret file size */
Status decode_secret_file_size(DecodeInfo *dncInfo);

/* decode secret file data*/
Status decode_secret_data(DecodeInfo *dncInfo);
/* Close the file pointers*/
void close_file_pointers_decoding(DecodeInfo *dncInfo);
#endif