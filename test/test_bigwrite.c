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

#ifdef __cplusplus
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "mcchunk.h"

#include "insist.h"

#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {
  int32_t chunkx = 0, chunkz = 0;
  regionfile* region = open_regionfile("testdata/r.0.0.mca");
  insist(region, "write_testdata/r.0.0.mca failed to open..");

  nbt_node* raw_chunk = get_raw_chunk(region, chunkx, chunkz);
  insist(raw_chunk, "get_raw_chunk returned NULL");
  chunk* c = nbt_to_chunk(raw_chunk, GET_ENTITIES);
  insist(c, "get_chunk returned NULL");
  free_region(region);

  uint8_t x, z, y;
  for (x = 0; x < 16; x++) {
    for (z = 0; z < 16; z++) {
      for (y = 0; y < 255; y++) {
        if (c->blocks[y][z][x] == 24) {
          c->blocks[y][z][x] = 57;
          c->data[y][z][x] = 0;
        }
      }
    }
  }
  nbt_node* entity = nbt_list_item(c->entities, 0); /* Let's just get the first entity in the list.. */
  size_t i;
  for (i = 0; i < 10000; i++) {
    nbt_node* clone = nbt_clone(entity);
    struct nbt_list* new = malloc(sizeof(struct nbt_list));
    new->data = clone;
    list_add_tail(&new->entry, &c->entities->payload.tag_list->entry);
  }

  regionfile* write_region = open_regionfile("write_testdata/r.0.0.mca");
  insist(write_region, "write_testdata/r.0.0.mca failed to open..");
  int ret = write_chunk(write_region, chunkx, chunkz, raw_chunk, c);
  insist(ret == 0, "write_chunk returned non-zero %d", ret);
  nbt_free(raw_chunk);
  free_chunk(c);
  free_region(write_region);
  return 0;
};
