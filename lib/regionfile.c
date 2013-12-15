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

void for_each_chunk_raw(regionfile* region, raw_chunk_func function, void* context) {
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
        function(node, context);
        nbt_free(node);
      }
    }
  }
  fclose(f);
};

struct __for_each_chunk_struct {
  void* context;
  chunk_func function;
};

void __for_each_chunk(nbt_node* node, void* context) {
  struct __for_each_chunk_struct* data = context;
  chunk* c = nbt_to_chunk(node, 0);
  if (c) {
    data->function(c, data->context);
    free_chunk(c);
  }
};

void for_each_chunk(regionfile* region, chunk_func function, void* context) {
  struct __for_each_chunk_struct data;
  data.context = context;
  data.function = function;
  for_each_chunk_raw(region, __for_each_chunk, &data);
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

extern nbt_node* new_chunk_data_to_nbt(nbt_node* node, chunk* c);

int write_chunk(regionfile* region, int32_t cx, int32_t cz, nbt_node* raw, chunk* chunk) {
  if (region == NULL)
    return -1;
  if (raw == NULL)
    return -1;
  if (chunk != NULL) {
    raw = new_chunk_data_to_nbt(raw, chunk);
    if (raw == NULL)
      return -1;
  }
  cx &= 0x1f;
  cz &= 0x1f;
  uint32_t offset = region->offsets[cx + cz * 32];
  if (offset == 0)
    return -2;
  uint32_t numSectors = offset & 0xff;
  if (numSectors == 0)
    return -3;
  uint32_t sectorStart = offset >> 8;
  FILE* f = NULL;
  struct buffer buf = nbt_dump_compressed(raw, STRAT_INFLATE);
  uint8_t sectorsNeeded = (buf.len + 5) / SECTOR_BYTES + 1;
  if (sectorStart != 0 && numSectors >= sectorsNeeded) {
    f = fopen(region->filename, "rb+");
    if (f && fseek(f, sectorStart*SECTOR_BYTES, SEEK_SET) == 0) {
      uint32_t size = htobe32(buf.len);
      if (fwrite(&size, 1, sizeof(uint32_t), f) != sizeof(uint32_t))
        goto error;
      static const char COMPRESSION = 2;
      if (fwrite(&COMPRESSION, 1, sizeof(COMPRESSION), f) != sizeof(COMPRESSION))
        goto error;
      if (fwrite(buf.data, 1, buf.len, f) != buf.len)
        goto error;
    };
    goto success;
  }
error:
  if (f)
    fclose(f);
  buffer_free(&buf);
  return 1;
success:
  buffer_free(&buf);
  fclose(f);
  return 0;
};

size_t determine_region_file(char* buf, size_t len, int32_t cx, int32_t cz) {
  return snprintf(buf, len, "r.%d.%d.mca", cx >> 5, cz >> 5);
};
