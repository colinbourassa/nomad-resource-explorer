/**
 * Extracts files from the .DAT containers used by the 1994 space exploration
 * game "Nomad" (Gametek / Intense! / Papyrus).
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "file_structs.h"

#define MAX_FILENAME_LEN 13 // space in the index entries for 13 chars + null term

bool decode_dat(const char* filename);

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
 */
bool decode_dat(const char* datfilename)
{
  bool status = false;
  uint8_t buffer[64 * 1024];
  uint16_t filecount = 0;
  uint32_t indexsize = 0;
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

      printf("Extracting index %u ('%s', size %u, offset 0x%X)...\n",
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
        if (fread(buffer, 1, index[indexpos].size, fd) != index[indexpos].size)
        {
          status = false;
          fprintf(stderr, "Error: failed to read %d bytes from offset %u.\n",
                  index[indexpos].size, index[indexpos].offset);
        }
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
        if (fwrite(buffer, 1, index[indexpos].size, fd_target) == index[indexpos].size)
        {
          fclose(fd_target);
        }
        else
        {
          status = false;
          fprintf(stderr, "Error: failed to write %d bytes to '%s'.\n",
                  index[indexpos].size, index[indexpos].filename);
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

