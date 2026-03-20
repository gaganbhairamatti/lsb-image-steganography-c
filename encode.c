#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include "colors.h"




/* 
 * Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18, and height after that. size is 4 bytes
 */


/* 
 * Calculate the usable image size for BMP file
 * Inputs: FILE pointer to image
 * Returns: total number of bytes available for encoding
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    // Move file pointer to 18th byte in BMP header (where width and height are stored)
    fseek(fptr_image, 18, SEEK_SET);

    // Read width and height (both are 4-byte integers)
    if(fread(&width, sizeof(int), 1, fptr_image) != 1 || fread(&height, sizeof(int), 1, fptr_image) != 1)
    {
        fprintf(stderr, RED "ERROR: Unable to read width and height from the BMP header.\n" RESET);
        return 0;
    }

    // Each pixel has 3 bytes (RGB), return total capacity
    return width * height * 3;
}

/* 
 * Open Source Image, Secret file, and Stego Image
 * Returns e_success if all files opened successfully, else e_failure
 */
Status open_files(EncodeInfo *encodeInfo)
{
    // Open source image for reading
    encodeInfo->fptr_src_image = fopen(encodeInfo->src_image_fname, "r");
    if (!encodeInfo->fptr_src_image)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR: Unable to open source image %s\n" RESET, encodeInfo->src_image_fname);
        return e_failure;
    }

    // Open secret file for reading
    encodeInfo->fptr_secret = fopen(encodeInfo->secret_fname, "r");
    if (!encodeInfo->fptr_secret)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR: Unable to open secret file %s\n" RESET, encodeInfo->secret_fname);
        return e_failure;
    }

    // Open stego image for writing (destination)
    encodeInfo->fptr_stego_image = fopen(encodeInfo->stego_image_fname, "w");
    if (!encodeInfo->fptr_stego_image)
    {
        perror("fopen");
        fprintf(stderr, RED "ERROR: Unable to create stego image %s\n" RESET, encodeInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}
/* Validate command-line arguments for encoding: source image, secret file, and destination image*/
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encodeInfo)
{
    char *ptr;

    // Validate source image file extension (.bmp)
    ptr = strstr(argv[2], ".bmp");   
    if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        encodeInfo->src_image_fname = argv[2];
        printf(GREEN "Source image validated: %s\n" RESET, argv[2]);
    }
    else
    {
        printf(RED "Error: Invalid source image file. It must be a .bmp file.\n" RESET);
        return e_failure;
    }
    
    // Validate secret file extension (.txt, .c, .sh)
    ptr = strchr(argv[3], '.');
    if(ptr != NULL && strcmp(ptr, ".txt") == 0)
    {
        encodeInfo->secret_fname = argv[3];
        strcpy(encodeInfo->extn_secret_file, ".txt");
        printf(GREEN "Secret file validated: %s\n" RESET, argv[3]);
    }
    else if(ptr != NULL && strcmp(ptr, ".c") == 0)
    {
        encodeInfo->secret_fname = argv[3];
        strcpy(encodeInfo->extn_secret_file, ".c");
        printf(GREEN "Secret file validated: %s\n" RESET, argv[3]);
    }
    else if(ptr != NULL && strcmp(ptr, ".sh") == 0)
    {
        encodeInfo->secret_fname = argv[3];
        strcpy(encodeInfo->extn_secret_file, ".sh");
        printf(GREEN "Secret file validated: %s\n" RESET, argv[3]);
    }
    else
    {
        printf(RED "Error: Invalid secret file type. Use .txt, .c, or .sh\n" RESET);
        return e_failure;
    }

    // Check if destination file is provided; else use default
    if (argv[4] == NULL)
    {
        printf(YELLOW "Destination file not provided. Using default: stego.bmp\n" RESET);
        encodeInfo->stego_image_fname = "stego.bmp";
    }
    else
    {
        ptr = strstr(argv[4], ".bmp");
        if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
        {
            encodeInfo->stego_image_fname = argv[4];
            printf(GREEN "Destination file validated: %s\n" RESET, argv[4]);
        }
        else
        {
            printf(RED "Error: Invalid destination file. It must be a .bmp file.\n" RESET);
            return e_failure;
        }
    }

    return e_success;
}



/* 
 * Check if the image has enough capacity to encode the secret file
 */
Status check_capacity(EncodeInfo *encodeInfo)
{
    // Get image capacity in bytes
    encodeInfo->image_capacity = get_image_size_for_bmp(encodeInfo->fptr_src_image);

    // Get secret file size
    encodeInfo->size_secret_file = get_file_size(encodeInfo->fptr_secret);

    // Calculate total bits needed for encoding (header + magic string + extension + file data)
    int encoding_data_size = 54 + (strlen(MAGIC_STRING) + 4 + strlen(encodeInfo->extn_secret_file) + 4 + encodeInfo->size_secret_file) * 8;

    // Check if the image can store the secret data
    if (encoding_data_size > encodeInfo->image_capacity)
    {
        fprintf(stderr, RED "ERROR: Image does not have enough capacity to encode the secret file.\n" RESET);
        return e_failure;
    }

    printf(GREEN "[INFO] Image capacity is sufficient for encoding.\n" RESET);
    return e_success;
}

/* 
 * Get size of any file
 */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);  // Move pointer to end
    return ftell(fptr);         // Return current position = file size
}

/* 
 * Copy BMP header (first 54 bytes) from source image to destination image
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    // Move pointer to start
    rewind(fptr_src_image);

    // Read header from source image
    if(fread(buffer, 54, 1, fptr_src_image) != 1)
        return e_failure;

    // Write header to destination image
    if(fwrite(buffer, 54, 1, fptr_dest_image) != 1)
        return e_failure;

    return e_success;
}

/* 
 * Encode magic string into image
 */
Status encode_magic_string(char *magic_string, EncodeInfo *encodeInfo)
{
    return encode_data_to_image(magic_string, strlen(magic_string), encodeInfo->fptr_src_image, encodeInfo->fptr_stego_image);
}

/* 
 * Encode an array of bytes into the LSB of image bytes
 */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[8];

    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from source image
        if(fread(buffer, 8, 1, fptr_src_image) != 1)
            return e_failure;

        // Encode one byte into LSBs of 8 image bytes
        encode_byte_to_lsb(data[i], buffer);

        // Write encoded bytes to destination
        if(fwrite(buffer, 8, 1, fptr_stego_image) != 1)
            return e_failure;
    }

    return e_success;
}

/* 
 * Encode a single byte into the LSB of 8 image bytes
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int j = 7; // Start from MSB

    for(int i = 0; i < 8; i++)
    {
        int bit = (data >> j) & 1;          // Get current bit
        image_buffer[i] = (image_buffer[i] & ~1) | bit; // Set LSB
        j--;
    }

    return e_success;
}

/* 
 * Encode size of secret file extension (32 bits)
 */
Status encode_secret_file_extn_size(long extn_size, EncodeInfo *encodeInfo)
{
    char buffer[32];

    // Read 32 bytes from image
    if(fread(buffer, 32, 1, encodeInfo->fptr_src_image) != 1)
        return e_failure;

    // Encode the integer into LSB
    encode_int_to_lsb(extn_size, buffer);

    // Write back to image
    if(fwrite(buffer, 32, 1, encodeInfo->fptr_stego_image) != 1)
        return e_failure;

    return e_success;
}

/* 
 * Encode a 32-bit integer into LSB of image bytes
 */
Status encode_int_to_lsb(long data, char *image_buffer)
{
    int j = 31; // Start from MSB

    for(int i = 0; i < 32; i++)
    {
        int bit = (data >> j) & 1;          // Get bit
        image_buffer[i] = (image_buffer[i] & ~1) | bit; // Set LSB
        j--;
    }

    return e_success;
}

/* 
 * Encode secret file extension
 */
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encodeInfo)
{
    return encode_data_to_image(file_extn, strlen(file_extn), encodeInfo->fptr_src_image, encodeInfo->fptr_stego_image);
}

/* 
 * Encode size of secret file
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encodeInfo)
{
    char buffer[32];

    if(fread(buffer, 32, 1, encodeInfo->fptr_src_image) != 1)
        return e_failure;

    encode_int_to_lsb(file_size, buffer);

    if(fwrite(buffer, 32, 1, encodeInfo->fptr_stego_image) != 1)
        return e_failure;

    return e_success;
}

/* 
 * Encode the actual secret file data into image
 */
Status encode_secret_file_data(EncodeInfo *encodeInfo)
{
    // Move pointer to start of secret file
    rewind(encodeInfo->fptr_secret);

    char file_data[encodeInfo->size_secret_file];

    // Read secret file into buffer
    if(fread(file_data, encodeInfo->size_secret_file, 1, encodeInfo->fptr_secret) != 1)
        return e_failure;

    // Encode the data into image
    return encode_data_to_image(file_data, encodeInfo->size_secret_file, encodeInfo->fptr_src_image, encodeInfo->fptr_stego_image);
}

/* 
 * Copy remaining bytes from source image to destination image
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    // Read remaining bytes one by one and write to destination
    while(fread(&ch, 1, 1, fptr_src) == 1)
    {
        if(fwrite(&ch, 1, 1, fptr_dest) != 1)
            return e_failure;
    }

    return e_success;
}

/* 
 * Close all files used during encoding
 */
Status close_encode_files(EncodeInfo *encodeInfo)
{
    int flag = 0;

    if(encodeInfo->fptr_src_image) { fclose(encodeInfo->fptr_src_image); flag = 1; }
    if(encodeInfo->fptr_secret)    { fclose(encodeInfo->fptr_secret);    flag = 1; }
    if(encodeInfo->fptr_stego_image){ fclose(encodeInfo->fptr_stego_image); flag = 1; }

    if(flag)
        printf(GREEN "[INFO] Files closed successfully\n" RESET);

    return e_success;
}


Status do_encoding(EncodeInfo *encodeInfo)
{
    // Step 1: Open source and destination files
    if (open_files(encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to open files\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Files opened successfully\n" RESET);

    // Step 2: Ensure the image has enough space to hide the secret file
    if (check_capacity(encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Insufficient capacity to encode.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Capacity check passed.\n" RESET);

    // Step 3: Copy BMP header from original image to stego image
    if (copy_bmp_header(encodeInfo->fptr_src_image, encodeInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to copy BMP header.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] BMP header copied successfully.\n" RESET);

    // Step 4: Encode a magic string to mark the start of hidden data
    if (encode_magic_string(MAGIC_STRING, encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to encode magic string.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Magic string encoded successfully.\n" RESET);

    // Step 5: Encode the length of the secret file's extension
    if (encode_secret_file_extn_size(strlen(encodeInfo->extn_secret_file), encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to encode extension size.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Extension size encoded successfully.\n" RESET);

    // Step 6: Encode the secret file's extension (e.g., .txt, .pdf)
    if (encode_secret_file_extn(encodeInfo->extn_secret_file, encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to encode extension.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Extension encoded successfully.\n" RESET);

    // Step 7: Encode the size of the secret file
    if (encode_secret_file_size(encodeInfo->size_secret_file, encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to encode file size.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] File size encoded successfully.\n" RESET);

    // Step 8: Encode the actual content/data of the secret file
    if (encode_secret_file_data(encodeInfo) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to encode file data.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] File data encoded successfully.\n" RESET);

    // Step 9: Copy any remaining image data from source to stego image
    if (copy_remaining_img_data(encodeInfo->fptr_src_image, encodeInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr, RED "ERROR: Failed to copy remaining image data.\n" RESET);
        return e_failure;
    }
    printf(GREEN "[INFO] Remaining image data copied successfully.\n" RESET);

    
    

    return e_success;
}
