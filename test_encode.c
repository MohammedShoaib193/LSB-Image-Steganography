/* 
NAME: Mohammed Shoaib
DATE: 17-02-2023
DESCRIPTION: Steganography: Hiding the data in the least significant bit
 */
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

void disp(int n)				// fumction to display, how to execute the program
{
    printf("Execute the file like this\n");
    if(n)
	printf("./a.out -e beautiful.bmp secret file \n");
    else
	printf("./a.out -d stego.bmp\n");
}
int main(int argc,char **argv)
{
    if(argc<2)
    { 
	disp(1);
	return 0;
    }
    int operation=check_operation_type(argv); 
    if(operation == e_encode)
    {
	puts("----------INFO: Selected Encode-----------");
	EncodeInfo encInfo;					//  declares the structure [encode]
	if(read_and_validate_encode_args(argv,&encInfo)==e_success)	// calls the function to read & validate the arguments
	{
	    puts("---------INFO: Read and validate args was success--------------");
	    if( do_encoding (&encInfo) == e_success)		// call's the encoding function to encode
		printf("-----------INFO: Encoding SUCCESS---------\n");
	    else
		printf("------------INFO: Encoding FAILURE-----------\n");
	}
	else
	{
	    puts("----------INFO: Read and validate args was Failed--------------");
	    disp(1);
	}
    }
    else if(operation == e_decode)
    {
	puts("-----------INFO: Selected decode----------------");
	DecodeInfo decinfo;					// declare's the structure[decode]
	if(read_and_validate_decode_args(argv,&decinfo)==e_success)
	{
	    puts("----------INFO: Read and validate args was success-------------");
	    if( do_decoding (&decinfo,argv) == e_success)	// call's the decoding function
		printf("-----------INFO: Decoding SUCCESS-----------\n");
	    else
		printf("-----------INFO: Decoding FAILURE---------\n");
	}
	else
	{
	    puts("----------INFO: Read and validate args was Failed-------------");
	    disp(0);
	}
    }
    else
    {
	puts("-------------INFO: Unsupported operation type----------");
	disp(1);
    }
    return 0;
} 
OperationType check_operation_type(char **argv)
{
    if(!(strcmp(argv[1],"-e")))			// condition to check whether user wants to do encoding or decoding
	return e_encode;
    else if(!(strcmp(argv[1],"-d")))
	return e_decode;
    else
	return e_unsupported;			// if user enters either encode or decodd option, then its unsupported
}
