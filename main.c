#include "nbt.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <endian.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Requires at least 1 argument!\n");
    return 1;
  }
  FILE* region = fopen(argv[1], "rb+");
  if (region) {
    uint32_t offsets[1024];
    if (fread(offsets, 4, 1024, region) != 1024)
      return 1;
    size_t i;
    for (i = 0; i < 1024; i++)
      offsets[i] = be32toh(offsets[i]);
    uint32_t timestamps[1024];
    if (fread(timestamps, 4, 1024, region) != 1024)
      return 1;
    for (i = 0; i < 1024; i++) {
      if (offsets[i] == 0)
        continue;
      uint32_t sectorStart = offsets[i] >> 8;
      uint32_t numSectors = offsets[i] & 0xff;
      if (numSectors == 0)
        continue;
      if (fseek(region, sectorStart*4096, SEEK_SET) == 0) {
        unsigned char buf[4096*numSectors];
        size_t size = fread(buf, 1, sizeof(buf), region);
        nbt_node* node = nbt_parse_compressed(buf+5, size-5);
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
    /*char buf[4096];
    size_t size = fread(buf, 1, sizeof(buf), region);
    fprintf(stderr, "size = %lu\n", size);
    //nbt_node* node = nbt_parse_file(region);
    nbt_node* node = nbt_parse_compressed(buf, size);
    if (node) {
      char* dump = nbt_dump_ascii(node);
      fprintf(stderr, "%s\n", dump);
      free(dump);
    } else {
      fprintf(stderr, "%s\n", nbt_error_to_string(errno));
    }*/
  }
  fclose(region);
  return 0;
}