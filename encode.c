#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
// Function to read and validate the command line arguments
Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
    if( (argv[2] != NULL) && (strstr(argv[2],".bmp")))		// checking the 3rd argument is bitmap file or not
    {
	encInfo -> src_image_fname=argv[2];			// stores the bitmap file name
	if( (argv[3] != NULL) && (strchr(argv[3],'.')))		// validation secret file
	{
	    strcpy(encInfo ->extn_secret_file,strchr(argv[3],'.')); //secret file should have the extension
	    encInfo -> secret_fname=argv[3];			// stores the secret file name
	}
	else
	    return e_failure;
	if( (argv[4] != NULL) && (strstr(argv[4],".bmp")))	// checking, if the user has given the output file name
	{
	    encInfo -> stego_image_fname = argv[4];
	}
	else if( argv[4] == NULL)
	{
	    encInfo -> stego_image_fname = "stego.bmp";	// creat's the default output image,if the user not passed the output image name
	    return e_success;
	}
	else if( !(strstr(argv[4],".bmp")))
	    return e_failure;
	return e_success;
    }
    else
    {
	puts("ERROR: file should be BMP file\n");
	return e_failure;
    }
}
// function to check the capacity of the input bitmap file
Status check_capacity( EncodeInfo *encInfo)
{
    // store the input bitmap size
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);	// store's the size of the secret file

    //the input bitmap file size should be greater then , size of header + len of magic string + size of secret file extension + length of secret file extension + length of secret file name + size of secet file
    if( encInfo->image_capacity > (54+strlen(MAGIC_STRING)+4+4+4+(encInfo -> size_secret_file) *8) )
    {
	return e_success;
    }
    else
	return e_failure;
}
// function to get the size of the file
uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

// Function to copy the header from input file to output bitmap file
Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char buffer[54];
    rewind(fptr_src_image);		// makes the file indicator to indicate to the beginning og the file
    fread(buffer,54,1,fptr_src_image);	// reads the 54 bytes of the data from the source file
    fwrite(buffer,54,1,fptr_dest_image); // write the 54 byte data to the output file
    return e_success;
}

// function to encode the magic string
Status encode_magic_string(char* magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string, strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

//function to encode the data
Status encode_data_to_image(char *data,int size, FILE *fptr_src_image,FILE *fptr_stego_image)
{
    char buffer[8];			// buffer of 8 bytes, bcz to encode 1byte data, we need 8bytes of data
    for(int i=0;i<size;i++)
    {
	fread(buffer,8,1,fptr_src_image);	// reads the 8byte data from the input bitmap file
	encode_byte_to_lsb(data[i],buffer);	// function call to encode data into lsb
	fwrite(buffer,8,1,fptr_stego_image);	// write the encode data into the output file
    }
    return e_success;
}
// Function to encode 1 byte data to 8 byte data by lsb encode method
Status encode_byte_to_lsb(char data,char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
	// reads the 1 byte data from the buffer and unset the lsb bit and do OR operation with bit's of secret data 8times
	image_buffer[i]=((image_buffer[i]&0xfe) | (data & (1<<i))>>i);
    }
    return e_success;
}

// Function to encode secret file extension size
Status encode_extension_size(int size,FILE *fptr_src_image,FILE *fptr_stego_image)
{
    char buffer[32];
    fread(buffer,32,1,fptr_src_image);	// reads 32 bytes as the size is in integer, which is 4byte
    encode_size_to_lsb(buffer,size);	// calls the function to encode 1 bit data to 1byte data
    fwrite(buffer,32,1,fptr_stego_image); // writes the encoded bytes to the output file
    return e_success;
}
// Function to encode the size of the files
Status encode_size_to_lsb(char *image_buffer,int data)
{
    for(int i=0;i<32;i++)
    {
	// reads the 1 byte data from the buffer and unset the lsb bit and do OR operation with bit's of secret data 8times
	image_buffer[i]=((image_buffer[i]&0xfe) | (data & (1<<i))>>i);
    }
}
// function to encode the secret file extension
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    // call's the encode data to image function to encode the secret file exten
    if( (encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image,encInfo->fptr_stego_image)) == e_success)
	return e_success;
    else
	return e_failure;
}
// function to encode the secret file size
Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);	// reads the 32 bytes from the input size as the size is in integer
    encode_size_to_lsb(buffer,file_size);	// function to encode  1bit to the 1byte data
    fwrite(buffer,32,1,encInfo->fptr_stego_image);	// writes the encoded data to the output file
    return e_success;
}
// function to encode the secret file data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_secret,0,SEEK_SET);		// set's the secret file indicator to the beginning of the file
    char buffer[encInfo->size_secret_file];
    fread(buffer,encInfo->size_secret_file,1,encInfo->fptr_secret);	// reads the secret file data
    if( (encode_data_to_image(buffer,encInfo->size_secret_file, encInfo->fptr_src_image,encInfo->fptr_stego_image)) == e_success)
	return e_success;
    else
	return e_failure;
}
// function to copy the remaining data to the output file
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src)>0)	// reads the bytes from the source file till End Of File
	fwrite(&ch,1,1,fptr_dest);	// write the readed data to the ouptut file
    return e_success;

}
// function for the Encoding process
Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Opening the Required files\n");
    if( open_files(encInfo) == e_success)	// checking the required files are opened or not
    {
	puts("INFO: Done");
	printf("INFO: checking the capacity\n");
	if( check_capacity(encInfo) == e_success )	// checks for the image capacity to encode
	{
	    puts("INFO: Done");
	    printf("INFO: Copying the header file data\n");
	    if( copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success) // chech for status of copying the header data
	    {
		puts("INFO: Done");
		printf("INFO: Encoding the magic string\n");
		if( encode_magic_string( MAGIC_STRING, encInfo) == e_success) // check for the status of the encoding of the magic string
		{
		    puts("INFO: Done");
		    printf("INFO: Encoding the secret file extension size\n");
		    if( encode_extension_size(strlen(strstr(encInfo->secret_fname,".")),encInfo->fptr_src_image,encInfo->fptr_stego_image)== e_success)	// checks status of the secret file exten size encoding
		    {
			puts("INFO: Done");
			puts("INFO: Encoding the secret file extension");

			if( encode_secret_file_extn((strstr(encInfo->extn_secret_file,".")),encInfo) == e_success)	// checks status of the secret file exten encoding
			{
			    puts("INFO: Done");
			    puts("INFO: Encoding the size of the secret file");
			    // checks status of the secret file size encoding
			    if( encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
			    {
				puts("INFO: Done");
				puts("INFO: Encoding the secret data");
				// checks status of the secret data encoding
				if (( encode_secret_file_data(encInfo)) == e_success)
				{
				    puts("INFO: Done");
				    puts("INFO: Copying the remaining data");
				    // checks status of the copying the ramaining data function
				    if ( copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
				    {
					puts("INFO: Done");
					return e_success;
				    }
				    else
				    {
					puts("ERROR: copying the remaining data is failed");
					return e_failure;
				    }
				}
				else
				{
				    puts("ERROR: Encoding the secret data is failed");
				    return e_failure;
				}
			    }
			    else
			    {
				puts("ERROR: Encoding the secret file size is failed");
				return e_failure;
			    }
			}
			else
			{
			    puts("ERROR: Encoding the secret file extension is failed");
			    return e_failure;
			}
		    }
		    else
		    {
			puts("ERROR: Encoding the secret file extension size is failed");
			return e_failure;
		    }
		}
		else
		{
		    puts("ERROR: Encoding the Magic string is failed");
		    return e_failure;
		}

	    }
	    else
	    {
		puts("ERROR: Copying of the header file data is failed");
		return e_failure;
	    }
	}
	else
	{
	    puts("ERROR: Checking the capacity is failed");
	    return e_failure;
	}
    }
    else
    {
	puts("ERROR: failed to open the required files");
	return e_failure;
    }
}
