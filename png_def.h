//program validates PNG and displays dimensions
#include <stdio.h> //header file defines core inputs and outputs 

#include <stdbool.h> 
#include <stdio.h>   
#include <stdlib.h>  /* for malloc() */
#include <errno.h>   
#include <string.h>  
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
// #include "zutil.h" 
#include "crc.h"  

//indicates if its a png file or not
//indicates dimensions
//indicates any crc errors

/******************************************************************************
 * DEFINED MACROS
 *****************************************************************************/

#define PNG_SIG_SIZE    8 /* number of bytes of png image signature data */
#define CHUNK_LEN_SIZE  4 /* chunk length field size in bytes */          
#define CHUNK_TYPE_SIZE 4 /* chunk type field size in bytes */
#define CHUNK_CRC_SIZE  4 /* chunk CRC field size in bytes */
#define DATA_IHDR_SIZE 13 /* IHDR chunk data field size */
#define MAX_INPUT_SIZE (2 * 1024 *1024) //letting the maximum input file size be 2MB

/******************************************************************************
 * STRUCTURES and TYPEDEFS 
 *****************************************************************************/
typedef unsigned char U8; //typedef renames "unsigned char" to U8 
typedef unsigned int  U32;

typedef struct chunk {
    U32 length;  /* length of data in the chunk, host byte order */
    U8  type[4]; /* chunk type */
    U8  *p_data; /* pointer to location where the actual data are */
    U32 crc;     /* CRC field  */
} *chunk_p;

typedef struct data_IHDR {// IHDR chunk data 
    U32 width;        /* width in pixels, big endian   */
    U32 height;       /* height in pixels, big endian  */
    U8  bit_depth;    /* num of bits per sample or per palette index.
                         valid values are: 1, 2, 4, 8, 16 */
    U8  color_type;   /* =0: Grayscale; =2: Truecolor; =3 Indexed-color
                         =4: Greyscale with alpha; =6: Truecolor with alpha */
    U8  compression;  /* only method 0 is defined for now */
    U8  filter;       /* only method 0 is defined for now */
    U8  interlace;    /* =0: no interlace; =1: Adam7 interlace */
} *data_IHDR_p;

/* A simple PNG file format, three chunks only*/
typedef struct simple_PNG {
    struct chunk *p_IHDR;
    struct chunk *p_IDAT;  /* only handles one IDAT chunk */  
    struct chunk *p_IEND;
} *simple_PNG_p;

/******************************************************************************
 * FUNCTION PROTOTYPES 
 *****************************************************************************/
int is_png(U8 *buf); //i modified this beacuse i didnt need size_t n in the arguments
int get_png_height(U8 *buf);
int get_png_width(U8 *buf);
// int get_png_data_IHDR(struct data_IHDR *out, FILE *fp, long offset, int whence);
int check_crc(U8 *buf, FILE *f);
int convert_4byte_to_int(U8 *buf);
int byte_to_int(U8 *buf, size_t size);

/******************************************************************************
 * FUNCTION  DEFINITIONS
 *****************************************************************************/

int is_png(U8 *buf)
{
    // identify if file has correct sequence to be png
    if (buf[0] != 0x89)
    {
        return 0;
    }
    if (buf[1] != 0x50)
    { // P
        return 0;
    }
    if (buf[2] != 0x4E)
    { // N
        return 0;
    }
    if (buf[3] != 0x47)
    { // G
        return 0;
    }
    if (buf[4] != 0x0D)
    {
        return 0;
    }
    if (buf[5] != 0x0A)
    {
        return 0;
    }
    if (buf[6] != 0x1A)
    {
        return 0;
    }
    if (buf[7] != 0x0A)
    {
        return 0;
    }

    return 1;
}

int byte_to_int(U8 *buf, size_t size)
{ // takes in big endian and returns its little endian integer value

    U32 data_length = 0;

    for (int i = 0; i < size; i++)
    { // reverse order
        data_length = (data_length << 8) | buf[i];
    }
    return data_length;
}

int convert_4byte_to_int(U8 *buf) {
    int num = (buf[0] << 24) |
                (buf[1] << 16) |
                (buf[2] << 8)  |
                buf[3];
    return num;
}


int get_png_width(U8 *buf){
    //width is bytes 8-11 in IHDR chunk
    U32 width = buf[8];

    for (int i = 8; i < 11; i++)
    { // reverse order
        width = width << 8 | buf[i + 1];
    }

    return width;
}

int get_png_height(U8 *buf){
    //height is bytes 12 to 15 in IHDR chunk
    U32 height = buf[12];

    for (int i = 12; i < 15; i++)
    { // reverse order
        height = height << 8 | buf[i + 1];
    }

    return height;
}


int check_crc(U8 *buf, FILE *f)
{
    // check crc of the file
    // return 0 if crc is correct, negative num if not

    // check IHDR crc
    fseek(f, 12, SEEK_SET);
    if(fread(buf, 1, DATA_IHDR_SIZE + 4, f) != DATA_IHDR_SIZE + 4){
        perror("fread IHDR");
        return -1;
    }
    unsigned long computed_crc = crc(buf, (DATA_IHDR_SIZE + 4));

    size_t size = fread(buf, 1, 4, f); // read in CRC
    unsigned int crc_val = byte_to_int(buf, size);

    if (computed_crc != crc_val)
    {
        return -1;
    }

    // check IDAT crc
    size = fread(buf, 1, 4, f); // read length
    unsigned int IDAT_length = byte_to_int(buf, size);

    if (fread(buf, 1, IDAT_length + 4, f) != (IDAT_length +4)){// read Type and Data into buffer
        perror("fread IDAT");
        return -2;
    }  
    computed_crc = crc(buf, IDAT_length + 4);

    size = fread(buf, 1, 4, f); // read the CRC value into buffer
    crc_val = byte_to_int(buf, size);

    if (computed_crc != crc_val)
    {
        return -2;
    }

    // check IEND crc
    fseek(f, 4, SEEK_CUR); // length = 0, so move pointer to Type
    if (fread(buf, 1, 4, f) != 4)
    { // read Type into buffer
        perror("fread IEND");
        return -3;
    }
    computed_crc = crc(buf, 4);

    size = fread(buf, 1, 4, f); // read CRC
    crc_val = byte_to_int(buf, size);

    if (computed_crc != crc_val)
    {
        return -3;
    }

    return 0;
}