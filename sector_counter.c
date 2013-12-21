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

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "regionfile", required_argument, 0, 'r' },
  { 0, 0, 0, 0 }
};

int usage(char* program) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n", program);
  fprintf(stderr, "--regionfile, -r Use this region file [required]\n");
  return 1;
};

int main(int argc, char** argv) {
  char* rfile = NULL;
  int arg, optindex;
  while ((arg = getopt_long(argc, argv, "hr:", g_LongOpts, &optindex)) != -1) {
    switch (arg) {
    case 'h':
      return usage(argv[0]);
    case 'r':
      rfile = optarg;
      break;
    }
  }
  if (!rfile)
    return usage(argv[0]);
  regionfile* region = open_regionfile(rfile);
  if (!region) {
    fprintf(stderr, "Failed to open %s.\n", rfile);
    return 1;
  }
  printf("%s has %zu chunks.\n", rfile, count_chunks(region));
  int32_t x = region->x * 32;
  int32_t max_x = x + 32;
  int32_t z = region->z * 32;
  int32_t max_z = z + 32;
  int32_t x_loop, z_loop;
  for (x_loop = x; x_loop < max_x; x_loop++) {
    for (z_loop = z; z_loop < max_z; z_loop++) {
      if (region_contains_chunk(region, x_loop, z_loop))
        printf("%d, %d is stored in a total of %hhu sectors.\n", x_loop, z_loop, region_chunk_sector_count(region, x_loop, z_loop));
    }
  }
  free_region(region);
  return 0;
};