#include <stdio.h>
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
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *str=strstr(argv[2],".bmp"); //validate the source file it is having .bmp or not
    if(str!=NULL && (strcmp(str,".bmp"))==0)
    {
        encInfo->src_image_fname=argv[2];
    }
    else
    {
        printf("Error: Invalid file extension. Expected .bmp file, but received \"%s\"\n",argv[2]);
        return e_failure;
    }
    str=strrchr(argv[3],'.');
    if(str!=NULL && ((strcmp(str,".txt"))==0 || (strcmp(str,".c"))==0 || (strcmp(str,".sh"))==0) || (strcmp(str,".h"))==0 ) //validating the secret file all supported extention
    {
        encInfo->secret_fname=argv[3];
        strcpy(encInfo->extn_secret_file,str);

    }
    else 
    {
        printf("Error: Invalid file extension. Expected .txt file \"%s\"\n",argv[3]);
        return e_failure;
    }
    if(argv[4]!=NULL)  //if the destination file name is given then we need validate
    {
        str=strstr(argv[4],".bmp");
        if(str!=NULL && (strcmp(str,".bmp"))==0)  //checking destination file must have .bmp
        {
            encInfo->dest_image_fname=argv[4];
        }
        else
        {
             printf("Error: Invalid file extension. Expected .bmp file, but received \"%s\"\n",argv[4]);
            return e_failure;
        }
    }
    else //user don't pass any file name then store default file name
    {
        encInfo->dest_image_fname="demo.bmp";
    }
    return e_success;
}

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
    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_dest_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}
Status check_capacity(EncodeInfo *encInfo)
{
    uint image_size=get_image_size_for_bmp(encInfo->fptr_src_image); //get the size of RGB data
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret); //get the size of secret data
    
    if(image_size>((encInfo->size_secret_file+strlen(MAGIC_STRING)+sizeof(int)+sizeof(int)+strlen(encInfo->extn_secret_file)*8))) //To check source file is having the capacity to store the data or not
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }

}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image); //move the file pointer to the beginning of the file
    char buffer[54];
    fread(buffer,sizeof(buffer),1,fptr_src_image); //read the header
    fwrite(buffer,sizeof(buffer),1,fptr_dest_image);//write the header to the destination file

    if(ftell(fptr_src_image)==ftell(fptr_dest_image)) // Verify file pointers match
    {
        return e_success;
    }
    else 
    {
        return e_failure;
    }
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];
    int i=0;
    while(magic_string[i])
    {
        fread(image_buffer,8,1,encInfo->fptr_src_image); //reading data from source 
        encode_byte_to_lsb(magic_string[i],image_buffer); //hiding data into lsb bits 
        fwrite(image_buffer,8,1,encInfo->fptr_dest_image); // write the data into destination file
        i++;
    }
    if(ftell(encInfo->fptr_src_image)!=ftell(encInfo->fptr_dest_image)) // Verify file pointers match
    {
        return e_failure;
    }
    return e_success;
   


}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char image_buffer[32]; // Buffer to store 32 bytes from source image
    fread(image_buffer,32,1,encInfo->fptr_src_image); // Read 32 bytes from source image

    encode_size_to_lsb(size,image_buffer); // Encode extension size into LSBs
    fwrite(image_buffer,32,1,encInfo->fptr_dest_image); // Write modified bytes to destination image
    if(ftell(encInfo->fptr_src_image)!=ftell(encInfo->fptr_dest_image)) // Verify file pointers match
    {
        return e_failure; // Return failure if mismatch
    }
    return e_success; 
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char image_buffer[8]; // Buffer to store 8 bytes from image
    int i=0; 
    while(file_extn[i]) // Loop until end of extension string
    {
        fread(image_buffer,8,1,encInfo->fptr_src_image); // Read 8 bytes from source image
        encode_byte_to_lsb(file_extn[i],image_buffer); // Encode extension character into LSB
        fwrite(image_buffer,8,1,encInfo->fptr_dest_image); // Write modified bytes to destination image
        i++; // Move to next character
    }
    if(ftell(encInfo->fptr_src_image)!=ftell(encInfo->fptr_dest_image)) // Check file pointer positions
    {
        return e_failure; 
    }
    return e_success; 
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
     char image_buffer[32]; // Buffer for 32 bytes of image data
     fread(image_buffer,32,1,encInfo->fptr_src_image); // Read 32 bytes from source image
     encode_size_to_lsb(file_size,image_buffer); // Encode secret file size into LSB
     fwrite(image_buffer,32,1,encInfo->fptr_dest_image); // Write modified data to destination image

    if(ftell(encInfo->fptr_src_image)!=ftell(encInfo->fptr_dest_image)) // Verify pointer positions
    {
        return e_failure; 
    }
    return e_success; 
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret); // Move secret file pointer to beginning
    char secret_data[encInfo->size_secret_file]; // Buffer to store secret file data
    fread(secret_data,encInfo->size_secret_file,1,encInfo->fptr_secret); // Read entire secret file

    char image_buffer[8]; // Buffer to store 8 bytes of image
    for(int i=0;i<encInfo->size_secret_file;i++) // Loop through each byte of secret data
    {
        fread(image_buffer,8,1,encInfo->fptr_src_image); // Read 8 bytes from image
        encode_byte_to_lsb(secret_data[i],image_buffer); // Encode one byte into LSB
        fwrite(image_buffer,8,1,encInfo->fptr_dest_image); // Write modified bytes to destination image
    }

    if(ftell(encInfo->fptr_src_image)!=ftell(encInfo->fptr_dest_image)) // Check pointer positions
    {
        return e_failure; 
    }
    return e_success; 
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch; // Variable to store single byte
    while((fread(&ch,1,1,fptr_src))!=0) // Read remaining bytes from source image
    {
        fwrite(&ch,1,1,fptr_dest); // Write remaining bytes to destination image
    }

    if(ftell(fptr_src)!=ftell(fptr_dest)) // Check pointer positions
    {
        return e_failure; 
    }
    return e_success; 
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++) // Loop through 8 bits of data
    {
        uint get=(data>>i) & 1; // Extract i-th bit of data
        image_buffer[i]=(image_buffer[i] & (~1)); // Clear LSB of image byte
        image_buffer[i]|=get; // Set LSB with extracted bit
    }
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
   for(int i=0;i<32;i++) // Loop through 32 bits of size
   {
        uint get=(size>>i) & 1; // Extract i-th bit of size
        image_buffer[i]=(image_buffer[i] & (~1)); // Clear LSB of image byte
        image_buffer[i]|=get; // Set LSB with extracted bit
   }
}

Status do_encoding(EncodeInfo *encInfo)
{
    if((open_files(encInfo))==e_failure)// Open source, destination, and secret files
    {
        return e_failure;
    }
    else
    {
        printf("INFO: opened required files\n");
    }
    if((check_capacity(encInfo))==e_success)// Check if image can hold secret data
    {
        printf("INFO: Capacity check passed.\n");
    }
    else
    {
        printf("ERROR: Capacity check failed.\n");
        return e_failure;
    }
    printf("## Encoding Procedure Started ##\n");
    if((copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_dest_image))==e_success)//Copy BMP header

    {
        printf("INFO: Copying Image Header\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: Unable to read BMP file header.\n");
        return e_failure;
    }
    if((encode_magic_string(MAGIC_STRING,encInfo))==e_success)// Encode magic string
    {
         printf("INFO: Encoding Magic String Signature\nINFO: Done\n");
    }
    else 
    {
        printf("ERROR: Magic string not encoded\n");
        return e_failure;
    }
    if((encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo))==e_success)//encode extention size
    {
        printf("INFO: Encoding secret file extention size\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: secret file extention size not encoded\n");
        return e_failure;
    }
    if((encode_secret_file_extn(encInfo->extn_secret_file,encInfo))==e_success)//Encode secret file extention
    {
        printf("INFO: Encoding secret file extention\nINFO: Done\n");
    }
    else
    {
         printf("ERROR: secret file extention not encoded\n");
        return e_success;
    }
    if((encode_secret_file_size(encInfo->size_secret_file,encInfo))==e_success)//encode secret file size
    {
        printf("INFO: Encoding secret File Size\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: Secret file size not encoded\n");
        return e_failure;
    }
    if((encode_secret_file_data(encInfo))==e_success) //encode secret data
    {
         printf("INFO: Encoding secret File Data.\nINFO: Done\n");
    }
    else
    {
        printf("ERROR: Secret file data not encoded\n");
    }
    if((copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_dest_image))==e_success)//copy remaining data
    {
        printf("INFO: Remaining data encoding\nINFO: done\n");
    }
    else
    {
        printf("ERROR: Remaining data encoding fail\n");
        return e_failure;
    }
    close_file_pointers(encInfo); //close file pointers
    return e_success;
}
void close_file_pointers(EncodeInfo* encInfo)
{
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_dest_image);
    fclose(encInfo->fptr_secret);
}
