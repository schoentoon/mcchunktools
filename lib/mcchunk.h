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

#ifndef _MCCHUNK_H
#define _MCCHUNK_H

#ifdef __cplusplus
extern "C" {
#else
#  if __STDC_VERSION__ < 199901L
#    define restrict
#  endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "nbt.h"

#define SECTOR_BYTES 4096
#define SECTOR_INTS 1024

typedef struct {
  uint32_t offsets[SECTOR_INTS];
  uint32_t timestamps[SECTOR_INTS];
  unsigned char* freeSectors; /* Dynamic array, 0x00 terminated. 1 is free sector, otherwise is used sector */
  char* filename;
} regionfile;

#define CHUNK_WIDTH 16
#define CHUNK_LENGTH 16
#define CHUNK_HEIGHT 256

typedef struct {
  int32_t x;
  int32_t z;
  int8_t blocks[CHUNK_HEIGHT][CHUNK_LENGTH][CHUNK_WIDTH];
  int8_t data[CHUNK_HEIGHT][CHUNK_LENGTH][CHUNK_WIDTH];
  int8_t biomes[CHUNK_LENGTH][CHUNK_WIDTH];
  int64_t inhabitedTime;
  nbt_node* tile_entities;
  nbt_node* entities;
} chunk;

enum chunk_options {
  GET_TILE_ENTITIES = (1<<0),
  GET_ENTITIES      = (1<<1)
};

/** Used to open a region file simply pass
 * the filename into it
 */
regionfile* open_regionfile(char* filename);

/** Used to free the region structure
 */
void free_region(regionfile* region);

/** Count the amount of chunks available in this
 * region structure
 */
size_t count_chunks(regionfile* region);

typedef void (*raw_chunk_func)(nbt_node* node, void* context);

/** Similar to for_each_chunk but gives you access to the raw nbt_node*
 * instead.
 */
void for_each_chunk_raw(regionfile* region, raw_chunk_func function, void* context);

typedef void (*chunk_func)(chunk* c, void* context);

/** Loop through all the available chunks in our regionfile
 * don't do some odd for loop yourself calling get_chunk as
 * this function is simply more efficient, it keeps the file
 * open for example. (And it's just easier..)
 * Don't keep the chunk around, as it is freed automatically
 */
void for_each_chunk(regionfile* region, chunk_func function, void* context);

/** Check if this region structure contains the
 * chunk located at x: cx y: cy returns 1 if it
 * is in the region structure, 0 otherwise
 * ingame you can get the chunk coordinates from
 * the debug screen (F3). It'll look something like
 * this:
 * x: -20.64464 (-21) // c: -2 (11)
 * y: 120.500 (feet pos, 122.120 eyes pos)
 * z: 36.39552 (36) // c: 2 (4)
 *
 * The x, y & z coordinates here are your players location
 * the number just behind "c:" is the chunk number. So in
 * this case it would be x: -2, z: 2.
 */
int region_contains_chunk(regionfile* region, int32_t cx, int32_t cz);

/** Return the amount of internal sectors this chunk consists of,
 * will return 0 on error.
 */
uint8_t region_chunk_sector_count(regionfile* region, int32_t cx, int32_t cz);

/** Returns the nbt structure for this particular chunk
 * @see region_contains_chunk @see chunk_from_coord
 */
nbt_node* get_raw_chunk(regionfile* region, int32_t cx, int32_t cz);

/** Convert a raw nbt_node* to a chunk structure instead
 * will return NULL upon failure.
 */
chunk* nbt_to_chunk(nbt_node* node, uint16_t flags);

/** Calculate in which chunk this coordinate is
 */
#define chunk_from_coord(i) (i/16)

/** Determine in which region file the chunk cx, cz
 * is located. @see region_contains_chunk @see chunk_from_coord
 * Don't forget to free the output!
 */
size_t determine_region_file(char* buf, size_t len, int32_t cx, int32_t cz);

/** Writes the chunk to the regionfile at position cx, cz
 * at this point this implementation is very very lacking.
 * It can only write chunks that were already in the file for example.
 * I recommend to not use it as of now.
 */
int write_chunk(regionfile* region, int32_t cx, int32_t cz, nbt_node* raw, chunk* c);

/** Initialize the block names database you must call
 * this once before using @see get_block_name
 */
void initblockdb();

/** Get the human readable name for block_id with data
 * if the block simply doesn't exist it'll return NULL
 * @see initblockdb
 */
char* get_block_name(uint8_t block_id, uint8_t data);

/** Initialize the biome name database
 * you must call this before using @see get_biome_name
 */
void initbiomedb();

/** Get the human readable name for biome_id if this
 * biome_id simply doesn't exist it'll return NULL
 * (-1 isn't even handled) @see initbiomedb
 */
char* get_biome_name(uint8_t biome_id);

/** Get a chunk structure for the chunk located at cx, cz in
 * region. Blocks in this structure are accessed using the
 * blocks and data arrays, it would look like the following:
 * c->blocks[y][z][x];
 */
chunk* get_chunk(regionfile* region, int32_t cx, int32_t cz, uint16_t flags);

/** Free function for the chunk structure
 */
void free_chunk(chunk* c);

#ifdef __cplusplus
}
#endif

#endif //_MCCHUNK_H
