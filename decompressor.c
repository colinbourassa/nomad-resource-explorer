#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

void printbyte(uint8_t x)
{
  if (isprint(x))
  {
    printf("%c", x);
  }
  else
  {
    if (x == 0) printf("<00>\n");
    else printf("<%02X>", x);
  }
  fflush(stdout);
}

int main(int argc, char **argv)
{
  uint8_t buffer[0x1000];
  uint16_t bufpos = 0xFEE;
  uint8_t codeword[2];
  uint8_t flagbyte = 0;
  uint8_t decodebyte = 0;
  uint8_t chunkIndex = 0;
  uint8_t byteIndexInChunk = 0;
  uint8_t chunkSize = 0;
  uint16_t chunkSource = 0;
  bool done = false;

  if (argc < 3)
  {
    printf("Usage: <inputfile> <outputfile>\n");
    return 0;
  }

  int inputfd = open(argv[1], 0, O_RDONLY);
  int outputfd = creat(argv[2], 0664);

  if (inputfd < 0)
  {
    fprintf(stderr, "Error opening file '%s' for reading.\n", argv[1]);
    return 1;
  }

  while (!done)
  {
    if (read(inputfd, &flagbyte, 1) != 1)
    {
      done = true;
    }
    else
    {
      for (chunkIndex = 0; (chunkIndex < 8) && !done; chunkIndex++)
      {
        if ((flagbyte & (1 << chunkIndex)) != 0)
        {
          // single-byte literal
          if (read(inputfd, &decodebyte, 1) != 1)
          {
            done = true;
          }
          else
          {
            write(outputfd, &decodebyte, 1);
            //printbyte(decodebyte);

            buffer[bufpos] = decodebyte;
            if (++bufpos >= 0x1000)
            {
              bufpos = 0;
            }
          }
        }
        else
        {
          // two-byte reference to a sequence in the circular buffer
          if (read(inputfd, codeword, 2) != 2)
          {
            done = true;
          }
          else
          {
            chunkSize = ((codeword[1] & 0xF0) >> 4) + 3;
            chunkSource = ((codeword[1] & 0x0F) << 8) | codeword[0];

            for (byteIndexInChunk = 0; byteIndexInChunk < chunkSize; byteIndexInChunk++)
            {
              decodebyte = buffer[chunkSource];
              write(outputfd, &decodebyte, 1);
              //printbyte(decodebyte);

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
        }
      }
    }
  }

  close(inputfd);
  close(outputfd);

  return 0;
}

