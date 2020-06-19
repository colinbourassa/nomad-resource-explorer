/**
 * Creates a .DAT data archive for Nomad (GameTek, 1993).
 *
 * Although the original .DAT format allows LZ compression, it also
 * gave the option of storing files uncompressed. This utility only
 * uses uncompressed storage. The space saving is no longer really
 * necessary since we're not concerned about floppy disk distribution,
 * and it allows the code for this utility to be kept much simpler.
 * Additionally, uncompressed .DAT contents can be changed in-place,
 * allowing quick modifications to game data without needing to
 * repack the .DAT.
 *
 * Usage: dat_builder <dat-file-to-create> <list-file>
 *  The parameter <list-file> is the name of a plain text file with
 *  one filename per line. These are the files that will be packed
 *  together in the target .DAT.
 *
 * Note that you cannot arbitrarily change the set of files that
 * are packed into a given .DAT. The game executable expects that
 * TEST.DAT will contain GAME.PAL, and that INVENT.DAT will contain
 * inv0001.stp, etc.
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
#include <ctype.h>

#define MAX_NAME_LEN 14
#define EXT_LEN 4
#define DATA_BUF_MAX_SIZE (8 * 1024 * 1024)

// This attribute ensures packing on gcc; MS compilers will require something else
typedef struct __attribute__((packed)) dat_index_entry
{
  uint16_t flags;
  uint32_t compressed_size;
  uint32_t uncompressed_size;
  char filename[MAX_NAME_LEN];
  uint32_t start_offset;
} dat_index_entry;

/**
 * Gets the number of lines in the provided file.
 */
int get_linecount(const char* filename)
{
  FILE* fp = fopen(filename, "r");
  int linecount = 0;
  char c;

  if (fp)
  {
    for (c = getc(fp); c != EOF; c = getc(fp))
    {
      if (c == '\n')
      {
        linecount++;
      }
    }
    fclose(fp);
  }

  return linecount;
}

/**
 * Returns true if the provided filename's extension matches ".lbm" (case insensitive).
 * Returns false otherwise.
 */
bool is_lbm_image(const char* filename)
{
  bool is_lbm = false;
  const int len = strlen(filename);
  char extension[EXT_LEN + 1];
  int pos = 0;

  if (len > EXT_LEN)
  {
    memcpy(extension, filename + len - EXT_LEN, EXT_LEN);
    extension[EXT_LEN] = 0;

    for (pos = 0; pos < EXT_LEN; pos++)
    {
      extension[pos] = tolower(extension[pos]);
    }

    is_lbm = (strncmp(extension, ".lbm", EXT_LEN) == 0);
  }

  return is_lbm;
}

/**
 * Reads the provided file list and builds a .DAT file with the provided name.
 */
bool build_dat(const char* datfilename, const char* listfilename)
{
  FILE* listfile = NULL;
  FILE* datfile = NULL;
  FILE* infile = NULL;
  bool status = false;

  char inputline[MAX_NAME_LEN];
  int inputline_len = 0;
  struct stat info;
  dat_index_entry index_entry;

  int header_buf_pos = 0;
  int data_buf_pos = 0;
  long unsigned int header_buf_remaining = 0;
  int data_buf_remaining = 0;
  uint16_t filecount = get_linecount(listfilename);
  const int header_size = filecount * sizeof(dat_index_entry);

  uint8_t* data_buffer = (uint8_t*)malloc(DATA_BUF_MAX_SIZE);
  uint8_t* header_buffer = (uint8_t*)malloc(header_size);

  if (filecount && data_buffer && header_buffer)
  {
    listfile = fopen(listfilename, "r");

    if (listfile)
    {
      status = true;
    }
    else
    {
      fprintf(stderr, "Unable to open file list '%s'.\n", listfilename);
    }

    while (status && fgets(inputline, MAX_NAME_LEN, listfile) != NULL)
    {
      header_buf_remaining = header_size - header_buf_pos;
      data_buf_remaining = DATA_BUF_MAX_SIZE - data_buf_pos;

      inputline_len = strlen(inputline);
      if (inputline_len && inputline[inputline_len - 1] == '\n')
      {
        inputline[inputline_len - 1] = 0;
      }
      stat(inputline, &info);

      // if there's enough room in the data and header buffers
      if ((info.st_size <= data_buf_remaining) && (sizeof(dat_index_entry) <= header_buf_remaining))
      {
        infile = fopen(inputline, "r");
        if (infile)
        {
          if (fread(data_buffer + data_buf_pos, 1, info.st_size, infile) == (long unsigned int)info.st_size)
          {
            // The game executable expects raw VGA .lbm files to be stored with a different type code
            // and with an additional two 16-bit words of header (indicating width and height).
            // Since the dat_extractor utility creates these .lbm files with these header bytes
            // prepended, we need to substract the 4-byte difference here when storing it back.
            if (is_lbm_image(inputline))
            {
              index_entry.flags = 0x0001;
              index_entry.compressed_size = info.st_size - 4;
              index_entry.uncompressed_size = info.st_size - 4;
            }
            else
            {
              index_entry.flags = 0x0005;
              index_entry.compressed_size = info.st_size;
              index_entry.uncompressed_size = info.st_size;
            }

            index_entry.start_offset = 2 + header_size + data_buf_pos;
            memset(index_entry.filename, 0, MAX_NAME_LEN);
            memcpy(index_entry.filename, inputline, MAX_NAME_LEN);
            memcpy(header_buffer + header_buf_pos, &index_entry, sizeof(index_entry));

            header_buf_pos += sizeof(dat_index_entry);
            data_buf_pos += info.st_size;

            printf("Copied content of '%s', size %ld, type %d...\n", inputline, info.st_size, index_entry.flags);
          }
          else
          {
            fprintf(stderr, "Could not read %ld bytes from %s.\n", info.st_size, inputline);
            status = false;
          }
          fclose(infile);
        }
        else
        {
          fprintf(stderr, "Failed to open '%s'.\n", inputline);
          status = false;
        }
      }
      else
      {
        fprintf(stderr, "Insufficient buffer space.\n");
        status = false;
      }
    }

    if (status)
    {
      datfile = fopen(datfilename, "w");
      if (datfile)
      {
        printf("Writing a %ld-byte filecount word...\n", sizeof(filecount));
        fwrite(&filecount, 1, sizeof(filecount), datfile);
        printf("Writing %d bytes of header...\n", header_buf_pos);
        fwrite(header_buffer, header_buf_pos, 1, datfile);
        printf("Writing %d bytes of data...\n", data_buf_pos);
        fwrite(data_buffer,   data_buf_pos,   1, datfile);
        fclose(datfile);
        printf("Done.\n");
      }
      else
      {
        fprintf(stderr, "Failed to open '%s' for writing.\n", datfilename);
        status = false;
      }
    }

    fclose(listfile);

    free(data_buffer);
    free(header_buffer);
  }

  return status;
}

/**
 * Entry point
 */
int main (int argc, char** argv)
{
  int status = 0;

  if (argc < 3)
  {
    printf("Nomad DAT File Builder\nUsage: %s <dat_file_name> <file_list>\n", argv[0]);
    return status;
  }

  status = build_dat(argv[1], argv[2]) ? 0 : -1;

  return status;
}

