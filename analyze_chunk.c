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
    char buf[BUFSIZ];
    if (determine_region_file(buf, sizeof(buf), chunkx, chunkz) > 0)
      fprintf(stderr, "I suggest you look in \"%s\" instead.\n", buf);
    return 1;
  }
  chunk* c = get_chunk(region, chunkx, chunkz, 0);
  if (c) {
    uint64_t analyze[256][16];
    bzero(analyze, sizeof(analyze));
    uint8_t analyze_biomes[256];
    bzero(analyze_biomes, sizeof(analyze_biomes));
    uint8_t x, z, y;
    for (x = 0; x < 16; x++) {
      for (z = 0; z < 16; z++) {
        analyze_biomes[c->biomes[x][z]]++;
        for (y = 0; y < 255; y++)
          analyze[c->blocks[y][z][x]][c->data[y][z][x]]++;
      }
    }
    free_chunk(c);
    initblockdb();
    size_t i;
    for (i = 0; i < 256; i++) {
      size_t j;
      for (j = 0; j < 16; j++) {
        if (analyze[i][j] > 0)
          fprintf(stderr, "%s %lu\n", get_block_name(i, j), analyze[i][j]);
      }
    }
    initbiomedb();
    for (i = 0; i < 256; i++) {
      if (analyze_biomes[i] > 0)
        fprintf(stderr, "%s %hhu\n", get_biome_name(i), analyze_biomes[i]);
    }
  } else {
    fprintf(stderr, "%s\n", nbt_error_to_string(errno));
  }
  free_region(region);
  return 0;
};