#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    int size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;

/* read and validate decode args from argv */
Status read_and_validate_decode_args(char *[],DecodeInfo *);

/* start to decode */
Status do_decoding(DecodeInfo *,char **);

/* Get the file pointer for i/p files*/
Status decode_open_file(DecodeInfo *);

/* decode magic string */
Status decode_magic_string(char *,DecodeInfo *);

/* decode secret file extension size*/
Status decode_secret_file_extn_size(DecodeInfo *,int *);

/* decode the file size */
Status decode_size(char *,int *);

/* decode the secret file size */
Status decode_secret_file_size(DecodeInfo *);

/* decode the secret file extension */
Status decode_sec_file_extn(FILE *,int,char *);

/* decode the data from the lsb */
Status decode_data_from_lsb(char *,char *);

/* decode the secret data from the .bmp file */
Status decode_secret_data(DecodeInfo *);

#endif
