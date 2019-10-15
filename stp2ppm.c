/*
 * This utility reads an STP image file that has already been through the LZ decompressor,
 * expands its RLE data, and indexes into the supplied palette file to write a P3 PPM image.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PPM_PIXELS_PER_LINE 4
#define LARGE_PAL_FILESIZE  771

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} palette_entry;

/*
 * Upconverts 6-bit VGA palette data to the full 8-bit range.
 */
void convert6BitPalette(uint8_t* paldata, long pal_filesize)
{
  long idx;

  // palette data actually starts on the fourth byte
  for (idx = 3; idx < pal_filesize; idx++)
  {
    // leftshift the six bits of data, and replicate the two high bits
    // in the low positions
    paldata[idx] = (paldata[idx] << 2) | (paldata[idx] >> 4);
  }
}

int main(int argc, char** argv)
{
  // handles for the input (.stp) image, input palette, and output PPM
  FILE* fd_img = 0;
  FILE* fd_pal = 0;
  FILE* fd_out = 0;

  // read buffers for the input
  uint8_t* imgdata = NULL;
  uint8_t* paldata = NULL;
  palette_entry* palette = NULL;

  uint16_t* width = 0;
  uint16_t* height = 0;
  uint32_t  pixelcount = 0;

  uint32_t color_count = 0;
  uint_fast16_t pixel_index = 0;
  uint8_t pal_index;

  long pal_filesize = 0;
  long img_filesize = 0;
  bool status = true;

  uint8_t* outputbuf = NULL;

  // buffer position tracking
  uint_fast32_t inputpos = 0;
  uint_fast32_t outputpos = 0;
  uint_fast32_t endptr = 0;
  uint8_t rlebyte = 0;

  // number of pixels that appear to be indexing beyond the palette
  uint_fast32_t index_error_count = 0;

  if (argc < 4)
  {
    printf("Nomad STP to PPM image converter\n"
           "Usage: %s <stp_image_file> <palette_file> <output>\n", argv[0]);
    return 0;
  }

  fd_img = fopen(argv[1], "r");
  fd_pal = fopen(argv[2], "r");
  fd_out = fopen(argv[3], "w");

  if (fd_img && fd_pal && fd_out)
  {
    // get the image filesize and allocate a buffer
    fseek(fd_img, 0L, SEEK_END);
    img_filesize = ftell(fd_img);
    rewind(fd_img);
    if (img_filesize > 0)
    {
      imgdata = malloc(img_filesize);
    }

    // get the palette filesize and allocate a buffer
    fseek(fd_pal, 0L, SEEK_END);
    pal_filesize = ftell(fd_pal);
    rewind(fd_pal);
    if (pal_filesize > 0)
    {
      paldata = malloc(pal_filesize);
    }

    if (imgdata && paldata)
    {
      printf("Reading %lu bytes of palette data.\n", pal_filesize);
      status = (fread(paldata, sizeof(uint8_t), pal_filesize, fd_pal) == pal_filesize);
      if (status)
      {
        convert6BitPalette(paldata, pal_filesize);
      }
      else
      {
        fprintf(stderr, "Failed to read palette file.\n");
      }
    }
    else
    {
      fprintf(stderr, "Failed to allocate read buffers.\n");
      status = false;
    }

    if (status)
    {
      printf("Reading %lu bytes from source data.\n", img_filesize);
      status = (fread(imgdata, sizeof(uint8_t), img_filesize, fd_img) == img_filesize);
      if (!status)
      {
        fprintf(stderr, "Failed to read image file.\n");
      }
    }

    if (status)
    {
      // FIXME: the source data will always be little-endian, so this code
      // should be modified to detect when byteswapping is necessary
      width = (uint16_t*)&imgdata[0];
      height = (uint16_t*)&imgdata[2];

      pixelcount = (*width) * (*height);
      outputbuf = malloc(pixelcount);

      if (outputbuf == NULL)
      {
        status = false;
        fprintf(stderr, "Failed to allocate %lu bytes for output buffer.\n", pixelcount * sizeof(palette_entry));
      }
    }

    if (status)
    {
      // STP image data begins at byte index 8
      inputpos = 8;
      outputpos = 0;

      // Iterate through all the chunks in the input (as long as there is space in the output).
      // At the end of this loop, outputbuf will contain pixelcount single-byte elements,
      // each one being an index into the palette data.
      while ((inputpos < img_filesize) && (outputpos < pixelcount))
      {
        rlebyte = imgdata[inputpos++];

        if (rlebyte & 0x80)
        {
          // bit 7 is set, so this is moving the output pointer ahead,
          // leaving the default value (0x00) in the skipped locations
          endptr = outputpos + (rlebyte & 0x7F);
          while ((outputpos < endptr) && (outputpos < pixelcount))
          {
            outputbuf[outputpos++] = 0x00;
          }
        }
        else if (rlebyte & 0x40)
        {
          // Bit 7 is clear and 6 is set, so this is a repeating sequence of a single byte.
          // (We only need to read one input byte for this RLE sequence, so verify that
          // the input pointer is still within the buffer range.)
          if (inputpos < img_filesize)
          {
            endptr = outputpos + (rlebyte & 0x3F);
            while ((outputpos < endptr) && (outputpos < pixelcount))
            {
              outputbuf[outputpos++] = imgdata[inputpos];
            }
          }

          // advance the input once more so that we read the next RLE byte at the top of the loop
          inputpos++;
        }
        else
        {
          // bits 7 and 6 are clear, so this is a byte sequence copy from the input
          endptr = outputpos + rlebyte;
          while ((outputpos < endptr) && (outputpos < pixelcount) && (inputpos < img_filesize))
          {
            outputbuf[outputpos++] = imgdata[inputpos++];
          }
        }
      }
    }

    if ((inputpos >= img_filesize) && (outputpos < pixelcount))
    {
      status = false;
      fprintf(stderr, "Error: finished processing input, but not all pixels have been accounted for in output.\n");
    }

    if ((outputpos >= pixelcount) && (inputpos < img_filesize))
    {
      status = false;
      fprintf(stderr, "Error: out of space in output buffer (%d pixels), but data remains in input"
              "(%lu of %lu bytes processed.\n", pixelcount, inputpos, img_filesize);
    }

    if (status)
    {
      // TODO: The second byte of the palette file is the VGA palette index
      // at which Nomad will start to populate the color data. This behavior
      // should probably be accounted for here -- maybe include a routine
      // that pre-populates a 256-color VGA palette with the entries normally
      // created by the BIOS. This way, any of the palette data files that
      // start filling data at some index other than zero will still result
      // in a complete palette.

      // FIXME: This is currently hardcoded to point to the fourth byte in the
      // palette data, since the first triplet is actually not a palette entry
      // but other information (sometimes including a color count.)
      palette = (palette_entry*)&paldata[3];

      // the smaller palettes seem to store the color count in the third byte
      color_count = paldata[2];

      // ...but the larger palettes leave that byte zeroed and always contain 256 colors
      if ((color_count == 0) && (pal_filesize == LARGE_PAL_FILESIZE))
      {
        color_count = 256;
      }

      fprintf(fd_out, "P3\n%d %d\n255", *width, *height);

      for (pixel_index = 0; pixel_index < pixelcount; ++pixel_index)
      {
        if (pixel_index % PPM_PIXELS_PER_LINE == 0)
        {
          fprintf(fd_out, "\n");
        }
        pal_index = outputbuf[pixel_index];

        if (pal_index < color_count)
        {
          fprintf(fd_out, "%03d %03d %03d   ", palette[pal_index].r, palette[pal_index].g, palette[pal_index].b);
        }
        else
        {
          index_error_count++;
        }
      }

      if (index_error_count)
      {
        fprintf(stderr, "Error: %lu pixels appeared to be indexing beyond the palette size of %d colors.\n",
                index_error_count, color_count);
      }
      else
      {
        printf("Done.\n");
      }
    }

  } // endif files were opened successfully

  if (imgdata)
  {
    free(imgdata);
  }

  if (paldata)
  {
    free(paldata);
  }

  if (fd_img)
  {
    fclose(fd_img);
  }

  if (fd_pal)
  {
    fclose(fd_pal);
  }

  if (fd_out)
  {
    fclose(fd_out);
  }

  return (status ? 0 : 1);
}

