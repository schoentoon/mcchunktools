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

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Requires at least 1 argument!\n");
    return 1;
  }
  regionfile* region = open_regionfile(argv[1]);
  fprintf(stderr, "%s has %zu chunks.\n", region->filename, count_chunks(region));
  nbt_node* node = get_chunk(region, 0, 0);
  if (node) {
    char* dump = nbt_dump_ascii(node);
    fprintf(stdout, "%s\n", dump);
    free(dump);
  } else {
    fprintf(stderr, "%s\n", nbt_error_to_string(errno));
  }
  nbt_free(node);
  free_region(region);
  return 0;
}