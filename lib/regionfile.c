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
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int __region_write_offsets(regionfile* region);
int __region_write_timestamps(regionfile* region);

regionfile* open_regionfile(char* filename) {
  FILE* f = NULL;
  regionfile* region = NULL;
  struct stat st;
  if (stat(filename, &st) != 0) {
    f = fopen(filename, "a");
    if (!f)
      return NULL;
    fclose(f);
    f = NULL;
    region = malloc(sizeof(regionfile));
    bzero(region, sizeof(regionfile));
    region->filename = strdup(filename);
    if (__region_write_offsets(region) != 0 || __region_write_timestamps(region) != 0) {
      free_region(region);
      return NULL;
    }
    {
      char* fn = filename;
      while (sscanf(fn, "r.%d.%d.mca", &region->x, &region->z) != 2) {
        if (*++fn == 0x00)
          goto error;
      }
    }
    region->freeSectors = malloc(sizeof(unsigned char)*(2+1));
    memset(region->freeSectors, 0x01, 2);
    region->freeSectors[2] = 0x00;
    region->freeSectors[0] = 0x02;
    region->freeSectors[1] = 0x02;
    return region;
  }
  size_t filesize = st.st_size;
  if (filesize & 0xFFF) {
    filesize = (filesize | 0xFFF) + 1;
    if (truncate(filename, filesize) != 0)
      return NULL;
  }
  if (filesize == 0) {
    filesize = SECTOR_BYTES * 2;
    if (truncate(filename, filesize) != 0)
      return NULL;
  }
  f = fopen(filename, "rb");
  if (f) {
    region = malloc(sizeof(regionfile));
    bzero(region, sizeof(regionfile));
    {
      char* fn = filename;
      while (sscanf(fn, "r.%d.%d.mca", &region->x, &region->z) != 2) {
        if (*++fn == 0x00)
          goto error;
      }
    }
    if (fread(region->offsets, 4, SECTOR_INTS, f) != SECTOR_INTS)
      goto error;
    size_t i;
    for (i = 0; i < SECTOR_INTS; i++)
      region->offsets[i] = be32toh(region->offsets[i]);
    if (fread(region->timestamps, 4, SECTOR_INTS, f) != SECTOR_INTS)
      goto error;
    for (i = 0; i < SECTOR_INTS; i++)
      region->timestamps[i] = be32toh(region->timestamps[i]);
    uint32_t freeSectorsLength = filesize/SECTOR_BYTES;
    region->freeSectors = malloc(sizeof(unsigned char)*(freeSectorsLength+1));
    memset(region->freeSectors, 0x01, freeSectorsLength);
    region->freeSectors[freeSectorsLength] = 0x00;
    region->freeSectors[0] = 0x02; /* The first 2 sectors are the offsets and timestamps so are never free! */
    region->freeSectors[1] = 0x02;
    for (i = 0; i < SECTOR_INTS; i++) {
      uint32_t sector = region->offsets[i] >> 8;
      uint32_t count = region->offsets[i] & 0xFF;
      uint32_t j;
      for (j = sector; j < (sector + count); j++) {
        if (j >= freeSectorsLength) {
          /* If we reach this we should actually call a repair like function which we don't have yet. */
          break;
        }
        region->freeSectors[j] = 0x02;
      };
    };
    region->filename = strdup(filename);
    fclose(f);
    return region;
error:
    free_region(region);
    fclose(f);
  }
  return NULL;
};

int __region_write_offsets(regionfile* region) {
  if (!region)
    return -1;
  FILE* f;
  if (region->keepopen == 1) {
    if (region->file)
      f = region->file;
    else {
      f = fopen(region->filename, "rb+");
      region->file = f;
    }
  } else
    f = fopen(region->filename, "rb+");
  if (!f)
    return -2;
  uint32_t offsets[SECTOR_INTS];
  size_t i;
  for (i = 0; i < SECTOR_INTS; i++)
    offsets[i] = htobe32(region->offsets[i]);
  if (fwrite(offsets, 4, SECTOR_INTS, f) != SECTOR_INTS) {
    if (region->file != f)
      fclose(f);
    return -3;
  }
  if (region->file != f)
    fclose(f);
  return 0;
}

int __region_write_timestamps(regionfile* region) {
  if (!region)
    return -1;
  FILE* f;
  if (region->keepopen == 1) {
    if (region->file)
      f = region->file;
    else {
      f = fopen(region->filename, "rb+");
      region->file = f;
    }
  } else
    f = fopen(region->filename, "rb+");
  if (!f)
    return -2;
  if (fseek(f, SECTOR_INTS*4, SEEK_SET) != 0) {
    fclose(f);
    return -2;
  }
  uint32_t timestamps[SECTOR_INTS];
  size_t i;
  for (i = 0; i < SECTOR_INTS; i++)
    timestamps[i] = htobe32(region->timestamps[i]);
  if (fwrite(timestamps, 4, SECTOR_INTS, f) != SECTOR_INTS) {
    if (region->file != f)
      fclose(f);
    return -3;
  }
  if (region->file != f)
    fclose(f);
  return 0;
}

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
  FILE* f;
  if (region->keepopen == 1) {
    if (region->file)
      f = region->file;
    else {
      f = fopen(region->filename, "rb+");
      region->file = f;
    }
  } else
    f = fopen(region->filename, "rb+");
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
  if (region->file != f)
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
    if (region->freeSectors)
      free(region->freeSectors);
    if (region->filename)
      free(region->filename);
    if (region->file)
      fclose(region->file);
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

uint8_t region_chunk_sector_count(regionfile* region, int32_t cx, int32_t cz) {
  if (!region)
    return 0;
  cx &= 0x1f;
  cz &= 0x1f;
  uint32_t offset = region->offsets[cx + cz * 32];
  if (offset == 0)
    return 0;
  return (uint8_t) offset & 0xff;
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
  FILE* f;
  if (region->keepopen == 1) {
    if (region->file)
      f = region->file;
    else {
      f = fopen(region->filename, "rb+");
      region->file = f;
    }
  } else
    f = fopen(region->filename, "rb+");
  nbt_node* output = NULL;
  if (f && fseek(f, sectorStart*SECTOR_BYTES, SEEK_SET) == 0) {
    unsigned char buf[numSectors*SECTOR_BYTES];
    if (fread(buf, 1, sizeof(buf), f) == sizeof(buf)) { /* First 4 bytes are the length */
      size_t size = be32toh((uint32_t) (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24)));
      output = nbt_parse_compressed(buf+5, size); /* Fifth byte is the compression algorithm */
    }                                             /* But we don't need that as cNBT will figure that out */
  }
  if (region->file != f)
    fclose(f);
  return output;
};

extern nbt_node* new_chunk_data_to_nbt(chunk* c);

int write_chunk(regionfile* region, int32_t cx, int32_t cz, chunk* chunk) {
  if (region == NULL)
    return -1;
  if (chunk == NULL)
    return -1;
  cx &= 0x1f;
  cz &= 0x1f;
  uint32_t offset = region->offsets[cx + cz * 32];
  if (offset == 0)
    return -2;
  uint32_t numSectors = offset & 0xff;
  if (numSectors == 0)
    return -3;
  nbt_node* raw = new_chunk_data_to_nbt(chunk);
  if (raw == NULL)
    return -1;
  uint32_t sectorStart = offset >> 8;
  FILE* f;
  if (region->keepopen == 1) {
    if (region->file)
      f = region->file;
    else {
      f = fopen(region->filename, "rb+");
      region->file = f;
    }
  } else
    f = fopen(region->filename, "rb+");
  struct buffer buf = nbt_dump_compressed(raw, STRAT_INFLATE);
  uint16_t sectorsNeeded = (buf.len + 5) / SECTOR_BYTES + 1;
  if (sectorsNeeded >= 256)
    goto error;
  if (sectorStart != 0 && numSectors >= sectorsNeeded) {
    if (fseek(f, sectorStart*SECTOR_BYTES, SEEK_SET) != 0)
      goto error;
    uint32_t size = htobe32(buf.len);
    if (fwrite(&size, 1, sizeof(uint32_t), f) != sizeof(uint32_t))
      goto error;
    static const char COMPRESSION = 2;
    if (fwrite(&COMPRESSION, 1, sizeof(COMPRESSION), f) != sizeof(COMPRESSION))
      goto error;
    if (fwrite(buf.data, 1, buf.len, f) != buf.len)
      goto error;
    goto success;
  } else {
    size_t i;
    size_t runLength = 0;
    uint32_t runStart = 0;
    unsigned char* curSector = region->freeSectors;
    for (i = 0; curSector[i] != 0x00; i++) {
      if (runLength > 0) {
        if (curSector[i] == 0x01)
          runLength++;
        else
          runLength = 0;
      } else if (curSector[i] == 0x01) {
        runStart = i;
        runLength = 1;
      }
      if (runLength >= sectorsNeeded)
        break;
    }
    if (runLength >= sectorsNeeded) {
      region->offsets[cx + cz * 32] = runStart << 8 | sectorsNeeded;
      if (__region_write_offsets(region) != 0)
        goto error;
      if (fseek(f, runStart*SECTOR_BYTES, SEEK_SET) != 0)
        goto error;
      uint32_t size = htobe32(buf.len);
      if (fwrite(&size, 1, sizeof(uint32_t), f) != sizeof(uint32_t))
        goto error;
      static const char COMPRESSION = 2;
      if (fwrite(&COMPRESSION, 1, sizeof(COMPRESSION), f) != sizeof(COMPRESSION))
        goto error;
      if (fwrite(buf.data, 1, buf.len, f) != buf.len)
        goto error;
      goto success;
    } else {
      if (fseek(f, 0L, SEEK_END) != 0)
        goto error;
      long filesize = ftell(f);
      uint32_t sectorNumber = filesize/SECTOR_BYTES;
      filesize += sectorsNeeded * SECTOR_BYTES;
      if (ftruncate(fileno(f), filesize) != 0)
        goto error;
      region->offsets[cx + cz * 32] = sectorNumber << 8 | sectorsNeeded;
      if (__region_write_offsets(region) != 0)
        goto error;
      if (fseek(f, sectorNumber*SECTOR_BYTES, SEEK_SET) != 0)
        goto error;
      uint32_t size = htobe32(buf.len);
      if (fwrite(&size, 1, sizeof(uint32_t), f) != sizeof(uint32_t))
        goto error;
      static const char COMPRESSION = 2;
      if (fwrite(&COMPRESSION, 1, sizeof(COMPRESSION), f) != sizeof(COMPRESSION))
        goto error;
      if (fwrite(buf.data, 1, buf.len, f) != buf.len)
        goto error;

      uint32_t freeSectorsLength = filesize/SECTOR_BYTES;
      region->freeSectors = realloc(region->freeSectors, sizeof(unsigned char) * (freeSectorsLength+1));
      region->freeSectors[freeSectorsLength] = 0x00;
      for (;sectorNumber < freeSectorsLength; sectorNumber++)
        region->freeSectors[sectorNumber] = 0x02;
      goto success;
    }
  }
error:
  nbt_free(raw);
  if (f && region->file != f)
    fclose(f);
  buffer_free(&buf);
  return 1;
success:
  nbt_free(raw);
  buffer_free(&buf);
  if (region->file != f)
    fclose(f);
  region->timestamps[cx + cz * 32] = time(NULL);
  __region_write_timestamps(region);
  return 0;
};

size_t determine_region_file(char* buf, size_t len, int32_t cx, int32_t cz) {
  return snprintf(buf, len, "r.%d.%d.mca", cx >> 5, cz >> 5);
};
