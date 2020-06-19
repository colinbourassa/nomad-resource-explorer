/**
 * Extracts files from the .DAT containers used by the 1993 space exploration
 * game "Nomad" (Gametek / Intense! / Papyrus).
 *
 * TODO:
 * Some files (fullscreen LBM images) are stored with the first four bytes
 * uncompressed, but the remainder of the file compressed. The graphical
 * Nomad Resource Explorer utility has the ability to unpack these, but
 * that capability was never rolled back into this standalone utility.
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

#define MAX_FILENAME_LEN 13 // space in the index entries for 13 chars + null term
#define LZ_RINGBUF_SIZE 0x1000

#define INDEX_BITFIELD_OFFSET    0x00
#define INDEX_UNCOMP_SIZE_OFFSET 0x02
#define INDEX_COMP_SIZE_OFFSET   0x06
#define INDEX_FILENAME_OFFSET    0x0A
#define INDEX_OFFSET_OFFSET      0x18

typedef struct
{
  uint8_t data[0x1C];
} dat_index_entry;

bool decode_dat(const char* filename);
int  lz_inflate(uint8_t* inputbuf,  uint16_t inputbuf_len,
                uint8_t* outputbuf, uint32_t outputbuf_len,
                int skip_uncompressed_bytes);

int main (int argc, char** argv)
{
  int status = 0;

  if (argc < 2)
  {
    printf("Nomad DAT File Extractor\nUsage: %s <dat_file>\n", argv[0]);
    return status;
  }

  status = decode_dat(argv[1]) ? 0 : -1;

  return status;
}

/**
 * Parses the index of a Nomad .DAT file and extracts the constituent data files.
 * As each file is extracted, it is decompressed with the LZ algorithm if neccessary.
 */
bool decode_dat(const char* datfilename)
{
  bool status = false;
  uint8_t* buf_compressed = NULL;
  uint8_t* buf_decompressed = NULL;
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
      bool skipfile = false;
      char filename[MAX_FILENAME_LEN + 1];
      strncpy(filename, &(index[indexpos].data[INDEX_FILENAME_OFFSET]), MAX_FILENAME_LEN);
      filename[MAX_FILENAME_LEN] = '\0';

      uint32_t* compressed_size   = (uint32_t*)&(index[indexpos].data[INDEX_COMP_SIZE_OFFSET]);
      uint32_t* uncompressed_size = (uint32_t*)&(index[indexpos].data[INDEX_UNCOMP_SIZE_OFFSET]);
      uint32_t* offset            = (uint32_t*)&(index[indexpos].data[INDEX_OFFSET_OFFSET]);
      uint16_t* flags             = (uint16_t*)&(index[indexpos].data[INDEX_BITFIELD_OFFSET]);
      int skip_uncompressed_bytes = 0;

      // check whether this file has an uncompressed 4-byte header
      // (used only for fullscreen raw VGA images)
      if ((~*flags & 0x0004) && (*flags & 0x0100))
      {
        skip_uncompressed_bytes = 4;
      }

      if (!skipfile)
      {
        buf_compressed = malloc(*compressed_size + skip_uncompressed_bytes);
        buf_decompressed = malloc(*uncompressed_size + skip_uncompressed_bytes);

        if ((buf_compressed != NULL) && (buf_decompressed != NULL))
        {
          printf("Extracting index %u ('%s', %s, size %u, offset 0x%X)...\n",
              indexpos, filename, (*flags & 0x0100) ? "compressed" : "uncompressed",
              *compressed_size + skip_uncompressed_bytes, *offset);
        }
        else
        {
          status = false;
          if (buf_compressed == NULL)
          {
            fprintf(stderr, "Error: failed to allocate buffer of %d bytes for reading file at index %d ('%s').\n",
                *compressed_size + skip_uncompressed_bytes, indexpos, filename);
          }
          else if (buf_decompressed == NULL)
          {
            fprintf(stderr, "Error: failed to allocate buffer of %d bytes for decompressing file at index %d ('%s').\n",
                *uncompressed_size, indexpos, filename);
          }
        }
      }

      // seek to the position at which the contained file starts
      if (status && !skipfile)
      {
        if (fseek(fd, *offset, SEEK_SET) != 0)
        {
          status = false;
          fprintf(stderr, "Error: failed to seek to offset %08X.\n", *offset);
        }
      }

      if (status && !skipfile)
      {
        // read the contained file
        if (fread(buf_compressed, 1, *compressed_size + skip_uncompressed_bytes, fd) !=
            *compressed_size + skip_uncompressed_bytes)
        {
          status = false;
          fprintf(stderr, "Error: failed to read %d bytes from offset %08X.\n",
                  *compressed_size + skip_uncompressed_bytes, *offset);
        }
      }

      if (status && !skipfile)
      {
        // check the flags to see whether this file was actually stored compressed; only decompress if necessary
        if (*flags & 0x0100)
        {
          decompressed_size = lz_inflate(buf_compressed, *compressed_size + skip_uncompressed_bytes,
                                         buf_decompressed, *uncompressed_size + skip_uncompressed_bytes,
                                         skip_uncompressed_bytes);
          status = (decompressed_size > 0);

          if (decompressed_size != (*uncompressed_size + skip_uncompressed_bytes))
          {
            fprintf(stderr, "Warning: File at index %d (%s) was listed as having an uncompressed size of %d, but unpacked to %d bytes!\n",
                    indexpos, filename, *uncompressed_size, decompressed_size);
          }
        }
        else
        {
          if (*compressed_size == *uncompressed_size)
          {
            memcpy(buf_decompressed, buf_compressed, *compressed_size);
            decompressed_size = *compressed_size;
          }
          else
          {
            fprintf(stderr, "Warning: file at index %d (%s) is marked as uncompressed but compressed/decompressed"
                            " sizes do not match! Skipping.", indexpos, filename);
            skipfile = true;
          }
        }
      }

      if (status && !skipfile)
      {
        // open a separate file to which the contained file's data will be written
        fd_target = fopen(filename, "w");
        if (fd_target < 0)
        {
          status = false;
          fprintf(stderr, "Error: failed to open '%s' for writing.\n", filename);
        }
      }

      if (status && !skipfile)
      {
        // write the buffered data
        if (fwrite(buf_decompressed, 1, decompressed_size, fd_target) == decompressed_size)
        {
          fclose(fd_target);
        }
        else
        {
          status = false;
          fprintf(stderr, "Error: failed to write %d bytes to '%s'.\n", decompressed_size, filename);
        }
      }

      if (buf_compressed != NULL)
      {
        free(buf_compressed);
      }

      if (buf_decompressed != NULL)
      {
        free(buf_decompressed);
      }

      buf_compressed = NULL;
      buf_decompressed = NULL;
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
               uint8_t* output, uint32_t outputbuf_len,
               int skip_uncompressed_bytes)
{
  uint8_t buffer[LZ_RINGBUF_SIZE];
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

  memset(buffer, 0x20, LZ_RINGBUF_SIZE);

  while ((skip_uncompressed_bytes > 0) &&
         (inputpos < inputbuf_len) &&
         (outputpos < outputbuf_len))
  {
    output[outputpos++] = input[inputpos++];
    skip_uncompressed_bytes--;
  }

  while ((inputpos < inputbuf_len) && (outputpos < outputbuf_len))
  {
    flagbyte = input[inputpos++];

    chunkIndex = 0;
    while ((chunkIndex < 8) && (inputpos < inputbuf_len) && (outputpos < outputbuf_len))
    {
      if ((flagbyte & (1 << chunkIndex)) != 0)
      {
        // single-byte literal
        decodebyte = input[inputpos++];
        output[outputpos++] = decodebyte;

        buffer[bufpos] = decodebyte;
        if (++bufpos >= LZ_RINGBUF_SIZE)
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

        byteIndexInChunk = 0;
        while ((byteIndexInChunk < chunkSize) && (outputpos < outputbuf_len))
        {
          decodebyte = buffer[chunkSource];
          output[outputpos++] = decodebyte;

          if (++chunkSource >= LZ_RINGBUF_SIZE)
          {
            chunkSource = 0;
          }

          buffer[bufpos] = decodebyte;
          if (++bufpos >= LZ_RINGBUF_SIZE)
          {
            bufpos = 0;
          }

          byteIndexInChunk += 1;
        }

        if (byteIndexInChunk < chunkSize)
        {
          fprintf(stderr, "Error: reached end of output buffer with %d bytes left in chunk (from input offset %08X)\n",
                  chunkSize - byteIndexInChunk, inputpos - 1);
        }
      }

      chunkIndex += 1;
    } // end for chunk index 0-7
  }

  if (outputpos > outputbuf_len)
  {
    fprintf(stderr, "Error: output buffer overrun (%d vs %d)\n", outputpos, outputbuf_len);
  }

  return outputpos;
}

