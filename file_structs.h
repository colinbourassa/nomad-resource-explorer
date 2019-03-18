/* Structs used by Nomad (Gametek) data files */

#include <stdint.h>

typedef struct
{
  uint16_t bitfield;     // 00-01
  uint16_t unknown_a;    // 02-03
  uint16_t unknown_b;    // 04-05
  uint16_t size;         // 06-07
  uint16_t unknown_c;    // 08-09
  char     filename[14]; // 0A-17
  uint32_t offset;       // 18-1B
} dat_index_entry;

