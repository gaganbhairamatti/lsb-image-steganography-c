# LSB Image Steganography using C

## Overview

This project implements Least Significant Bit (LSB) image steganography using the C programming language. It allows users to hide secret text data inside BMP image files and retrieve it later without visibly altering the image.

The project demonstrates low-level data manipulation using bitwise operations and file handling in C.

---

## System Architecture

### Encoding Process (Hiding Data)
Input Image → Read Pixel Data → Modify LSB Bits → Embed Secret Data → Generate Stego Image

### Decoding Process (Extracting Data)
Stego Image → Read Pixel Data → Extract LSB Bits → Reconstruct Data → Output Text

---

## Features

- Encode secret text into BMP image
- Decode hidden message from image
- Command-line interface
- LSB-based data hiding technique
- Efficient file handling in C
- No visible distortion in image

---

## Project Structure

```
lsb-image-steganography
│
├── main.c
├── encode.c
├── encode.h
├── decode.c
├── decode.h
│
├── common.h
├── types.h
│
├── images
│   └── beautiful.bmp
│
├── secret
│   └── secret.txt
│
└── README.md
```

---

## How It Works

### Encoding

- Reads BMP image file (`beautiful.bmp`)
- Reads secret text (`secret.txt`)
- Modifies Least Significant Bits of image pixels
- Generates encoded image (`stego.bmp`)

### Decoding

- Reads stego image (`stego.bmp`)
- Extracts LSB bits
- Reconstructs hidden message
- Stores output in (`decode_secret.txt`)

---

## Compilation & Run

```bash
gcc main.c encode.c decode.c -o stego
```

## Usage

### Encode Data

```bash
./stego -e beautiful.bmp secret.txt stego.bmp
```

### Decode Data

```bash
./stego -d stego.bmp decode_secret.txt
```

---

## Requirements

- GCC or any C compiler
- Uncompressed BMP image file (24-bit preferred)
- Command-line environment (Linux / macOS / Windows)
---

## Notes

- Only BMP images are supported
- Ensure the image has enough capacity to store the secret data
- Works entirely on file-level binary manipulation (no external communication protocols)

---

## Future Improvements

- Support for PNG/JPEG formats
- Add encryption before embedding
- GUI-based interface
- Increase embedding capacity

---

## Author

Developed by: Gagan Bhairamatti
