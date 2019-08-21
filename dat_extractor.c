/**
 * Extracts files from the .DAT containers used by the 1993 space exploration
 * game "Nomad" (Gametek / Intense! / Papyrus).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "file_structs.h"

#define MAX_FILENAME_LEN 13 // space in the index entries for 13 chars + null term
#define COMPRESSED_BUF_LEN 65536
#define DECOMPRESSED_BUF_LEN 1048576

bool decode_dat(const char* filename);
int  lz_inflate(uint8_t* inputbuf,  uint16_t inputbuf_len,
                uint8_t* outputbuf, uint32_t outputbuf_len);

int main (int argc, char** argv)
{
  int status = 0;

  if (argc < 2)
  {
    printf("Usage: %s <dat_file>\n", argv[0]);
    return status;
  }

  status = decode_dat(argv[1]) ? 0 : -1;

  return status;
}

/**
 * Parses the index of a Nomad .DAT file and extracts the constituent data files.
 * As each file is extracted, it is also decompressed with the LZ algorithm.
 */
bool decode_dat(const char* datfilename)
{
  bool status = false;
  uint8_t buffer_compressed[COMPRESSED_BUF_LEN];
  uint8_t buffer_decompressed[DECOMPRESSED_BUF_LEN];
  uint16_t filecount = 0;
  uint32_t indexsize = 0;
  uint32_t decompressed_size = 0;
  dat_index_entry* index = 0;
  FILE* fd_target;
  FILE* fd = fopen(datfilename, "r");

  if (fd > 0)
  {
    status = true;
  }
  else
  {
    fprintf(stderr, "Error: failed to open '%s'.\n", datfilename);
  }

  if (status)
  {
    // read the 16-bit count of contained files and allocate sufficient
    // space to buffer the index entries that describe them
    if (fread(&filecount, sizeof(filecount), 1, fd) == 1)
    {
      indexsize = filecount * sizeof(dat_index_entry);
      index = malloc(indexsize);
      status = (index != 0);
    }
    else
    {
      status = false;
    }
  }

  if (status)
  {
    // read the entire index header
    if (fread(index, sizeof(dat_index_entry), filecount, fd) != filecount)
    {
      status = false;
      fprintf(stderr, "Error: failed to read %d index entries from '%s'.\n", filecount, datfilename);
    }
  }

  if (status)
  {
    // iterate through each index entry
    uint16_t indexpos = 0;
    while ((indexpos < filecount) && status)
    {
      char filename[MAX_FILENAME_LEN + 1];
      strncpy(filename, index[indexpos].filename, MAX_FILENAME_LEN);
      filename[MAX_FILENAME_LEN] = '\0';

      printf("Extracting index %u ('%s', compressed size %u, offset 0x%X)...\n",
             indexpos, filename, index[indexpos].size, index[indexpos].offset);

      // seek to the position at which the contained file starts
      if (fseek(fd, index[indexpos].offset, SEEK_SET) != 0)
      {
        status = false;
        fprintf(stderr, "Error: failed to seek to offset %u.\n", index[indexpos].offset);
      }

      if (status)
      {
        // read the contained file
        if (fread(buffer_compressed, 1, index[indexpos].size, fd) != index[indexpos].size)
        {
          status = false;
          fprintf(stderr, "Error: failed to read %d bytes from offset %u.\n",
                  index[indexpos].size, index[indexpos].offset);
        }
      }

      if (status)
      {
        decompressed_size = lz_inflate(buffer_compressed, index[indexpos].size,
                                       buffer_decompressed, DECOMPRESSED_BUF_LEN);
        status = (decompressed_size > 0);
      }

      if (status)
      {
        // open a separate file to which the contained file's data will be written
        fd_target = fopen(index[indexpos].filename, "w");
        if (fd_target < 0)
        {
          status = false;
          fprintf(stderr, "Error: failed to open '%s' for writing.\n", index[indexpos].filename);
        }
      }

      if (status)
      {
        // write the buffered data
        if (fwrite(buffer_decompressed, 1, decompressed_size, fd_target) == decompressed_size)
        {
          fclose(fd_target);
        }
        else
        {
          status = false;
          fprintf(stderr, "Error: failed to write %d bytes to '%s'.\n",
                  decompressed_size, index[indexpos].filename);
        }
      }

      indexpos++;
    }
  }

  if (index)
  {
    free(index);
  }

  if (fd > 0)
  {
    fclose(fd);
  }

  return status;
}

/**
 * Inflates data from an 8-bit LZ-compressed buffer.
 */
int lz_inflate(uint8_t* input,  uint16_t inputbuf_len,
               uint8_t* output, uint32_t outputbuf_len)
{
  uint8_t buffer[0x1000];
  uint16_t bufpos = 0xFEE;
  uint16_t inputpos = 0;
  uint32_t outputpos = 0;
  uint8_t codeword[2];
  uint8_t flagbyte = 0;
  uint8_t decodebyte = 0;
  uint8_t chunkIndex = 0;
  uint8_t byteIndexInChunk = 0;
  uint8_t chunkSize = 0;
  uint16_t chunkSource = 0;

  while (inputpos < inputbuf_len)
  {
    flagbyte = input[inputpos++];

    for (chunkIndex = 0; (chunkIndex < 8) && (inputpos < inputbuf_len); chunkIndex++)
    {
      if ((flagbyte & (1 << chunkIndex)) != 0)
      {
        // single-byte literal
        decodebyte = input[inputpos++];
        output[outputpos++] = decodebyte;

        buffer[bufpos] = decodebyte;
        if (++bufpos >= 0x1000)
        {
          bufpos = 0;
        }
      }
      else
      {
        // two-byte reference to a sequence in the circular buffer
        codeword[0] = input[inputpos++];
        codeword[1] = input[inputpos++];

        chunkSize   = ((codeword[1] & 0xF0) >> 4) + 3;
        chunkSource = ((codeword[1] & 0x0F) << 8) | codeword[0];

        for (byteIndexInChunk = 0; byteIndexInChunk < chunkSize; byteIndexInChunk++)
        {
          decodebyte = buffer[chunkSource];
          output[outputpos++] = decodebyte;

          if (++chunkSource >= 0x1000)
          {
            chunkSource = 0;
          }

          buffer[bufpos] = decodebyte;
          if (++bufpos >= 0x1000)
          {
            bufpos = 0;
          }
        }
      }
    } // end for chunk index 0-7
  }

  return outputpos;
}

