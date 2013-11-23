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

  insist(count_chunks(region) == 121, "Expected 121 chunks in our test file, got %zu, did our testfile change?", count_chunks(region));
  insist(region_contains_chunk(region, chunkx, chunkz), "Our test region file doesn't contain chunk 0,0");

  chunk* c = get_chunk(region, chunkx, chunkz, GET_TILE_ENTITIES|GET_ENTITIES);

  insist(c->tile_entities != NULL, "Requested tile entities but we didn't get them?");
  insist(c->entities != NULL, "Requested entities but we didn't get them?");

  insist(c->inhabitedTime == 65, "We expected inhabited time to be 65, not %ld", c->inhabitedTime);

  insist(c, "get_chunk returned NULL");

  insist(c->blocks[56][1][2] == 95 && c->data[56][1][2] == 15, "Expected black stained glass at x:2, z:1, y:56");

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

  insist(analyze[0][0] == 50940, "Got %zu air blocks, expected 50940", analyze[0][0]);
  insist(analyze[1][0] == 768, "Got %zu stone blocks, expected 768", analyze[1][0]);
  insist(analyze[7][0] == 256, "Got %zu bedrock blocks, expected 256", analyze[7][0]);
  insist(analyze[24][0] == 13312, "Got %zu sandstone blocks, expected 13312", analyze[24][0]);
  insist(analyze[54][2] == 1, "Got %zu chests, expected 1", analyze[54][2]);
  insist(analyze[95][5] == 2, "Got %zu lime stained glass blocks, expected 2", analyze[95][5]);
  insist(analyze[95][15] == 1, "Got %zu black stained glass blocks, expected 1", analyze[95][15]);


  insist(analyze_biomes[2] == 252, "Got %hhu blocks with a desert biome, expected 252", analyze_biomes[2]);
  insist(analyze_biomes[17] == 4, "Got %hhu blocks with a desert hills biome, expected 4", analyze_biomes[17]);

  free_region(region);
  return 0;
};