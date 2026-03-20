/*
DOCUMENTATION:

NAME    : Gagan Bhairamatti
ROLL NO : 25021C_403
PROJECT : LSB IMAGE STEGANOGRAPHY.

SAMPLE INPUT (Encoding):
1. Source Image      : beautiful.bmp
2. Secret File       : secret.txt
3. Destination Image : stego.bmp (optional)

SAMPLE OUTPUT (Encoding):
========== ENCODING PROCESS STARTED ==========

[INFO] Files opened successfully
[INFO] Image capacity is sufficient for encoding.
[INFO] Capacity check passed.
[INFO] BMP header copied successfully.
[INFO] Extension size encoded successfully.
[INFO] Extension encoded successfully.
[INFO] File size encoded successfully.
[INFO] File data encoded successfully.
[INFO] Remaining image data copied successfully.
[INFO] Files closed successfully

================================================
SUCCESS : Encoding completed successfully!
Desination image saved as: stego.bmp
================================================

SAMPLE INPUT (Decoding):
1. Stego Image       : stego.bmp
2. Destination File  : decode_secret.txt (optional)

SAMPLE OUTPUT (Decoding):
========== DECODING PROCESS STARTED ==========

[INFO] Stego image file opened successfully.
[INFO] BMP header skipped successfully.
[INFO] Magic string verified successfully.
[INFO] Magic string decoded successfully.
[INFO] Secret file extension size decoded: 4
[INFO] Extension size decoded successfully.
[INFO] Secret file opened successfully: decode_secret.txt
[INFO] Secret file extension decoded: .txt
[INFO] Extension decoded successfully.
[INFO] Secret file size decoded: 36 bytes
[INFO] File size decoded successfully.
[INFO] Secret file data decoded successfully.
[INFO] File data decoded successfully.
[INFO] Files closed successfully

================================================
SUCCESS : Decoding completed successfully!
Secret file saved as: decode_secret.txt
================================================

DESCRIPTION:
This project implements an LSB Image Steganography system in C.
It allows the user to:
- Encode a secret text file into a BMP image
- Decode a hidden secret file from a BMP image
- Handle file validation and capacity checking
- Maintain original image integrity after encoding
*/
#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"
#include <string.h>
#include "colors.h"

int main(int argc, char *argv[])
{
    EncodeInfo encodeInfo;
    DecodeInfo decodeInfo;

    if (argc < 2)
    {
        printf(YELLOW "Follow the procedure:\n" RESET);
        printf(MAGENTA "Usage:\n" RESET);
        printf(CYAN "Encoding : ./a.out -e <.bmp file> <.txt file> [Output file.bmp]\n" RESET);
        printf(CYAN "Decoding : ./a.out -d <.bmp file> [output_file_name]\n" RESET);
        return 0;
    }

    OperationType res = check_operation_type(argv[1]);  // Check operation type

    if (res == e_encode)  // IF -> e_encode
    {
        if (argc >= 4 && argc <= 5)
        {
            if (read_and_validate_encode_args(argv, &encodeInfo) == e_failure)  // Validate CLA
            {
                return e_failure;
            }

            printf(CYAN "\n========== ENCODING PROCESS STARTED ==========\n\n" RESET);

            if (do_encoding(&encodeInfo) == e_failure)  // Start the encoding
            {
                printf(RED "\n================================================\n" RESET);
                printf(RED "FAILURE : Encoding failed!\n" RESET);
                printf(RED "================================================\n\n" RESET);
                return e_failure;
            }

            close_encode_files(&encodeInfo);

            printf(MAGENTA "\n================================================\n" RESET);
            printf(GREEN "SUCCESS : Encoding completed successfully!\n" RESET);
            printf(YELLOW "Desination image saved as: %s\n" RESET, encodeInfo.stego_image_fname);
            printf(MAGENTA"================================================\n\n" RESET);
        }
        else
        {
            printf(RED "Error: Invalid number of arguments for Encoding\n" RESET);
            printf(YELLOW "Follow the procedure\n" RESET);
            printf(BLUE "Usage:\n" RESET);
            printf(CYAN "Encoding : ./a.out -e <.bmp file> <.txt file> [Output file with .bmp]\n" RESET);
        }
    }
    else if (res == e_decode)  // IF -> e_decode
    {
        if (argc >= 3 && argc <= 4)
        {
            if (read_and_validate_decode_args(argv, &decodeInfo) == e_failure)  // Validate CLA
            {
                return e_failure;
            }

            printf(CYAN "\n========== DECODING PROCESS STARTED ==========\n\n" RESET);

            if (do_decoding(&decodeInfo) == e_failure)  // Start the Decoding
            {
                printf(RED "\n================================================\n" RESET);
                printf(RED "FAILURE : Decoding failed!\n" RESET);
                printf(RED "================================================\n\n" RESET);
                return e_failure;
            }

            close_decode_files(&decodeInfo);

            printf(MAGENTA "\n================================================\n" RESET);
            printf(GREEN "SUCCESS : Decoding completed successfully!\n" RESET);
            printf(YELLOW "Secret file saved as: %s\n" RESET, decodeInfo.secret_fname);
            printf(MAGENTA "================================================\n\n" RESET);
        }
        else
        {
            printf(RED "Error: Invalid number of arguments for Decoding\n" RESET);
            printf(YELLOW "Follow the procedure\n" RESET);
            printf(BLUE "Usage:\n" RESET);
            printf(CYAN "Decoding : ./a.out -d <.bmp file> [output_file_name]\n" RESET);
        }
    }
    else  // IF -> e_unsupported
    {
        printf(RED "Error: Unsupported operation. Use Either -e for encode or -d for decode\n" RESET);
        printf(BLUE "Usage:\n" RESET);
        printf(CYAN "Encoding : ./a.out -e <.bmp file> <.txt file> [Output file with .bmp]\n" RESET);
        printf(CYAN "Decoding : ./a.out -d <.bmp file> [output_file_name]\n" RESET);
        return e_failure;
    }

    return 0;
}

/* Validation Function Definitions */

OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    
    return e_unsupported;
}