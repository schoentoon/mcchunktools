/*  mcchunktools
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nbt.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <endian.h>

#define SECTOR_BYTES 4096
#define SECTOR_INTS 1024

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Requires at least 1 argument!\n");
    return 1;
  }
  FILE* region = fopen(argv[1], "rb+");
  if (region) {
    uint32_t offsets[SECTOR_INTS];
    if (fread(offsets, 4, SECTOR_INTS, region) != SECTOR_INTS)
      return 1;
    size_t i;
    for (i = 0; i < SECTOR_INTS; i++)
      offsets[i] = be32toh(offsets[i]);
    uint32_t timestamps[SECTOR_INTS];
    if (fread(timestamps, 4, SECTOR_INTS, region) != SECTOR_INTS)
      return 1;
    for (i = 0; i < SECTOR_INTS; i++) {
      if (offsets[i] == 0)
        continue;
      uint32_t sectorStart = offsets[i] >> 8;
      uint32_t numSectors = offsets[i] & 0xff;
      if (numSectors == 0)
        continue;
      if (fseek(region, sectorStart*SECTOR_BYTES, SEEK_SET) == 0) {
        unsigned char buf[numSectors*SECTOR_BYTES];
        if (fread(buf, 1, sizeof(buf), region) == sizeof(buf)) {
          size_t size = be32toh((uint32_t) buf);
          nbt_node* node = nbt_parse_compressed(buf+5, size);
          if (node) {
            char* dump = nbt_dump_ascii(node);
            fprintf(stdout, "%s\n", dump);
            free(dump);
          } else {
            fprintf(stderr, "%s\n", nbt_error_to_string(errno));
          }
          nbt_free(node);
          return 0;
        }
      }
    }
  }
  fclose(region);
  return 0;
}