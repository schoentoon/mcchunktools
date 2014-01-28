/*  mcchunktools
 *  Copyright (C) 2013-2014  Toon Schoenmakers
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
  int cleaned_up = cleanup_region(region);
  fprintf(stderr, "Cleaned up a total of %d bytes in %s\n", cleaned_up, rfile);
  free_region(region);
  return 0;
};