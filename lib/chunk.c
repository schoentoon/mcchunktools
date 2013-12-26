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

chunk* nbt_to_chunk(nbt_node* node, uint16_t flags) {
  chunk* output = malloc(sizeof(chunk));
  bzero(output, sizeof(chunk));
  nbt_node* xPos = nbt_find_by_name(node, "xPos");
  if (xPos && xPos->type == TAG_INT)
    output->x = xPos->payload.tag_int;
  nbt_node* zPos = nbt_find_by_name(node, "zPos");
  if (zPos && zPos->type == TAG_INT)
    output->z = zPos->payload.tag_int;
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
      memcpy(output->blocks[y], blocks_a->payload.tag_byte_array.data, blocks_a->payload.tag_byte_array.length);
      size_t i;
      for (i = 0; i < blocks_a->payload.tag_byte_array.length; i++) {
        int8_t data = 0;
        if (i % 2 == 0)
          data = data_a->payload.tag_byte_array.data[i/2] & 15;
        else
          data = data_a->payload.tag_byte_array.data[i/2] >> 4;
        output->data[y][z][x] = data;
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
  if (biomes_a && biomes_a->type == TAG_BYTE_ARRAY && biomes_a->payload.tag_byte_array.length == 256)
    memcpy(output->biomes, biomes_a->payload.tag_byte_array.data, biomes_a->payload.tag_byte_array.length);
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
  return output;
error:
  free_chunk(output);
  return NULL;
};

chunk* get_chunk(regionfile* region, int32_t cx, int32_t cz, uint16_t flags) {
  nbt_node* node = get_raw_chunk(region, cx, cz);
  if (!node)
    return NULL;
  chunk* output = nbt_to_chunk(node, flags);
  nbt_free(node);
  return output;
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

extern unsigned char _binary_blob_empty_chunk_gz_start;
extern unsigned char _binary_blob_empty_chunk_gz_end;

nbt_node* get_empty_raw_chunk() {
  size_t size = (&_binary_blob_empty_chunk_gz_end) - (&_binary_blob_empty_chunk_gz_start);
  printf("%d\n", (int) size);
  return nbt_parse_compressed(&_binary_blob_empty_chunk_gz_start, size);
};

nbt_node* new_chunk_data_to_nbt(nbt_node* node, chunk* c) {
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
      memcpy(blocks_a->payload.tag_byte_array.data, c->blocks[y], blocks_a->payload.tag_byte_array.length);
      size_t i;
      for (i = 0; i < blocks_a->payload.tag_byte_array.length; i += 2) {
        int8_t data = c->data[y][z][x];
        if (++x == CHUNK_WIDTH) {
          if (++z == CHUNK_LENGTH) {
            z = 0;
            y++;
          }
          x = 0;
        }
        data |= (c->data[y][z][x] << 4);
        data_a->payload.tag_byte_array.data[i/2] = data;
        if (++x == CHUNK_WIDTH) {
          if (++z == CHUNK_LENGTH) {
            z = 0;
            y++;
          }
          x = 0;
        }
      }
    } else
      return NULL;
  }
  nbt_node* biomes_a = nbt_find_by_name(node, "Biomes");
  if (biomes_a && biomes_a->type == TAG_BYTE_ARRAY && biomes_a->payload.tag_byte_array.length == 256)
    memcpy(biomes_a->payload.tag_byte_array.data, c->biomes, biomes_a->payload.tag_byte_array.length);
  nbt_node* inhabitedTime_a = nbt_find_by_name(node, "InhabitedTime");
  if (inhabitedTime_a && inhabitedTime_a->type == TAG_LONG)
    inhabitedTime_a->payload.tag_long = c->inhabitedTime;
  nbt_node* xPos = nbt_find_by_name(node, "xPos");
  if (xPos && xPos->type == TAG_INT)
    xPos->payload.tag_int = c->x;
  nbt_node* zPos = nbt_find_by_name(node, "zPos");
  if (zPos && zPos->type == TAG_INT)
    zPos->payload.tag_int = c->z;
  if (c->entities) {
    nbt_node* entities = nbt_find_by_name(node, "Entities");
    if (entities && entities->type == TAG_LIST) {
      const struct list_head* cursor;
      list_for_each(cursor, &c->entities->payload.tag_list->entry) {
        const struct nbt_list* entry = list_entry(cursor, const struct nbt_list, entry);
        nbt_node* clone = nbt_clone(entry->data);
        struct nbt_list* new = malloc(sizeof(struct nbt_list));
        new->data = clone;
        list_add_tail(&new->entry, &entities->payload.tag_list->entry);
      }
    }
  }
  if (c->tile_entities) {
    nbt_node* tentities = nbt_find_by_name(node, "TileEntities");
    if (tentities && tentities->type == TAG_LIST) {
      const struct list_head* cursor;
      list_for_each(cursor, &c->tile_entities->payload.tag_list->entry) {
        const struct nbt_list* entry = list_entry(cursor, const struct nbt_list, entry);
        nbt_node* clone = nbt_clone(entry->data);
        struct nbt_list* new = malloc(sizeof(struct nbt_list));
        new->data = clone;
        list_add_tail(&new->entry, &tentities->payload.tag_list->entry);
      }
    }
  }
  return node;
};