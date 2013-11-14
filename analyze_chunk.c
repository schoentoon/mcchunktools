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

#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "regionfile", required_argument, 0, 'r' },
  { "chunkx",     required_argument, 0, 'x' },
  { "chunkz",     required_argument, 0, 'z' },
  { 0, 0, 0, 0 }
};

int usage(char* program) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n", program);
  fprintf(stderr, "--regionfile, -r Use this region file [required]\n");
  fprintf(stderr, "--chunkx, -x     The x coordinate of the chunk to read out.\n");
  fprintf(stderr, "--chunkz, -z     The z coordinate of the chunk to read out.\n");
  return 1;
};

int main(int argc, char** argv) {
  int32_t chunkx = 0, chunkz = 0;
  char* rfile = NULL;
  int arg, optindex;
  while ((arg = getopt_long(argc, argv, "hr:x:z:", g_LongOpts, &optindex)) != -1) {
    switch (arg) {
    case 'h':
      return usage(argv[0]);
    case 'r':
      rfile = optarg;
      break;
    case 'x':
      chunkx = atoi(optarg);
      break;
    case 'z':
      chunkz = atoi(optarg);
      break;
    }
  }
  if (!rfile)
    return usage(argv[0]);
  regionfile* region = open_regionfile(rfile);
  if (!region_contains_chunk(region, chunkx, chunkz)) {
    fprintf(stderr, "This region file doesn't contain chunk x:%d, z:%d\n", chunkx, chunkz);
    char* tmp = determine_region_file(chunkx, chunkz);
    fprintf(stderr, "I suggest you look in \"%s\" instead.\n", tmp);
    free(tmp);
    return 1;
  }
  nbt_node* node = get_chunk(region, chunkx, chunkz);
  if (node) {
    uint64_t analyze[256][16];
    bzero(analyze, sizeof(analyze));
    nbt_node* sections = nbt_find_by_name(node, "Sections");
    struct list_head* pos;
    list_for_each(pos, &sections->payload.tag_list->entry) {
      const struct nbt_list* entry = list_entry(pos, const struct nbt_list, entry);
      nbt_node* blocks = nbt_find_by_name(entry->data, "Blocks");
      nbt_node* data = nbt_find_by_name(entry->data, "Data");
      if (blocks && data && blocks->type == TAG_BYTE_ARRAY && data->type == TAG_BYTE_ARRAY
        && blocks->payload.tag_byte_array.length == 4096
        && data->payload.tag_byte_array.length == 2048) {
        size_t i;
        for (i = 0; i < blocks->payload.tag_byte_array.length; i++) {
          if (i % 2 == 0)
            analyze[blocks->payload.tag_byte_array.data[i]][data->payload.tag_byte_array.data[i/2] & 15]++;
          else
            analyze[blocks->payload.tag_byte_array.data[i]][data->payload.tag_byte_array.data[i/2] >> 4]++;
        }
      }
    }
    nbt_free(node);
    initblockdb();
    size_t i;
    for (i = 0; i < 256; i++) {
      size_t j;
      for (j = 0; j < 16; j++) {
        if (analyze[i][j] > 0)
          fprintf(stderr, "%s %lu\n", get_block_name(i, j), analyze[i][j]);
      }
    }
  } else {
    fprintf(stderr, "%s\n", nbt_error_to_string(errno));
  }
  free_region(region);
  return 0;
};