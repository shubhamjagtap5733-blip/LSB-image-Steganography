/*
Project Name:LSB Image Steganography
Name: Shubham Maroti Jagtap
Date: 11/03/2026
description:This project implements Image Steganography using the Least Significant Bit (LSB) technique in C programming. 
            The main purpose of this project is to hide secret information inside an image without noticeably changing the image.
            In this method, the least significant bits of the image pixels are modified to store the secret data. 
            Since only the last bit of each byte is changed, the difference in the image is not visible to the human eye.
*/
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    if(((check_operation_type(argv[1]))==e_encode) && argc!=4 && argc!=5) //checking arguments for encoding
    {
        printf("invalid arguments for encoding\n");

        printf("./lsb_steg: Encoding: ./lsb_steg -e <.bmp_file> <.text_file> [output file]\n./lsb_steg: Decoding: ./lsb_steg -d <.bmp_file> [output file]\n");
        return e_failure;
    }
    if(((check_operation_type(argv[1]))==e_decode) && argc!=3 && argc!=4)
    {
        printf("invalid arguments for decoding\n");
        printf("./lsb_steg: Encoding: ./lsb_steg -e <.bmp_file> <.text_file> [output file]\n./lsb_steg: Decoding: ./lsb_steg -d <.bmp_file> [output file]\n");
        return e_failure;
    }
    if((check_operation_type(argv[1]))==e_unsupported)
    {
        printf("Invalid operation\n");
        printf("./lsb_steg: Encoding: ./lsb_steg -e <.bmp_file> <.text_file> [output file]\n./lsb_steg: Decoding: ./lsb_steg -d <.bmp_file> [output file]\n");
        return e_failure;
    }
    EncodeInfo encInfo;
    if(((check_operation_type(argv[1]))==e_encode) && ((read_and_validate_encode_args(argv,&encInfo))==e_failure))
    {
        return e_failure;
    }
    if((check_operation_type(argv[1]))==e_encode)
    {
        if((do_encoding(&encInfo))==e_success)
        {
            printf("INFO: ## Encoding Done Successfully ##\n");
            return e_success;
        }
        else
        {
            printf("Encoding fail\n");
            return e_failure;
        }
    }
    DecodeInfo dncInfo;
    if(((check_operation_type(argv[1]))==e_decode) && ((read_and_validate_decode_args(argv,&dncInfo))==e_failure))
    {
        return e_failure;
    }
    if((check_operation_type(argv[1]))==e_decode)
    {
        if((do_decoding(&dncInfo))==e_success)
        {
            printf("INFO: ## Decoding Done Successfully ##\n");
            return e_success;
        }
        else
        {
            printf("decoding fail\n");
            return e_failure;
        }
    }

    
    
}
OperationType check_operation_type(char *symbol)
{
    if(symbol==NULL)
    {
        return e_unsupported;
    }
    if((strcmp(symbol,"-e"))==0)
    {
        return e_encode;
    }
    else if((strcmp(symbol,"-d"))==0)
    {
        return e_decode;
    }
    return e_unsupported;

}