#include <stdio.h>
#include "decode.h"
#include "types.h"
#include "common.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *dncInfo) // validating the arguments
{
    char *str=strstr(argv[2],".bmp"); // Check if input image has .bmp extension
    if(str!=NULL && (strcmp(str,".bmp"))==0)
    {
        dncInfo->src_image_fname=argv[2]; // Store source image file name
    }
    else
    {
        printf("Error: Invalid file extension. Expected .bmp file, but received \"%s\"\n",argv[2]); // Error for invalid file type
        return e_failure; // Return failure if not bmp
    }
    
    if(argv[3]!=NULL) // Check if output file name is provided
    { 
        char *extn=strrchr(argv[3],'.'); // Find extension in output filename
        if(extn==NULL) // If no extension present
        {
          strcpy(dncInfo->secret_fname,argv[3]); // Copy filename directly
        }
        else if(str!=NULL && ((strcmp(extn,".txt"))==0 || (strcmp(extn,".c"))==0 || (strcmp(extn,".sh"))==0)) // Validate supported extensions
        {
            strtok(argv[3],"."); // Remove extension part
            strcpy(dncInfo->secret_fname,argv[3]); // Store output filename
        }
        
    }
    else
    {
        strcpy(dncInfo->secret_fname,"default"); // Assign default filename if not provided
    }
    return e_success; 
}

Status open_src_file(DecodeInfo *dncInfo)
{
    dncInfo->fptr_src_image=fopen(dncInfo->src_image_fname,"rb"); //open the source file
    if (dncInfo->fptr_src_image == NULL)  
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", dncInfo->src_image_fname);

        return e_failure;  
    }
    fseek(dncInfo->fptr_src_image,54,SEEK_SET); //ignore the header of source file
    return e_success;
}

Status decode_magic_string(DecodeInfo *dncInfo)
{
      char image_buffer[8];
      int i=0;
      while(i<2)
      {
           unsigned char ch=0;
           fread(image_buffer,8,1,dncInfo->fptr_src_image); //read the 8 bytes from source file
           decode_byte_from_lsb(&ch,image_buffer); //call this function to get 1 byte of magic string
           dncInfo->magic_string[i]=ch; //store the in the buffer
           i++;
      }
      dncInfo->magic_string[i]='\0';
    if((strcmp(dncInfo->magic_string,MAGIC_STRING))!=0)  //compare the magic string valid or not
    {
        return e_failure; //if not valid then terminate program
    }
    return e_success;
}
Status decode_secret_file_extn_size(DecodeInfo *dncInfo)
{
      dncInfo->extn_size = 0;
      char image_buffer[32];   
      fread(image_buffer,32,1,dncInfo->fptr_src_image); //decoding integer we need 32 bytes that's why read 32 bytes
      decode_size_to_lsb(&dncInfo->extn_size,image_buffer);
      
      return e_success;
}
Status decode_secret_file_extn(DecodeInfo *dncInfo)
{
    char image_buffer[8];//for decode one character we need 8 bytes
    int i=0;
    for(i=0;i<dncInfo->extn_size;i++)
    {
        unsigned char ch=0;
        fread(image_buffer,8,1,dncInfo->fptr_src_image); //read 8 bytes from source file
        decode_byte_from_lsb(&ch,image_buffer);
        dncInfo->extn_secret_file[i]=ch;
    }
    dncInfo->extn_secret_file[i]='\0';
    return e_success;
}
Status open_secret_file(DecodeInfo *dncInfo)
{
    strcat(dncInfo->secret_fname,dncInfo->extn_secret_file); //concatenate the output file name and output file extention
    dncInfo->fptr_secret=fopen(dncInfo->secret_fname,"w"); //open output file
    if (dncInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", dncInfo->secret_fname);
        return e_failure;
    }
    return e_success;
}
Status decode_secret_file_size(DecodeInfo *dncInfo)
{
    dncInfo->size_secret_file=0; // Initialize secret file size to 0
    char image_buffer[32]; // Buffer to store 32 bytes from image
    fread(image_buffer,32,1,dncInfo->fptr_src_image); // Read 32 bytes from source image
    decode_size_to_lsb(&dncInfo->size_secret_file,image_buffer); // Decode file size from LSBs
    return e_success;
}
Status decode_secret_data(DecodeInfo *dncInfo)
{
    char image_buffer[8]; // Buffer to store 8 bytes from image
    for(int i=0;i<dncInfo->size_secret_file;i++) // Loop for each byte of secret file
    {
        unsigned char ch=0; // Variable to store decoded character
        fread(image_buffer,8,1,dncInfo->fptr_src_image); // Read 8 bytes from image
        decode_byte_from_lsb(&ch,image_buffer); // Decode one byte from LSB
        fputc(ch,dncInfo->fptr_secret); // Write decoded character to secret file
    }
    return e_success; 
}

Status decode_byte_from_lsb(unsigned char *data, char *image_buffer)
{
    for(int i=0;i<8;i++) //loop run 8 time because of at a time we decode 1 byte
    {
        *data |= (image_buffer[i] & 1) << i;  //get bit from lsb store into variable
    }
}
Status decode_size_to_lsb(uint *size, char *image_buffer)
{
    for(int i=0;i<32;i++) //loop run 32 time because of we decode integer data
    {
        *size |= (image_buffer[i] & 1) << i;//get bit from lsb store into variable
    }
}
Status do_decoding(DecodeInfo *dncInfo)
{
    printf("INFO: ## Decoding Procedure Started ##\n");
    if((open_src_file(dncInfo))==e_failure)  //open the source file
    {
        printf("ERROR: file is not opened done\n");
        return e_failure;
    }
    else
    {
        printf("INFO: Opening required files\nINFO: Done\n");
    }
    if(decode_magic_string(dncInfo)==e_failure) //decode magic string
    {
        printf("ERROR: Magic string is not decoded.\n");
        return e_failure;
    }
    else{
        printf("INFO: Decoding Magic String Signature\nINFO: Done\n");
    }
    if((decode_secret_file_extn_size(dncInfo))==e_success) //decode output file extention size
    {
        printf("INFO: Decoding Output File Extention size\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: secret file extention size decoding fail.\n");
        return e_failure;
    }
    if((decode_secret_file_extn(dncInfo))==e_success)//decode output file extention
    {
        printf("INFO: Decoding Output File Extention\nINFO: Done\n");
    }
    else{
        printf("ERROR: Extention decoding fail\n");
    }
    // if((open_secret_file(dncInfo))==e_failure);
    // {
    //     printf("error open\n");
    //     return e_failure;
    // }
    open_secret_file(dncInfo); //open output file
    printf("INFO: Opened output file\nINFO: Done. Opened all required files\n");
    if((decode_secret_file_size(dncInfo))==e_success) //decode secret data size
    {
        printf("INFO: Decoding File Size\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: File size is not decoded\n");
        return e_failure;
    }

    if((decode_secret_data(dncInfo))==e_success) //decode secret data
    {
        printf("INFO: Decoding  File Data\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: Problem while reading secret data\n");
    }
    close_file_pointers_decoding(dncInfo); //close the file pointer
    return e_success;
}
void close_file_pointers_decoding(DecodeInfo *dncInfo)
{
    fclose(dncInfo->fptr_src_image); //close the source file pointer
    fclose(dncInfo->fptr_secret); //close the output file pointer
}
