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

#include "mcchunk.h"

#include <string.h>
#include <endian.h>

regionfile* open_regionfile(char* filename) {
  FILE* f = fopen(filename, "rb");
  if (f) {
    regionfile* region = malloc(sizeof(regionfile));
    if (fread(region->offsets, 4, SECTOR_INTS, f) != SECTOR_INTS)
      goto error;
    size_t i;
    for (i = 0; i < SECTOR_INTS; i++)
      region->offsets[i] = be32toh(region->offsets[i]);
    if (fread(region->timestamps, 4, SECTOR_INTS, f) != SECTOR_INTS)
      goto error;
    region->filename = strdup(filename);
    fclose(f);
    return region;
error:
    free(region);
    fclose(f);
  }
  return NULL;
};

size_t count_chunks(regionfile* region) {
  if (region) {
    size_t i, output = 0;
    for (i = 0; i < SECTOR_INTS; i++) {
      if (region->offsets[i] != 0)
        output++;
    }
    return output;
  }
  return 0;
};

void for_each_chunk(regionfile* region, void *function(nbt_node* node)) {
  if (!region || !function)
    return;
  FILE* f = fopen(region->filename, "rb");
  size_t i;
  for (i = 0; i < SECTOR_INTS; i++) {
    uint32_t offset = region->offsets[i];
    if (offset == 0)
      continue;
    uint32_t numSectors = offset & 0xff;
    if (numSectors == 0)
      continue;
    uint32_t sectorStart = offset >> 8;
    if (f && fseek(f, sectorStart*SECTOR_BYTES, SEEK_SET) == 0) {
      unsigned char buf[numSectors*SECTOR_BYTES];
      if (fread(buf, 1, sizeof(buf), f) == sizeof(buf)) {
        size_t size = be32toh((uint32_t) (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)));
        nbt_node* node = nbt_parse_compressed(buf+5, size);
        function(node);
        nbt_free(node);
      }
    }
  }
  fclose(f);
};

void free_region(regionfile* region) {
  if (region) {
    free(region->filename);
    free(region);
  }
};

int region_contains_chunk(regionfile* region, int32_t cx, int32_t cz) {
  if (!region)
    return 0;
  cx &= 0x1f;
  cz &= 0x1f;
  uint32_t offset = region->offsets[cx + cz * 32];
  if (offset == 0)
    return 0;
  return 1;
};

nbt_node* get_raw_chunk(regionfile* region, int32_t cx, int32_t cz) {
  if (!region)
    return NULL;
  cx &= 0x1f;
  cz &= 0x1f;
  uint32_t offset = region->offsets[cx + cz * 32];
  if (offset == 0)
    return NULL;
  uint32_t numSectors = offset & 0xff;
  if (numSectors == 0)
    return NULL;
  uint32_t sectorStart = offset >> 8;
  FILE* f = fopen(region->filename, "rb");
  nbt_node* output = NULL;
  if (f && fseek(f, sectorStart*SECTOR_BYTES, SEEK_SET) == 0) {
    unsigned char buf[numSectors*SECTOR_BYTES];
    if (fread(buf, 1, sizeof(buf), f) == sizeof(buf)) { /* First 4 bytes are the length */
      size_t size = be32toh((uint32_t) (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)));
      output = nbt_parse_compressed(buf+5, size); /* Fifth byte is the compression algorithm */
    }                                             /* But we don't need that as cNBT will figure that out */
  }
  fclose(f);
  return output;
};

char* determine_region_file(int32_t cx, int32_t cz) {
  char buf[BUFSIZ];
  snprintf(buf, sizeof(buf), "r.%d.%d.mca", cx >> 5, cz >> 5);
  return strdup(buf);
};