#include <stdio.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "decode.h"

int i,j;
// Function to open the required file's
Status decode_open_file(DecodeInfo *decinfo)
{
    // open's the input file and makes stego file indicator to point to that file
    decinfo->fptr_stego_image=fopen(decinfo->stego_image_fname,"r");
    if (decinfo->fptr_stego_image == NULL)		// validation for fopen
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", decinfo->stego_image_fname);

	return e_failure;
    }
    return e_success;
}
// Function to read and validates the CLA 
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decinfo)
{
    // checking the 3 argument is bitmap file or not
    if( (argv[2] != NULL) && (strstr(argv[2],".bmp")))
    {
	decinfo -> stego_image_fname=argv[2];	// stores the encoded file name
	if( (argv[3] != NULL) && (strchr(argv[3],'.')))		// checking the default file extension provided by the user
	{
	    return e_success;
	}
	else if( (argv[3] !=NULL) && (strstr(argv[3],".") == NULL))
	    return e_failure;
	else
	    return e_success;
    }
    else
    {
	puts("ERROR: Not a .bmp file");
	return e_failure;
    }
}
// function to decode the magic string
Status decode_magic_string(char *magic_string,DecodeInfo *decinfo)
{
    fseek(decinfo->fptr_stego_image,54,SEEK_SET);	// moves the file pointer to the 54th byte, as the header size is 54 bytes in bitmap files
    int len=strlen(magic_string);		// stores the length of the magic string
    unsigned char buffer1[8];
    unsigned char buffer2[len+1];
    j=0;
    while(j<len)
    {
	buffer2[j]=0x00;
	fread(buffer1,8,1,decinfo->fptr_stego_image);	// reads the 8byte to decode 1 byte data from the input file
	for(i=0;i<8;i++)
	{
	    //takes the byte data from the encoded data and do AND operation with 1 and moves the data acc. to the positiom and do 8 times to get the byte [decoded] and stores it in the buffer
	    buffer2[j]= buffer2[j] | ( (buffer1[i] & 1) << i) ;
	}
	j++;
    }
    buffer2[j]='\0';  
    if( (strcmp(buffer2,MAGIC_STRING)) == 0 )	// Compares the decoded string with the original string
	return e_success;
    else
	return e_failure;
}
// function to decode secret file extension size
Status decode_secret_file_extn_size(DecodeInfo *decinfo,int *n)
{
    char buffer1[32];
    fread(buffer1,32,1,decinfo->fptr_stego_image);	// as the size is in integer, reads the 32 bytes from the inout file
    decode_size(buffer1,n);	// function to decode 1byte data from 8bytes 

    return e_success;
}
// function to decode secret file extension
Status decode_secret_file_extn(DecodeInfo *decinfo,int size)
{
    char ch;
    char buffer[8];
    char *dest=decinfo->extn_secret_file;	// pointing to the extensionbuffer
    for(int i=0;i<size;i++)
    {
	// reads the 8byte data
	fread(&buffer,8,1,decinfo->fptr_stego_image);
	decode_data_from_lsb(buffer,&ch);	// calls the function to decode 1 byte of data
	dest[i]=ch;			// stores the decode byte
    }
    return e_success;
}
// function to decode the size
Status decode_size(char *data,int *n)
{
    int num;
    for(i=0;i<32;i++)
    {
	// take encoded byte and do AND operation wit 1 and moves the result bit acc. to position and OR operatiom with result and repeat the same 32 times as it is integer
	num =num|( (data[i] & 1)<<i);
    }
    *n=num;
}

// Function to decode the 1 byte from 8 bytes of data
Status decode_data_from_lsb(char *source,char *dest)
{
    char ch;
    for(i=0;i<8;i++)
    {
	// take encoded byte and do AND operation wit 1 and moves the result bit acc. to position and OR operation with result and repeat the same 8 times to get 1 byte data
	ch= ch | ( (source[i] & 1<<0) << i) ;
    }
    *dest=ch; 	// stores the decoded 1 byte data
}

// function to open output file
Status decode_new_sec_file_open(DecodeInfo *decinfo,char *name)
{
    if(name != NULL && strstr(name,"." ))		// checking the user passed the output file name and that file has the extension
    {
	char sec[50]="h";
	i=0;
	while (name[i] != '.')				// loop to get only the output file name
	{
	    sec[i]=name[i];
	    i++;
	}
	sec[i] == '\0';
	decinfo -> secret_fname= strcat(sec,decinfo->extn_secret_file);		// cancatenates the file name and extension
	decinfo -> fptr_secret = fopen(decinfo->secret_fname,"w");	// open the output file in write mode
	if (decinfo->fptr_secret == NULL)			// fopen validation
	{
	    perror("fopen");
	    fprintf(stderr, "ERROR: Unable to open file %s\n", decinfo->secret_fname);
	    return e_failure;
	}
    }
    else 				// cond. when the user didn't pass the output file name
    {
	char sec[50]="def_sec";
	decinfo -> secret_fname= strcat(sec,decinfo->extn_secret_file);	//opening the default file
	decinfo -> fptr_secret = fopen(decinfo->secret_fname,"w");
	if (decinfo->fptr_secret == NULL)	// validation for fopen
	{
	    perror("fopen");
	    fprintf(stderr, "ERROR: Unable to open file %s\n", decinfo->secret_fname);
	    return e_failure;
	}
    }
    return e_success;
}
// Function to decode the secret file size 
Status decode_secret_file_size(DecodeInfo *decinfo)
{ 
    int n=decinfo->size_secret_file;
    char buffer1[32];
    fread(buffer1,32,1,decinfo->fptr_stego_image);	//reads the 32 byte data from the encoded image 
    decode_size(buffer1,&n);				// function call to find the size of the secret data

    decinfo->size_secret_file=n;			// storing the size of the secret data
    return e_success;

}
// Function to decode secret data
Status decode_secret_data(DecodeInfo *decinfo)
{
    char buffer[8];
    char ch;
    int size=decinfo->size_secret_file;			
    while(size>0)
    {
	fread(buffer,8,1,decinfo->fptr_stego_image);	// reads the 8byte data from input image

	decode_data_from_lsb(buffer,&ch);		// function to decode 1 byte data
	fwrite(&ch,1,1,decinfo->fptr_secret);		// storing the decoded data into a output file
	size--;
    }
    return e_success;
}
// Function to do decoding
Status do_decoding( DecodeInfo *decinfo,char *argv[])
{
    char *name=NULL;
    int size=0;
    puts("INFO: opening the file");
    if( decode_open_file(decinfo) == e_success)	// checks the status of opening file function
    {
	puts("INFO: Done");
	puts("INFO: Decoding Magic String Signature");
	if( decode_magic_string(MAGIC_STRING,decinfo) == e_success)  // checks the status of decoding magic string function
	{
	    puts("INFO: Done");
	    puts("ÏNFO: decoding secret file exten size");
	    if( decode_secret_file_extn_size(decinfo,&size)==e_success)  // checks the status of decoding secret file extension size function
	    {
		puts("INFO: Done");
		puts("INFO: Decoding secret File Extension");
		if( decode_secret_file_extn(decinfo,size) == e_success)  // checks the status of decoding secret file extensinon function
		{
		    puts("INFO: Done");
		    if(argv[3] != NULL)	// if the default output file is provided, then checks for the extension and opens it
		    {
			name=argv[3];
			printf("INFO: Opening c%s\n",argv[3]);
		    }
		    else
			printf("INFO: As there is file passed, Opening default def_sec%s\n",decinfo->extn_secret_file);
		    puts("ÏNFO: Opening the secret file");
		    if( decode_new_sec_file_open(decinfo,name) == e_success)  // checks the status of opening output file function
		    {
			puts("INFO: Done");
			puts("INFO: Decoding secret File size");
			if( decode_secret_file_size(decinfo) ==e_success ) // checks the status of decode secret file size function
			{
			    puts("INFO: Done");
			    puts("INFO: Decoding secret Data");
			    if( decode_secret_data(decinfo) ==e_success)  // checks the status of decoding secret data function
			    {
				puts("ÍNFO: Done");
				puts("INFO: ## Decoding Done Successfully ##");
				return e_success;
			    }
			    else
			    {
				puts("ERROR: decoding secret data is failed");
				return e_failure;
			    }
			}
			else
			{
			    puts("ERROR: decoding secret file size is failed");
			    return e_failure;
			}
		    }
		    else
		    {
			puts("ERROR: failed to open dafult secret file");
			return e_failure;
		    }
		}
		else
		{
		    puts("ERROR: decoding secret file extension is failed");
		    return e_failure;
		}
	    }	
	    else
	    {
		puts("ERROR: decoding secret file extension size is failed");
		return e_failure;
	    }
	}
	else
	{
	    puts("ERROR: decoding MAGIC STRING is failed");
	    return e_failure;
	}
    }
    else
    {
	puts("ERROR: Opening required file is failed");
	return e_failure;
    }
}

