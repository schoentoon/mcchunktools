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

chunk* get_chunk(regionfile* region, int32_t cx, int32_t cz, uint16_t flags) {
  nbt_node* node = get_raw_chunk(region, cx, cz);
  if (!node)
    return NULL;
  chunk* output = malloc(sizeof(chunk));
  bzero(output, sizeof(chunk));
  output->x = cx;
  output->z = cz;
  nbt_node* sections = nbt_find_by_name(node, "Sections");
  uint8_t x = 0;
  uint8_t z = 0;
  uint8_t y = 0;
  struct list_head* pos;
  list_for_each(pos, &sections->payload.tag_list->entry) {
    const struct nbt_list* entry = list_entry(pos, const struct nbt_list, entry);
    nbt_node* blocks_a = nbt_find_by_name(entry->data, "Blocks");
    nbt_node* data_a = nbt_find_by_name(entry->data, "Data");
    if (blocks_a && data_a && blocks_a->type == TAG_BYTE_ARRAY && data_a->type == TAG_BYTE_ARRAY
      && blocks_a->payload.tag_byte_array.length == 4096
      && data_a->payload.tag_byte_array.length == 2048) {
      size_t i;
      for (i = 0; i < blocks_a->payload.tag_byte_array.length; i++) {
        int8_t data = 0;
        int8_t block = blocks_a->payload.tag_byte_array.data[i];
        if (i % 2 == 0)
          data = data_a->payload.tag_byte_array.data[i/2] & 15;
        else
          data = data_a->payload.tag_byte_array.data[i/2] >> 4;
        output->blocks[x][z][y] = block;
        output->data[x][z][y] = data;
        if (++x == CHUNK_WIDTH) {
          if (++z == CHUNK_LENGTH) {
            z = 0;
            y++;
          }
          x = 0;
        }
      }
    } else
      goto error;
  }
  nbt_node* biomes_a = nbt_find_by_name(node, "Biomes");
  if (biomes_a && biomes_a->type == TAG_BYTE_ARRAY && biomes_a->payload.tag_byte_array.length == 256) {
    x = 0;
    z = 0;
    size_t i;
    for (i = 0; i < biomes_a->payload.tag_byte_array.length; i++) {
      output->biomes[x][z] = biomes_a->payload.tag_byte_array.data[i];
      if (++x == CHUNK_WIDTH) {
        if (++z == CHUNK_LENGTH)
          z = 0;
        x = 0;
      }
    }
  }
  nbt_node* inhabitedTime_a = nbt_find_by_name(node, "InhabitedTime");
  if (inhabitedTime_a && inhabitedTime_a->type == TAG_LONG)
    output->inhabitedTime = inhabitedTime_a->payload.tag_long;
  if (flags & GET_TILE_ENTITIES) {
    nbt_node* tentities = nbt_find_by_name(node, "TileEntities");
    if (tentities && tentities->type == TAG_LIST)
      output->tile_entities = nbt_clone(tentities);
  }
  if (flags & GET_ENTITIES) {
    nbt_node* entities = nbt_find_by_name(node, "Entities");
    if (entities && entities->type == TAG_LIST)
      output->entities = nbt_clone(entities);
  }
  nbt_free(node);
  return output;
error:
  free_chunk(output);
  nbt_free(node);
  return NULL;
};

void free_chunk(chunk* c) {
  if (c) {
    if (c->tile_entities)
      nbt_free(c->tile_entities);
    if (c->entities)
      nbt_free(c->entities);
    free(c);
  }
};