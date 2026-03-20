#include  <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#include "colors.h"  // Make sure you have color definitions like RED, GREEN, RESET

#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define CYAN    "\x1B[36m"
#define RESET   "\x1B[0m"


// Validate command-line arguments for decoding: stego image and output secret file
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decodeInfo)
{
    char *ptr;

    // Validate that the stego image file has .bmp extension
    ptr = strstr(argv[2], ".bmp");
    if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        decodeInfo->stego_image_fname = argv[2];
        printf(GREEN "Destination image validated: %s\n" RESET, argv[2]); // Inform user about valid stego image
    }
    else
    {
        printf(RED "Error: Invalid stego image file. It must be a .bmp file.\n" RESET); // Error if not .bmp
        return e_failure;
    }

    // Check if output secret file is provided; if not, use default name "decode_secret"
    if(argv[3] == NULL)
    {
        strcpy(decodeInfo->secret_fname, "decode_secret");
        printf(YELLOW "Destination file not provided. Using default: decode_secret\n" RESET); // Inform default used
    }
    else 
    {
        ptr = strchr(argv[3], '.');  // Ensure user did not provide a file extension
        if(ptr == NULL)
        {
            strcpy(decodeInfo->secret_fname, argv[3]);
            printf(GREEN "Output secret file validated: %s\n" RESET, argv[3]); // Confirm valid secret file name
        }
        else
        {
            printf(RED "Error: Destination file name should not include extension\n" RESET); // Error if extension included
            return e_failure;
        }
    }

    return e_success; // Arguments validated successfully
}

// Skip the 54-byte BMP header to start decoding secret data
Status skip_bmp_header(FILE *fptr_stego_image)
{
    if(fseek(fptr_stego_image, 54L, SEEK_SET) != 0)
    {
        fprintf(stderr, RED "ERROR: Cannot skip BMP header.\n" RESET);
        return e_failure;  // Header skip failed
    }
    
    return e_success;
}


// Open the secret file for writing decoded data
Status open_secret_file(DecodeInfo *decodeInfo)
{
    // Attempt to open the secret file in write mode
    decodeInfo->fptr_secret = fopen(decodeInfo->secret_fname, "w");

    // Check if file opened successfully
    if(decodeInfo->fptr_secret == NULL)
    {
        fprintf(stderr, RED "ERROR: Unable to open file %s for writing.\n" RESET, decodeInfo->secret_fname);
        return e_failure;  // Return failure if file cannot be opened
    }

    printf(GREEN "[INFO] Secret file opened successfully: %s\n" RESET, decodeInfo->secret_fname);
    return e_success;  // File opened successfully
}


// Open the stego image file for reading the encoded data
Status open_image_file(DecodeInfo *decodeInfo)
{
    // Attempt to open the stego image file in read mode
    decodeInfo->fptr_stego_image = fopen(decodeInfo->stego_image_fname, "r");

    // Check if file opened successfully
    if(decodeInfo->fptr_stego_image == NULL)
    {
        fprintf(stderr, RED "ERROR: Unable to open stego image file %s\n" RESET, decodeInfo->stego_image_fname);
        return e_failure;  // Return failure if file cannot be opened
    }
    return e_success;  // File opened successfully
}

// Decode and verify the magic string from the stego image
Status decode_magic_string(DecodeInfo *decodeInfo)
{
    char buffer[8];
    int size = strlen(MAGIC_STRING);

    char magic_string[size + 1];  // Store decoded magic string

    for (int i = 0; i < size; i++)
    {
        if(fread(buffer, 8, 1, decodeInfo->fptr_stego_image) != 1)
        {
            fprintf(stderr, RED "ERROR: Failed to read bytes for magic string.\n" RESET);
            return e_failure;
        }

        magic_string[i] = decode_byte_from_lsb(buffer);  // Decode one character from LSB
    }

    magic_string[size] = '\0';

    if(strcmp(magic_string, MAGIC_STRING) != 0)
    {
        fprintf(stderr, RED "ERROR: Magic string mismatch. Not a valid stego image.\n" RESET);
        return e_failure;
    }

    printf(GREEN "[INFO] Magic string verified successfully.\n" RESET);
    return e_success;
}

// Decode a single byte from 8 bytes of LSB data
char decode_byte_from_lsb(char *image_buffer)
{
    int i, j = 7, get;
    char ch = 0x00;

    for (i = 0; i < 8; i++)
    {
        get = (image_buffer[i] & 1);      // Extract least significant bit
        ch = ch | (get << j);             // Reconstruct byte from MSB to LSB
        j--;
    }

    return ch;
}

// Decode size of secret file extension (stored in 32 bits)
Status decode_secret_file_extn_size(DecodeInfo *decodeInfo)
{
    char buffer[32];

    if(fread(buffer, 32, 1, decodeInfo->fptr_stego_image) != 1)
    {
        fprintf(stderr, RED "ERROR: Failed to read extension size.\n" RESET);
        return e_failure;
    }

    decodeInfo->secret_file_extn_size = decode_int_from_lsb(buffer);  // Store decoded extension size
    printf(YELLOW "[INFO] Secret file extension size decoded: %ld\n" RESET, decodeInfo->secret_file_extn_size);
    return e_success;
}

// Decode a 32-bit integer from LSBs of 32 bytes
long decode_int_from_lsb(char *image_buffer)
{
    int i, j = 31, get;
    int num = 0;

    for (i = 0; i < 32; i++)
    {
        get = (image_buffer[i] & 1);      // Extract LSB
        num = num | (get << j);           // Reconstruct integer from MSB to LSB
        j--;
    }

    return num;
}

// Decode secret file extension and open the secret file
Status decode_secret_file_extn(DecodeInfo *decodeInfo)
{
    int size = decodeInfo->secret_file_extn_size;
    char s_extn[size + 1], buffer[8];

    for (int i = 0; i < size; i++)
    {
        if(fread(buffer, 8, 1, decodeInfo->fptr_stego_image) != 1)
        {
            fprintf(stderr, RED "ERROR: Failed to read extension bytes.\n" RESET);
            return e_failure;
        }

        s_extn[i] = decode_byte_from_lsb(buffer);  // Decode each character of extension
    }

    s_extn[size] = '\0';
    strcpy(decodeInfo->secret_fname, strcat(decodeInfo->secret_fname, s_extn));  // Append extension

    if(open_secret_file(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Cannot open secret file for writing.\n" RESET);
        return e_failure;
    }

    printf(YELLOW "[INFO] Secret file extension decoded: %s\n" RESET, s_extn);
    return e_success;
}

// Decode the size of secret file (stored in 32 bits)
Status decode_secret_file_size(DecodeInfo *decodeInfo)
{
    char buffer[32];

    if(fread(buffer, 32, 1, decodeInfo->fptr_stego_image) != 1)
    {
        fprintf(stderr, RED "ERROR: Failed to read secret file size.\n" RESET);
        return e_failure;
    }

    decodeInfo->secret_file_size = (long)decode_int_from_lsb(buffer);  // Store decoded file size
    printf(YELLOW"[INFO] Secret file size decoded: %ld bytes\n" RESET, decodeInfo->secret_file_size);
    return e_success;
}

// Decode the secret file data from LSBs and write to secret file
Status decode_secret_file_data(DecodeInfo *decodeInfo)
{
    char buffer[8];
    char ch = 0;

    for (int i = 0; i < decodeInfo->secret_file_size; i++)
    {
        if(fread(buffer, 8, 1, decodeInfo->fptr_stego_image) != 1)
        {
            fprintf(stderr, RED "ERROR: Failed to read secret file data.\n" RESET);
            return e_failure;
        }

        ch = decode_byte_from_lsb(buffer);  // Decode one byte
        if(fwrite(&ch, 1, 1, decodeInfo->fptr_secret) != 1)
        {
            fprintf(stderr, RED "ERROR: Failed to write decoded byte to secret file.\n" RESET);
            return e_failure;
        }
    }

    printf(GREEN "[INFO] Secret file data decoded successfully.\n" RESET);
    return e_success;
}

// Close all files used during decoding
Status close_decode_files(DecodeInfo *decodeInfo)
{
    int flag = 0;

    // Close the stego image file if it is open
    if(decodeInfo->fptr_stego_image)
    {
        fclose(decodeInfo->fptr_stego_image);
        flag = 1; // Mark that a file was closed
    }

    // Close the secret output file if it is open
    if(decodeInfo->fptr_secret)
    {
        fclose(decodeInfo->fptr_secret);
        flag = 1; // Mark that a file was closed
    }

    // Inform user that files were successfully closed
    if(flag == 1)
        printf(GREEN "[INFO] Files closed successfully\n" RESET);

    return e_success; // Return success status
}



Status do_decoding(DecodeInfo *decodeInfo)
{
    // Open the Image File
    if (open_image_file(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to open stego image file.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Stego image file opened successfully.\n" RESET);

    // Skip BMP header
    if (skip_bmp_header(decodeInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to skip BMP header.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] BMP header skipped successfully.\n" RESET);

    // Decode Magic String
    if (decode_magic_string(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to decode magic string.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Magic string decoded successfully.\n" RESET);

    // Decode Extension Size
    if (decode_secret_file_extn_size(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to decode extension size.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Extension size decoded successfully.\n" RESET);

    // Decode Extension
    if (decode_secret_file_extn(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to decode extension.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Extension decoded successfully.\n" RESET);

    // Decode File Size
    if (decode_secret_file_size(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to decode file size.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] File size decoded successfully.\n" RESET);

    // Decode File Data
    if (decode_secret_file_data(decodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to decode file data.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] File data decoded successfully.\n" RESET);

    return e_success;
} 